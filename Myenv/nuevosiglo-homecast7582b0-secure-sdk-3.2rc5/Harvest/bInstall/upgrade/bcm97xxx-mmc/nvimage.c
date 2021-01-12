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

static int size = -1;

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
#undef ENCRYPT_NVRAM
#else
#ifdef ENCRYPT_NVRAM
#include <openssl/aes.h>

static unsigned char nvram_key[32];

int enc_load_key(void)
{
    int fd = open("nvram_key", O_RDONLY);
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
    unsigned char* encrypted_buf = malloc(wsize);
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
    unsigned char* read_buf = malloc(rsize);
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
    memset(buffer, 0xff, size);
    /* prepare environment binary soup */
    for (i = 0; env[i]; i++) {
        if (buf - begin + strlen(env[i]) + 5 > size) {
            fprintf(stderr, "error: %s does not fit on env image\n", env[i]);
            return -ENOMEM;
        }
        if (strlen(env[i]) + 1 > 0xFF) {
            reclen = (int) strlen(env[i]) + 1;
            buf[0] = 0x02;
            buf[1] = (reclen & 0xFF00) >> 8;
            buf[2] = (reclen & 0x00FF);
            buf[3] = 0;
            buf += 4;
        } else {
            buf[0] = 1;
            buf[1] = (strlen(env[i]) + 1) & 0xFF;
            buf[2] = 0;
            buf += 3;
        }
        strcpy(buf, env[i]);
        buf += strlen(env[i]);
        buf[0] = 0;
        written++;
    }

    lseek(fd, 0L, SEEK_SET);
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
    int i;
    int len;
#ifdef ENCRYPT_NVRAM
    if (enc_read(fd, buf, size) < 0) {
        return -EIO;
    }
#else
    if (read(fd, buf, size) < 0) {
        return -EIO;
    }
#endif
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
    ptr = buf;
    len = 0;
    for (i = 0;; i++) {
        if (*ptr != 0x01 && *ptr != 0x02) {
            break;
        }
        if (*ptr & 0x01) {
            len = ((int) ptr[1]) & 0xFF;
            *ptr = 0;
            ptr += 3;
        } else {
            len = ((ptr[1] & 0xFF) << 8) + (ptr[2] & 0xFF);
            *ptr = 0;
            ptr += 4;
        }
        if (i >= (valnum - 1)) {
            valnum <<= 1;
            *env = (char **) realloc(*env, valnum * sizeof(char *));
            if (!*env) {
                fprintf(stderr, "error: cant realloc environment to %zd\n", valnum * sizeof(char *));
                return -ENOMEM;
            }
        }
        if ((int) (ptr - buf + len + 1) > size) {
            fprintf(stderr, "error: incorrect format (value #%d, pos %d,len %d)\n", i, (int) (ptr - buf), len);
            return -EINVAL;
        }
        (*env)[i] = ptr;
        (*env)[i + 1] = NULL;
        ptr += len - 1;
    }
    if (*(unsigned char *) ptr == 0xff)
        return -1;
    return i;
}

int main(int argc, char *argv[])
{
    int ret, i;
    struct stat filestat;

    if ((!(argc % 2) || argc < 2) && argc != 2) {
        fprintf(stderr, "usage:\n"
            "    %s image_filename image_size [variable] [value]\n"
            "    %s image_file", argv[0], argv[0]);
        return -EINVAL;
    }

#ifdef ENCRYPT_NVRAM
    if (enc_load_key() < 0) {
        fprintf(stderr, "error: cant read encryption key\n");
        return -EIO;
    }
#endif

    if ((prim_fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )) < 0) {
        fprintf(stderr, "error: cant open file %s: %s\n", argv[1], strerror(errno));
        cleanup();
        return errno;
    }
    if(argc == 2) {
        if(fstat(prim_fd, &filestat) != 0) {
            printf("Cannot stat file %s\n", argv[0]);
            cleanup();
            return errno;
        }
        size = filestat.st_size;
    } else {
        errno = 0;
        size = strtol(argv[2], NULL, 0);
        if(errno != 0) {
            printf("Invalid image size\n");
            return errno;
        }
    }

    if (!(read_buf = malloc(size)) || !(write_buf = malloc(size))) {
        fprintf(stderr, "error: cant allocate buffor(%d) memory\n", size);
        cleanup();
        return -ENOMEM;
    }

    if ((ret = read_nv(prim_fd, &env, read_buf)) < 0) {
        printf("NVRAM image empty\n");
        ret = 0;
    }
    if (argc == 2)
        for(i = 0; i < ret; i++) {
            printf("%s\n", env[i]);
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
        for(j = 3; j + 1 < argc; j += 2) {
            int pos = -1;
            if ((len + strlen(argv[j]) + strlen(argv[j + 1]) + 5) > size) {
                fprintf(stderr, "error: %s=%s does not fit on env image\n", argv[j], argv[j + 1]);
                cleanup();
                return -ENOMEM;
            }
            for(i = 0; i < ret; i++) {
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
