#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <mtd/mtd-user.h>
#include "crc32.h"

static int size = -1;
static int erase_size = -1;

static int prim_fd = -1, sec_fd = -1;
static char *read_buf = NULL;
static char *write_buf = NULL;
static char **env = NULL;

void cleanup(void)
{
    if (prim_fd > 0)
        close(prim_fd);
    if (sec_fd > 0)
        close(sec_fd);
    free(read_buf);
    free(write_buf);
    free(env);
}

#ifdef USE_QB_BINARY_IMAGE
#define ACTUAL_NVRAM_SIZE       ((32 * 1024) - sizeof(QBImageHeader))
#include "QBBinaryImage.h"

#ifndef QB_IMAGE_VERSION_DEF
#define QB_IMAGE_VERSION_DEF 1
#endif

static unsigned char nvram_key[32];
static unsigned char nvram_hmac_key[32];
static const char primHmacData[] = "nvram";
static const char secHmacData[] = "nvramB";

ssize_t skip_read(int fd, void *buffer, size_t count)
{
    __kernel_loff_t blocknum = 0;
    int ret;
    size_t offset = 0;

    while (offset < count) {
        size_t toread = erase_size < count - offset ? erase_size : count - offset;

        while ((ret = ioctl(fd, MEMGETBADBLOCK, &blocknum)) == 1) {
            blocknum++;
        }
        if (ret != 0)
            return -EIO;

        lseek(fd, blocknum * erase_size, SEEK_SET);

        ret = read(fd, (unsigned char *) buffer + offset, toread);
        if (ret != toread) {
            return -EIO;
        }
        blocknum++;
        offset += toread;
    }

    return count;
}

ssize_t skip_write(int fd, void *buffer, size_t count)
{
    __kernel_loff_t blocknum = 0;
    int ret;
    struct erase_info_user erase;
    size_t offset = 0;

    while (offset < count) {
        size_t towrite = erase_size < count - offset ? erase_size : count - offset;

        while ((ret = ioctl(fd, MEMGETBADBLOCK, &blocknum)) == 1) {
            blocknum++;
        }
        if (ret != 0)
            return -EIO;

        erase.start = blocknum * erase_size;
        erase.length = erase_size;
        if (ioctl(fd, MEMERASE, &erase) < 0) {
            fprintf(stderr, "error: erase failed: %s\n", strerror(errno));
            return -EIO;
        }
        lseek(fd, blocknum * erase_size, SEEK_SET);

        ret = write(fd, (unsigned char *) buffer + offset, towrite);
        if (ret != towrite) {
            return -EIO;
        }
        blocknum++;
        offset += towrite;
    }

    return count;
}

int enc_load_key(void)
{
    int fd = open("/etc/keys/nvram", O_RDONLY);
    if (fd < 0)
        return -1;
    if (read(fd, nvram_key, sizeof(nvram_key)) != sizeof(nvram_key))
        return -1;
    close(fd);

    fd = open("/etc/keys/nvram_hmac", O_RDONLY);
    if (fd < 0)
        return -1;
    if (read(fd, nvram_hmac_key, sizeof(nvram_hmac_key)) != sizeof(nvram_hmac_key))
        return -1;
    close(fd);
    return 0;
}

int enc_write(int fd, void *buffer, int wsize)
{
    unsigned char *destImage = NULL;
    size_t imageSize = ACTUAL_NVRAM_SIZE + sizeof(QBImageHeader);
    int loaded;
    const char *hmacData = (fd == prim_fd) ? primHmacData : secHmacData;

    destImage = malloc(imageSize);

    if (QBImageDynamicCreate(buffer, ACTUAL_NVRAM_SIZE,
                             hmacData, strlen(hmacData),
                             nvram_hmac_key, nvram_key,
                             destImage, &imageSize) != 0) {
        fprintf(stderr, "Cannot create image\n");
        free(destImage);
        return -1;
    }

    loaded = skip_write(fd, destImage, imageSize);

    free(destImage);

    if (loaded < 0 || (size_t) loaded != imageSize) {
        fprintf(stderr, "Cannot write to device\n");
        return -1;
    }

    return wsize;
}

