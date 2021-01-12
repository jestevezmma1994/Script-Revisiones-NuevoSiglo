/*
 * Copyright (c) 1996-2003 MIPS Technologies, Inc.  All rights reserved.
 * 
 * Unpublished rights (if any) reserved under the copyright laws of the
 * United States of America and other countries.
 * 
 * This code is proprietary to MIPS Technologies, Inc. ("MIPS
 * Technologies").  Any copying, reproducing, modifying or use of this
 * code (in whole or in part) that is not expressly permitted in writing
 * by MIPS Technologies or an authorized third party is strictly
 * prohibited.  At a minimum, this code is protected under unfair
 * competition and copyright laws.  Violations thereof may result in
 * criminal penalties and fines.
 * 
 * MIPS Technologies reserves the right to change this code to improve
 * function, design or otherwise.  MIPS Technologies does not assume any
 * liability arising out of the application or use of this code, or of
 * any error or omission in such code.  Any warranties, whether express,
 * statutory, implied or otherwise, including but not limited to the
 * implied warranties of merchantability or fitness for a particular
 * purpose, are excluded.  Except as expressly provided in any written
 * license agreement from MIPS Technologies or an authorized third party,
 * the furnishing of this code does not give recipient any license to any
 * intellectual property rights, including any patent rights, that cover
 * this code.
 * 
 * This code shall not be exported or transferred for the purpose of
 * reexporting in violation of any U.S. or non-U.S. regulation, treaty,
 * Executive Order, law, statute, amendment or supplement thereto.
 * 
 * This code constitutes one or more of the following: commercial
 * computer software, commercial computer software documentation or other
 * commercial items.  If the user of this code, or any related
 * documentation of any kind, including related technical data or
 * manuals, is an agency, department, or other entity of the United
 * States government ("Government"), the use, duplication, reproduction,
 * release, modification, disclosure, or transfer of this code, or any
 * related documentation of any kind, is restricted in accordance with
 * Federal Acquisition Regulation 12.212 for civilian agencies and
 * Defense Federal Acquisition Regulation Supplement 227.7202 for
 * military agencies.  The use of this code by the Government is further
 * restricted in accordance with the terms of the license agreement(s)
 * and/or applicable contract terms and conditions covering this code
 * from MIPS Technologies or an authorized third party.
 */

/*
 * stdio.h : standard I/O library definitions
 */


