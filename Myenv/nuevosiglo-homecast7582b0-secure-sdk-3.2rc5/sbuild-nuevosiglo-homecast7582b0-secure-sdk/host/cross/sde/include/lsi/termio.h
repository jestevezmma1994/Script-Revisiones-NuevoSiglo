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
 * lsi/termio.h: ioctl/ttctl definitions for LSI PMON 
 */


#ifndef _LSI_TERMIO_H
#define _LSI_TERMIO_H

/* operation codes for ioctl */
#define TCGETA		1
#define TCSETAF		2
#define TCSETAW		3
/* SDE setjmp() incompatible with LSI */
/*#define SETINTR		4*/
#define SETSANE		5
#define FIONREAD	6
/*#define GETINTR		7*/
#define GETTERM		8
#define SETTERM		9
#define SETNCNE		10
#define CBREAK		11
#define TERMTYPE	12

/* iflags */
#define ISTRIP		0x0020
#define ICRNL		0x0040
#define IXON		0x0400
#define IXANY		0x0800
#define IXOFF		0x1000

/* oflags */
#define ONLCR		0x0004

/* lflags */
#define ISIG   		0x0001
#define ICANON		0x0002
#define ECHO		0x0008
#define ECHOE		0x0010

/* cflags */
#define	CBAUD		0000017		
#define	B0		0000000
#define	B50		0000001	
#define	B75		0000002
#define	B110		0000003
#define	B134		0000004
#define	B150		0000005
#define	B200		0000006
#define	B300		0000007
#define	B600		0000010
#define	B1200		0000011
#define	B1800		0000012
#define	B2400		0000013
#define	B4800		0000014
#define	B9600		0000015
#define	B19200		0000016
#define	B38400		0000017

/* cc definitions */
#define VINTR		0
#define VERASE		2
#define VEOL		5
#define VEOL2		6
#define V_START		8
#define V_STOP		9

/* operation codes for ttctl */
#define TT_CM		1	/* cursor movement */
#define TT_CLR		2	/* clear screen */
#define TT_CUROFF	3	/* switch cursor off */
#define TT_CURON	4	/* switch cursor on */

#ifndef __ASSEMBLER__

#define CNTRL(x) 	(x&0x1f)

#define NCC 23
struct termio {
    unsigned short	c_iflag;
    unsigned short	c_oflag;
    unsigned short	c_cflag;
    unsigned short	c_lflag;
    unsigned char	c_cc[NCC];
};

#endif /* !ASSEMBLER */

#endif /* _LSI_TERMIO_H */
