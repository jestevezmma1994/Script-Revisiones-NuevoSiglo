/*
 * Copyright (c) 2000-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * sys/panelio.h: panel display ioctl interface
 */


#ifndef _sys_panelio_H_
#define _sys_panelio_H_

#ifdef __cplusplus
extern "C" {
#endif

struct panelinfo {
    unsigned char	type;
    unsigned char	flags;
    unsigned char	rows;
    unsigned char	cols;
};

#define PANELTYPE_ALPHA		0	/* alphanumeric display */
#define PANELTYPE_HEX		1	/* hex display */
#define PANELTYPE_LED		2	/* individual leds */

#define PANELFLGS_BRIGHTNESS	0x01	/* can set brightness */
#define PANELFLGS_CONTRAST	0x02	/* can set contrast */
#define PANELFLGS_BLINK		0x04	/* display can be blinked */
#define PANELFLGS_FLASH		0x08	/* chars can be flashed */
#define PANELFLGS_SCROLL	0x10	/* display can be scrolled */
#define PANELFLGS_PROGRESS	0x20	/* display can show progress */

struct panelmode {
    unsigned char	options;	/* display options */
    unsigned char	brighton;	/* brightness (0 to 100%) */
    unsigned char	brightoff;	/* brightness (0 to 100%) */
    unsigned char	contrast;	/* contrast (0 to 100%) */
    unsigned long	blinkon;	/* on period in ns */
    unsigned long	blinkoff;	/* off period in ns */
    unsigned long	scrollrate;	/* scroll rate in ns */
    int			scrollchars;	/* scroll amount */
};

#define PANELOPT_PAD	0x01		/* clear to end of row */
#define PANELOPT_CENTRE	0x02		/* centre within row  */
#define PANELOPT_WRAP	0x04		/* wrap at end of row */
#define PANELOPT_IGNLF	0x08		/* ignore linefeed char */
#define PANELOPT_IGNNUL	0x10		/* ignore null char */
#define PANELOPT_ROTATE	0x20		/* rotate text until rewritten */
#define PANELOPT_FADE	0x40		/* slow blinking/flashing */
#define PANELOPT_FLASH	0x80		/* next chars flash */

struct panelcoord {
    unsigned short	row;		
    unsigned short	col;
};

#define	PANELIOINFO	 _IOR('p', 0, struct panelinfo)	/* get info  */
#define	PANELIOGMODE	 _IOR('p', 1, struct panelmode) /* get mode */
#define	PANELIOSMODE	 _IOW('p', 2, struct panelmode) /* set mode */
#define	PANELIOCLEAR	 _IO('p', 3) 			/* clear display */
#define	PANELIOPROGRESS	 _IOW('p', 4, int) 		/* display progress */
#define	PANELIOSCOORD	 _IOW('p', 5, struct panelcoord) /* next display coord */

#ifdef __cplusplus
}
#endif

#endif /* _sys_panelio_H_ */
