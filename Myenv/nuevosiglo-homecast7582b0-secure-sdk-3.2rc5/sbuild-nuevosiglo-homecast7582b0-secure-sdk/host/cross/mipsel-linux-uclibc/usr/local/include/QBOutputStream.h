/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_OUTPUT_STREAM_H_
#define QB_OUTPUT_STREAM_H_

/**
 * @file QBOutputStream.h
 * @brief Buffered output stream
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <SvCore/SvCommonDefs.h>
#include <QBUTF8Utils.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBOutputStream Buffered output stream
 * @ingroup QBStringUtils
 * @{
 *
 * Buffered output stream for writing to files or memory buffers.
 **/

/**
 * Status of the output stream.
 **/
typedef enum {
    /** no error */
    QBOutputStreamStatus_OK = 0,
    /** unsufficient memory for internal stream buffer */
    QBOutputStreamStatus_noMemory,
    /** there was an overflow when writing to the stream */
    QBOutputStreamStatus_overflow,
    /** could not write buffered data to the file */
    QBOutputStreamStatus_ioError,
    /** invalid data written to the stream */
    QBOutputStreamStatus_invalidData,
} QBOutputStreamStatus;

/**
 * Buffered output stream.
 **/
typedef struct QBOutputStream_ *QBOutputStream;
struct QBOutputStream_ {
    /** current status of the stream */
    QBOutputStreamStatus status;

    /** input buffer */
    struct {
        /** buffer space */
        uint8_t *data;
        /** offset of the free space from the beginning of QBOutputStream::buffer::data */
        size_t used;
        /** number of free bytes left in QBOutputStream::buffer::data */
        size_t free;
    } buffer;
} __attribute__ ((aligned(8)));


/**
 * Create file backed output stream.
 *
 * This method creates a buffered output stream backed by a file.
 * If file at @a path exists, it will be truncated; otherwise it
 * will be created with @c 0644 permissions. All data written to
 * the stream will be written to the underlying file when
 * QBOutputStreamFlush() is called. File will be closed
 * by QBOutputStreamDestroy().
 *
 * @param[in] path          file path
 * @param[in] maxLength     max number of bytes that can be written
 *                          to the output stream, @c 0 for no limit
 * @return                  new output stream, @c NULL in case of error
 **/
extern QBOutputStream
QBOutputStreamCreateWithFilePath(const char *path,
                                 off_t maxLength);

/**
 * Create file backed output stream.
 *
 * This method is similar to QBOutputStreamCreateWithFilePath().
 * Created output stream is backed by a file or other operating system
 * object represented by the descriptor. This descriptor is <b>not</b>
 * closed by QBOutputStreamDestroy().
 *
 * @param[in] fd            descriptor opened for writing
 * @param[in] maxLength     max number of bytes that can be written
 *                          to the output stream, @c -1 for no limit
 * @return                  new output stream, @c NULL in case of error
 **/
extern QBOutputStream
QBOutputStreamCreateWithFile(int fd,
                             off_t maxLength);

/**
 * Create memory backed output stream.
 *
 * This method creates a buffered output stream backed by memory.
 * Buffers will be allocated in chunks of automatically determined size
 * as needed.
 * You can pass a pointer to an already allocated block of memory
 * to use as a first chunk. All memory allocated by the stream
 * will be freed by QBOutputStreamDestroy().
 *
 * @param[in] data          first buffer to use, can be @c NULL
 * @param[in] length        size of @a data buffer if not @c NULL
 * @param[in] maxLength     max number of bytes that can be written
 *                          to the output stream, @c -1 for no limit
 * @return                  new output stream, @c NULL in case of error
 **/
extern QBOutputStream
QBOutputStreamCreateWithMemory(uint8_t *data,
                               size_t length,
                               off_t maxLength);

/**
 * Destroy output stream.
 *
 * @param[in] self          output stream handle
 **/
extern void
QBOutputStreamDestroy(QBOutputStream self);

/**
 * Get current status of the stream.
 *
 * @param[in] self          output stream handle
 * @return                  stream status
 **/
static inline QBOutputStreamStatus
QBOutputStreamGetStatus(QBOutputStream self)
{
    assert(self != NULL);
    return self->status;
}

/**
 * Get current position in the stream: number of bytes written
 * to the stream so far.
 *
 * @param[in] self          output stream handle
 * @return                  position in the stream, @c -1 in case of error
 **/
extern off_t
QBOutputStreamGetPosition(QBOutputStream self);

/**
 * Get internal storage of memory backed output stream.
 *
 * @param[in] self          output stream handle
 * @param[in] iov           an array of iovec structures
 * @param[in] iovCnt        number of items in @a iov array
 * @return                  number of iovec structures needed to return
 *                          full contents of the memory backed output stream
 *                          (can be higher than @a iovCnt),
 *                          @c -1 in case of error
 **/
extern int
QBOutputStreamGetStorage(QBOutputStream self,
                         struct iovec *iov,
                         unsigned int iovCnt);