#ifndef __STDIO_H
#ifdef __cplusplus
extern "C" {
#endif
#define __STDIO_H

/* _POSIX_SOURCE features in here */
#include <sys/posix.h>

/* get __va_list definition */
#define __need___va_list
#include <stdarg.h>

/* --- Inclusions --- */
#include <stddef.h>		/* bogus gets offsetof */

#if !defined(_ANSI_SOURCE) || defined(_POSIX_SOURCE)
#include <sys/types.h>
#endif

/* --- Constants --- */
#define EOF	(-1)
#define BUFSIZ	1024

#define _IOFBF		0
#define _IOLBF		_IO_LBUF
#define _IONBF		(-1)
#define L_tmpnam	(4 + 1 + FILENAME_MAX) /* "/tmp/" + file name + '\0' */
#define TMP_MAX		32767		/* Arbitrary! */
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

/* XXX problems here */
#define FILENAME_MAX	1024	/* includes room for trailing '\0' ?? */
#define FOPEN_MAX	INT_MAX

#include <libc_thread.h>

#if !defined(_ANSI_SOURCE) || defined(_POSIX_SOURCE)
typedef off_t			fpos_t;
#else
__extension__ typedef long long fpos_t;
#endif

typedef struct _iob {
  int			_io_count;
  union _iou {
      unsigned char	*_io_ptr;
      int		_io_char;
  } _io;
  unsigned char		*_io_buf;
  int			_io_bufsiz;
  unsigned short	_io_flags;
  short			_io_fd;
  void			*_io_lock;
} FILE;

  /* --- Flag bits --- */
#define _IO_READ	0x0001
#define _IO_WRITE	0x0002
#define _IO_UPDATE	0x0004
#define _IO_LBUF	0x0008
#define _IO_MYBUF	0x0010
#define _IO_EOF		0x0020
#define _IO_ERR		0x0040
#define _IO_STRING	0x0080
/*#define _IO_TEMP	0x0100*/
#define _IO_STRALLOC	0x0200
#define _IO_TTY		0x1000
#define _IO_BINARY	0x8000

#define _io_testflag(f, flag)	((f)->_io_flags & (flag))
#define _io_setflag(f, flag)	((f)->_io_flags |= (flag))
#define _io_clearflag(f, flag)	((f)->_io_flags &= ~(flag))

/* --- Prototypes --- */

  /* --- Streams: open and close --- */
int	fclose	(FILE *);
FILE	*fopen	(const char *, const char *);
FILE	*freopen (const char *, const char *, FILE *);
FILE	*tmpfile (void);

  /* --- Streams: input --- */
int	fgetc	(FILE *);
char	*fgets	(char *, int , FILE *);
size_t	fread	(void *, size_t, size_t, FILE *);
int	fscanf	(FILE *, const char *, ...);
int	getc	(FILE *);
int	getchar	(void);
char	*gets	(char *);
int	scanf	(const char *, ...);
int	sscanf	(const char *, const char *, ...);
int	ungetc	(int, FILE *);

#if _POSIX_VERSION >= 199309L
int	getc_unlocked(FILE *);
int	getchar_unlocked(void);
#endif

  /* --- Streams: output --- */
int	fprintf	(FILE *, const char *, ...);
int	fputc	(int , FILE *);
int	fputs	(const char *, FILE *);
size_t	fwrite	(const void *, size_t, size_t, FILE *);
int	printf	(const char *, ...);
int	putc	(int, FILE *);
int	putchar	(int);
int	puts	(const char *);
int	sprintf	(char *, const char *, ...);

#if _POSIX_VERSION >= 199309L
int	putc_unlocked(int, FILE *);
int	putchar_unlocked(int);
#endif

int	vfprintf (FILE *, const char *, __va_list);
int	vprintf	(const char *, __va_list);
int	vsprintf (char *, const char *, __va_list);

int	vscanf (const char *, __va_list);
int	vsscanf (const char *, const char *, __va_list);
int	vfscanf (FILE *, const char *, __va_list);

#if !defined(_POSIX_SOURCE) && !defined(_ANSI_SOURCE)
int	snprintf (char *, size_t, const char *, ...);
int	vsnprintf (char *, size_t, const char *, __va_list);
int	asprintf (char **, const char *, ...);
int	vasprintf (char **, const char *, __va_list);
#endif
int	__snprintf (char *, size_t, const char *, ...);
int	__vsnprintf (char *, size_t, const char *, __va_list);

/* --- Streams: inquiry and control --- */
void	clearerr (FILE *);
int	feof	(FILE *);
int	ferror	(FILE *);
int	fflush	(FILE *);
int	fgetpos	(FILE *, fpos_t *);
int	fseek	(FILE *, long int, int);
int	fsetpos	(FILE *, const fpos_t *);
long int ftell	(FILE *);
void	rewind	(FILE *);
void	setbuf	(FILE *, char *);
int	setvbuf	(FILE *, char *, int, size_t);

#if _POSIX_VERSION >= 199309L
void	flockfile(FILE *);
void	funlockfile(FILE *);
int 	ftrylockfile(FILE *);
void	_flockfree(FILE *);
#endif

  /* --- File system manipulation --- */
int	remove	(const char *);
int	rename	(const char *, const char *);

  /* --- Miscellaneous --- */
char	*tmpnam	(char *s);
void	perror	(const char *s);
#ifndef _POSIX_SOURCE
extern int sys_nerr;
extern const char * const sys_errlist[];
#endif

#ifndef _ANSI_SOURCE
#define L_cuserid	(32+1)		/* Arbitrary hmm */
#define L_ctermid	FILENAME_MAX
#define STREAM_MAX	INT_MAX

int	fileno(FILE *);
FILE	*fdopen(int, const char *);
char	*ctermid(char *);
char	*ctermid_r(char *);
#endif

/* internal for use by macros */
extern int	_filbuf(FILE *);
extern int	_flsbuf(int, FILE *);
extern int	_fwalk (int (*) (FILE *));

/* --- Streams: input --- */
#define getc(f)		fgetc(f)
#define getchar()	getc(stdin)
#define putc(c, f)	fputc(c, f)
#define putchar(c)	putc(c, stdout)

#if _POSIX_VERSION >= 199309L

#define getc_unlocked(f)						\
  ((--(f)->_io_count >= 0) ? *(f)->_io._io_ptr++ : _filbuf(f))

/* evaluate c once only, using GNU statement expression */
#define putc_unlocked(c, f)						\
  __extension__ ({int __c = (c);				      	\
   ((--(f)->_io_count < 0) || (_io_testflag(f, _IO_LBUF) && __c == '\n')) \
	? _flsbuf(__c, f)						\
	: (int) (*(f)->_io._io_ptr++ = __c);				\
  })

#define getchar_unlocked()	getc_unlocked(stdin)
#define putchar_unlocked(c)	putc_unlocked(c, stdout)

#endif /* _POSIX_C_SOURCE >= 199309L */

  /* --- Streams: inquiry and control --- */
#define feof(f)		(_io_testflag((f), _IO_EOF) != 0)
#define ferror(f)	(_io_testflag((f), _IO_ERR) != 0)

#if _POSIX_VERSION >= 199309L
#define clearerr_unlocked(f) ((void)_io_clearflag((f), (_IO_EOF | _IO_ERR)))
#endif
void	clearerr(FILE *);

#ifdef _POSIX_SOURCE
#define fileno(f)	((f)->_io_fd)
#endif

#ifdef __cplusplus
}
#endif
#endif /* !defined __STDIO_H */