int enc_read(int fd, void *buffer, int rsize)
{
    unsigned char temp[ACTUAL_NVRAM_SIZE + sizeof(QBImageHeader)];
    unsigned char destBuf[ACTUAL_NVRAM_SIZE];
    int loaded;
    QBImageContext ctx;
    QBImageHeader qb_header;
    const char *hmacData = (fd == prim_fd) ? primHmacData : secHmacData;
    unsigned char *payload = temp + sizeof(QBImageHeader);

    loaded = skip_read(fd, temp, ACTUAL_NVRAM_SIZE + sizeof(QBImageHeader));

    if (loaded != ACTUAL_NVRAM_SIZE + sizeof(QBImageHeader)) {
        fprintf(stderr, "Cannot read from device\n");
        return -1;
    }

#if QB_IMAGE_VERSION_DEF < 2
    memcpy(&qb_header, temp, sizeof(QBImageHeader));
    if (QBImageDynamicVerifyHeader(&qb_header, nvram_hmac_key, hmacData, strlen(hmacData)) != 0) {
#else
    if (QBImageDynamicGetVerifiedHeader((QBImageHeader *) temp, nvram_hmac_key, nvram_key, hmacData, strlen(hmacData), &qb_header) != 0) {
#endif
        fprintf(stderr, "NVRAM header verification failed\n");
        return -1;
    }

#if QB_IMAGE_VERSION_DEF < 2
    if (qb_header.verificationMode != QB_IMAGE_VERIFICATION_HMAC256_SHA256) {
#else
    if (qb_header.encryptedData.verificationMode != QB_IMAGE_VERIFICATION_HMAC256_SHA256) {
#endif
        fprintf(stderr, "Invalid verification mode in header\n");
        return -1;
    }

#if QB_IMAGE_VERSION_DEF < 2
    if (qb_header.encryptionMode != QB_IMAGE_ENCRYPTION_AES256CBC) {
#else
    if (qb_header.encryptedData.encryptionMode != QB_IMAGE_ENCRYPTION_AES256CBC) {
#endif
        fprintf(stderr, "Invalid encryption mode in header\n");
        return -1;
    }

#if QB_IMAGE_VERSION_DEF < 2
    if (QB_IMAGE_CONVERT_ENDIAN(qb_header.imageSize) != ACTUAL_NVRAM_SIZE) {
#else
    if (QB_IMAGE_CONVERT_ENDIAN(qb_header.encryptedData.imageSize) != ACTUAL_NVRAM_SIZE) {
#endif
        fprintf(stderr, "Invalid NVRAM size in header\n");
        return -1;
    }

    ctx = QBImageContextCreate();
#if QB_IMAGE_VERSION_DEF < 2
    QBImageDynamicVerifyStart(nvram_hmac_key, ctx);
    QBImageDynamicVerifyUpdate(payload, QB_IMAGE_CONVERT_ENDIAN(qb_header.imageSize), ctx);
    if (QBImageDynamicVerifyFinish(hmacData, strlen(hmacData), &qb_header, ctx) != 0) {
#else
    QBImageDynamicVerifyStart(ctx, nvram_hmac_key);
    QBImageDynamicVerifyUpdate(ctx, payload, QB_IMAGE_CONVERT_ENDIAN(qb_header.encryptedData.imageSize));
    if (QBImageDynamicVerifyFinish(ctx, hmacData, strlen(hmacData), &qb_header) != 0) {
#endif
        fprintf(stderr, "NVRAM verification failed\n");
        QBImageContextDestroy(ctx);
        return -1;
    }

#if QB_IMAGE_VERSION_DEF < 2
    QBImageDecryptStart(&qb_header, nvram_key, ctx);
    if (QBImageDecrypt(payload, QB_IMAGE_CONVERT_ENDIAN(qb_header.imageSize), destBuf, ctx) != 0) {
#else
    QBImageDecryptStart(ctx, &qb_header, nvram_key);
    if (QBImageDecrypt(ctx, payload, QB_IMAGE_CONVERT_ENDIAN(qb_header.encryptedData.imageSize), destBuf) != 0) {
#endif
        fprintf(stderr, "NVRAM decryption failed\n");
        QBImageContextDestroy(ctx);
        return -1;
    }

    memcpy(buffer, destBuf, rsize);
    QBImageContextDestroy(ctx);

    return rsize;
}
#else
#ifdef ENCRYPT_NVRAM
#include <openssl/aes.h>

static unsigned char nvram_key[32];

int enc_load_key(void)
{
    int fd = open("/etc/keys/nvram", O_RDONLY);
    if (fd < 0)
        return -1;
    if (read(fd, nvram_key, sizeof(nvram_key)) != sizeof(nvram_key))
        return -1;
    close(fd);
    return 0;
}

int enc_write(int fd, void *buffer, int wsize)
{
    AES_KEY enc_key;
    unsigned char *encrypted_buf = malloc(wsize);
    int i;

    AES_set_encrypt_key(((const unsigned char *) nvram_key), 256, &enc_key);

    for (i = 0; i < wsize; i += 16) {
        AES_encrypt((unsigned char *) buffer + i, encrypted_buf + i, &enc_key);
    }
    i = write(fd, encrypted_buf, wsize);

    free(encrypted_buf);
    return i;
}

int enc_read(int fd, void *buffer, int rsize)
{
    AES_KEY dec_key;
    unsigned char *read_buf = malloc(rsize);
    int i;

    AES_set_decrypt_key(((const unsigned char *) nvram_key), 256, &dec_key);

    i = read(fd, read_buf, rsize);
    if (i != rsize) {
        free(read_buf);
        return i;
    }

    for (i = 0; i < rsize; i += 16) {
        AES_decrypt(read_buf + i, (unsigned char *) buffer + i, &dec_key);
    }

    free(read_buf);

    return size;
}
#endif
#endif

int write_env_ram(int fd, char **env, char *buffer)
{
    int i, written = 0;
    int reclen;
    char *begin = buffer, *buf = buffer;
    struct erase_info_user erase;
    memset(buffer, 0x00, size);
    uint32_t crc = 0;

    buf += 4; // Reserved for crc
    for (i = 0; env[i]; i++) {
        if (buf - begin + strlen(env[i]) + 1 > size) {
            fprintf(stderr, "error: %s does not fit on env device\n", env[i]);
            return -ENOMEM;
        }
        strcpy(buf, env[i]);
        buf += strlen(env[i]) + 1;
        written++;
    }
    crc = QBCrc32Calculate(begin + 4, size - 4);
    memcpy(begin, &crc, sizeof(crc));

    /* erase */
    erase.start = 0;
#ifndef USE_QB_BINARY_IMAGE
    erase.length = erase_size;
    if (ioctl(fd, MEMERASE, &erase) < 0) {
        fprintf(stderr, "error: erase(%d, %d) failed: %s\n", fd, erase.length, strerror(errno));
        return -EIO;
    }
    lseek(fd, 0L, SEEK_SET);
#endif
#ifdef ENCRYPT_NVRAM
    if (enc_write(fd, begin, size) != size) {
        fprintf(stderr, "error: write failed: %s\n", strerror(errno));
        return -EIO;
    }
#else
    if (write(fd, begin, size) != size) {
        fprintf(stderr, "error: write failed: %s\n", strerror(errno));
        return -EIO;
    }
#endif
    return written;
}

/*return numer of read values or <0 err code if failed */
int read_nv(int fd, char ***env, char *buf)
{
    int valnum;
    char *ptr;
    char *envStart;
    int i;
    int len;
    uint32_t crc = 0;
#ifdef ENCRYPT_NVRAM
    if (enc_read(fd, buf, size) < 0) {
        return -EIO;
    }
#else
    if (read(fd, buf, size) < 0) {
        return -EIO;
    }
#endif
    crc = QBCrc32Calculate(buf + 4, size - 4);
    if (crc != *(uint32_t *) buf) {
        return -1;
    }

    buf[size - 1] = 0;
    valnum = 16;
    if (*env)
        free(*env);
    *env = (char **) malloc(valnum * sizeof(char *));
    memset(*env, 0, valnum * sizeof(char *));
    if (!*env) {
        fprintf(stderr, "error: cant allocate memory for env\n");
        return -ENOMEM;
    }

    ptr = buf + 4;
    envStart = ptr;
    i = -1;
    while ((ptr - buf) < size) {
        if (*ptr == 0) {
            i++;
            if (ptr == envStart)
                break;

            if (i >= (valnum - 1)) {
                valnum <<= 1;
                *env = (char **) realloc(*env, valnum * sizeof(char *));
                if (!*env) {
                    fprintf(stderr, "error: cant realloc environment to %zd\n", valnum * sizeof(char *));
                    return -ENOMEM;
                }
            }
            (*env)[i] = envStart;
            (*env)[i + 1] = NULL;
            envStart = ptr + 1;
        }
        ptr++;
    }

    return i;
}

int main(int argc, char *argv[])
{
    struct mtd_info_user prim_mtd, sec_mtd;
    int ret, i, fd;
    char prim[0x10], sec[0x10], *p, *s;
    char buf[1024];

    if (!(argc % 2)) {
        fprintf(stderr, "usage:\n"
                "    %s [variable value]\n", argv[0]);
        return -EINVAL;
    }

#ifdef ENCRYPT_NVRAM
    if (enc_load_key() < 0) {
        fprintf(stderr, "error: cant read encryption key\n");
        return -EIO;
    }
#endif

    if ((fd = open("/proc/mtd", O_RDONLY)) < 0 || read(fd, buf, sizeof(buf)) <= 0) {
        fprintf(stderr, "error: cant read /proc/mtd file\n");
        cleanup();
        return errno;
    }

    p = strstr(buf, "nvram\"\n");
    s = strstr(buf, "nvramB\"\n");
    while (p && p != buf && p[-1] != '\n' && --p)
        if (*p == ':')
            *p = 0;
    while (s && s != buf && s[-1] != '\n' && --s)
        if (*s == ':')
            *s = 0;
    if (!p || p == buf || !s || s == buf) {
        fprintf(stderr, "error: nvram%s partition not found\n", (!p || p == buf) ? "" : "B");
        cleanup();
        return errno;
    }
    sprintf(prim, "/dev/%s", p);
    sprintf(sec, "/dev/%s", s);

    if ((prim_fd = open(prim, O_SYNC | O_RDWR)) < 0) {
        fprintf(stderr, "error: cant open file %s: %s\n", prim, strerror(errno));
        cleanup();
        return errno;
    }

    if ((sec_fd = open(sec, O_SYNC | O_RDWR)) < 0) {
        fprintf(stderr, "error: cant open file %s: %s\n", sec, strerror(errno));
        cleanup();
        return errno;
    }

    if (ioctl(prim_fd, MEMGETINFO, &prim_mtd)) {
        fprintf(stderr, "error: incorrect flash device %s\n", argv[1]);
        cleanup();
        return errno;
    }

    if (ioctl(sec_fd, MEMGETINFO, &sec_mtd)) {
        fprintf(stderr, "error: incorrect flash device %s\n", argv[2]);
        cleanup();
        return errno;
    }

#ifdef USE_QB_BINARY_IMAGE
    size = ACTUAL_NVRAM_SIZE;
#else
#if 1
#warning "16kB nvram size override for compliance with U-Boot!"
    size = 0x10000;
#else
    size = prim_mtd.size;
#endif
#endif
    erase_size = prim_mtd.erasesize;

    if (!(read_buf = malloc(size)) || !(write_buf = malloc(size))) {
        fprintf(stderr, "error: cant allocate buffor(%d) memory\n", size);
        cleanup();
        return -ENOMEM;
    }

// TODO: Add sync
    if ((ret = read_nv(prim_fd, &env, read_buf)) < 0) {
        fprintf(stderr, "warning: restoring primary flash...\n");
        if ((ret = read_nv(sec_fd, &env, read_buf)) < 0) {
            fprintf(stderr, "error: restore from secondary flash failed\n");
            env = (char **) malloc(16 * sizeof(char *));
            memset(env, 0, 16 * sizeof(char *));
        };
        if ((ret = write_env_ram(prim_fd, env, write_buf)) < 0) {
            fprintf(stderr, "error: primary flash restore failed\n");
            cleanup();
            return ret;
        }
        fprintf(stderr, "done\n");
    }
    if (argc < 2) {
        for (i = 0; i < ret; i++) {
            printf("%s\n", env[i]);
        }
    }

    if (argc > 2) {
        int j;
        int len = ret > 0 ? (env[ret - 1] - read_buf + strlen(env[ret - 1]) + 1) : 0;
        void *new_env = realloc(env, sizeof(char *) * (ret + argc / 2 + 1));
        if (new_env == NULL) {
            cleanup();
            return -ENOMEM;
        }
        env = (char **) new_env;
        for (j = 1; j + 1 < argc; j += 2) {
            int pos = -1;
            if ((len + strlen(argv[j]) + strlen(argv[j + 1]) + 5) > size) {
                fprintf(stderr, "error: %s=%s does not fit on env device\n", argv[j], argv[j + 1]);
                cleanup();
                return -ENOMEM;
            }
            for (i = 0; i < ret; i++) {
                if (!strncmp(env[i], argv[j], strlen(argv[j])) && env[i][strlen(argv[j])] == '=') {
                    pos = i;
                    break;
                }
            }
            if (pos < 0)
                pos = ret++;
            env[pos] = read_buf + len + 1;
            len += 1 + sprintf(read_buf + len + 1, "%s=%s", argv[j], argv[j + 1]);
            env[ret] = NULL;
        }
    }

    /*
     if (argc!=2)
     for (i=0;i<ret;i++){
     printf("<begin>%s<end>\n",env[i]);
     }
     */

    if (argc > 2) {
        ret = write_env_ram(sec_fd, env, write_buf);
        if (ret < 0) {
            fprintf(stderr, "error: secondary flash write failed\n");
            cleanup();
            return ret;
        }
        ret = write_env_ram(prim_fd, env, write_buf);
        if (ret < 0) {
            fprintf(stderr, "error: primary flash write failed\n");
            cleanup();
            return ret;
        }
    }

    cleanup();
    return 0;
}