/**
 * Get contents of memory backed output stream
 * as a NULL-terminated string.
 *
 * @param[in] self          output stream handle
 * @param[out] length       length of the returned string
 * @return                  newly allocated string holding the contents
 *                          written to the output stream,
 *                          @c NULL in case of error
 **/
extern char *
QBOutputStreamCreateCString(QBOutputStream self,
                            size_t *length);

/**
 * Write raw data to the output stream.
 *
 * @param[in] self          output stream handle
 * @param[in] data          buffer with data to write
 * @param[in] size          number of bytes of @a data
 **/
extern void
QBOutputStreamWriteData(QBOutputStream self,
                        const uint8_t *data,
                        size_t size);

/**
 * Write NULL-terminated C string to the output stream.
 *
 * @param[in] self          output stream handle
 * @param[in] str           NULL-terminated C string
 **/
extern void
QBOutputStreamWriteCString(QBOutputStream self,
                           const char *str);

/**
 * Write part of a NULL-terminated C string to the output stream.
 *
 * @param[in] self          output stream handle
 * @param[in] str           NULL-terminated C string
 * @param[in] length        number of bytes from @a str
 *                          to write to output stream
 **/
extern void
QBOutputStreamWriteCStringWithLength(QBOutputStream self,
                                     const char *str,
                                     size_t length);

/**
 * Write formatted string to the output stream in sprintf() style.
 *
 * @param[in] self          output stream handle
 * @param[in] format        string format like in printf()
 **/
extern void
QBOutputStreamWriteFormatted(QBOutputStream self,
                             const char *format,
                             ...) __attribute__ ((format(printf, 2, 3)));

/**
 * Write formatted string to the output stream in sprintf() style
 * with variable arguments list.
 *
 * @param[in] self          output stream handle
 * @param[in] format        string format like in printf()
 * @param[in] args          arguments list
 **/
extern void
QBOutputStreamWriteFormattedV(QBOutputStream self,
                              const char *format,
                              va_list args);

/**
 * Write wide character to the output stream.
 *
 * @param[in] self          output stream handle
 * @param[in] charCode      character code
 **/
extern void
QBOutputStreamWriteWideChar(QBOutputStream self,
                            uint32_t charCode);

/**
 * Increase size of the internal buffer of the output stream.
 *
 * @param[in] self          output stream handle
 * @param[in] size          desired buffer size
 * @return                  pointer to the expanded buffer,
 *                          @c NULL in case of error
 **/
extern uint8_t *
QBOutputStreamExpandBuffer(QBOutputStream self,
                           size_t size);

/**
 * Move data from the internal buffer of the output stream
 * to the underlying storage (for example: write to a file).
 *
 * @param[in] self          output stream handle
 **/
extern void
QBOutputStreamFlush(QBOutputStream self);


// inline methods for quick access to internal buffer of the stream

/**
 * Get direct pointer to internal buffer of the output stream.
 *
 * This method provides direct access to the internal buffer
 * of the output stream, expanding the buffer to desired size
 * if necessary. After writing your output data to the buffer
 * use QBOutputStreamCommitData().
 *
 * @param[in] self          output stream handle
 * @param[in] size          number of bytes to be written
 * @return                  pointer to the internal buffer,
 *                          @c NULL in case of error
 **/
static inline uint8_t *
QBOutputStreamGetInputBuffer(QBOutputStream self,
                             size_t size)
{
    assert(self != NULL);

    if (likely(self->buffer.free >= size))
        return self->buffer.data + self->buffer.used;

    QBOutputStreamFlush(self);
    if (self->buffer.free < size)
        return QBOutputStreamExpandBuffer(self, size);
    return self->buffer.data + self->buffer.used;
}

/**
 * Mark a buffer returned by QBOutputStreamGetInputBuffer()
 * (or a part of it) as written.
 *
 * @param[in] self          output stream handle
 * @param[in] size          number of bytes actually written
 *                          to the internal buffer
 **/
static inline void
QBOutputStreamCommitData(QBOutputStream self,
                         size_t size)
{
    assert(self != NULL);
    assert(size <= self->buffer.free);

    self->buffer.used += size;
    if (unlikely((self->buffer.free -= size) == 0))
        QBOutputStreamFlush(self);
}

/**
 * Write single byte to the output stream.
 *
 * @param[in] self          output stream handle
 * @param[in] b             byte value to append
 **/
static inline void
QBOutputStreamWriteByte(QBOutputStream self,
                        uint8_t b)
{
    assert(self != NULL);

    if (unlikely(self->buffer.free == 0))
        QBOutputStreamFlush(self);

    self->buffer.data[self->buffer.used++] = b;
    self->buffer.free -= 1;
}

/**
 * Write single character to the output stream.
 *
 * @param[in] self          output stream handle
 * @param[in] charCode      character code
 **/
static inline void
QBOutputStreamWriteChar(QBOutputStream self,
                        uint32_t charCode)
{
    assert(self != NULL);

    if (likely(charCode <= 0x7fu && self->buffer.free > 0)) {
        self->buffer.data[self->buffer.used++] = charCode;
        self->buffer.free -= 1;
    } else {
        QBOutputStreamWriteWideChar(self, charCode);
    }
}


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
