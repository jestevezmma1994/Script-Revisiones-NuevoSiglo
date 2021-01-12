/*	$NetBSD: pcivar.h,v 1.8 1995/06/18 01:26:50 cgd Exp $	*/

/*
 * Copyright (c) 1994 Charles Hannum.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Charles Hannum.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Definitions for PCI autoconfiguration.
 *
 * This file describes types and functions which are used for PCI
 * configuration.  Some of this information is machine-specific, and is
 * separated into pci_machdep.h.
 */

#ifdef IN_PMON
typedef unsigned char		u_int8_t;
typedef unsigned short		u_int16_t;
typedef unsigned long		u_int32_t;
typedef unsigned long long	u_int64_t;
typedef signed char		int8_t;
typedef short			int16_t;
typedef long			int32_t;
typedef long long		int64_t;
#else
#include <sys/types.h>
#endif

#ifndef NULL
#define NULL	(void *)0
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args)	args
#else
#define __P(args)	()
#endif
#endif

#if !defined(_KERNEL)
typedef u_int32_t		vm_offset_t;
typedef __typeof(sizeof(int))	vm_size_t;

/*
 * Configuration tag; created from a {bus,device,function} triplet by
 * pci_make_tag(), and passed to pci_conf_read() and pci_conf_write().
 * We could instead always pass the {bus,device,function} triplet to
 * the read and write routines, but this would cause extra overhead.
 *
 * Machines other than PCs are likely to use the equivalent of mode 1
 * tags always.  Mode 2 is historical and deprecated by the Revision
 * 2.0 specification.
 */
typedef u_int32_t pcitag_t;

/*
 * Type of a value read from or written to a configuration register.
 * Always 32 bits.
 */
typedef u_int32_t pcireg_t;
#endif

/* PCI interrupt levels; system interrupt levels for PCI bus use */
typedef enum {
	PCI_IPL_NONE,		/* block only the interrupt's IRQ*/
	PCI_IPL_BIO,		/* block I/O interrupts */
	PCI_IPL_NET,		/* network */
	PCI_IPL_TTY,		/* terminal */
	PCI_IPL_CLOCK,		/* clock */
} pci_intrlevel;

/* *BSD kernel has inocompatible version of pci_attach_args */
#ifndef _KERNEL
struct pci_attach_args {
	int pa_bus;
	int pa_device;
	int pa_function;
	pcitag_t pa_tag;
	pcireg_t pa_id, pa_class;
};
#endif

struct pci_match {
    pcireg_t	class, classmask;
    pcireg_t	id, idmask;
};

int	 _pci_hwinit __P((int));
void	 _pci_hwreinit __P((void));
pcitag_t _pci_make_tag __P((int, int, int));
void	 _pci_break_tag __P((pcitag_t, int *, int *, int *));
pcireg_t _pci_conf_read8 __P((pcitag_t, int));
void	 _pci_conf_write8 __P((pcitag_t, int, pcireg_t));
pcireg_t _pci_conf_read16 __P((pcitag_t, int));
void	 _pci_conf_write16 __P((pcitag_t, int, pcireg_t));
pcireg_t _pci_conf_read __P((pcitag_t, int));
void	 _pci_conf_write __P((pcitag_t, int, pcireg_t));
#define _pci_conf_read32 	_pci_conf_read
#define _pci_conf_write32	_pci_conf_write
pcireg_t _pci_conf_read __P((pcitag_t, int));
void	 _pci_conf_write __P((pcitag_t, int, pcireg_t));
pcireg_t _pci_statusread __P((void));
void	 _pci_statuswrite __P((pcireg_t));
int	 _pci_canscan __P((pcitag_t));
void	 _pci_flush __P((void));
void	 _pci_wbflush __P((void));
int	 _pci_map_io __P((pcitag_t, int, vm_offset_t *, vm_offset_t *));
int	 _pci_map_port __P((pcitag_t, int, unsigned int *));
int	 _pci_map_mem __P((pcitag_t, int, vm_offset_t *, vm_offset_t *));
int	 _pci_map_int __P((pcitag_t));
void	 _pci_devinfo __P((pcireg_t, pcireg_t, char *, int *));
pcireg_t _pci_allocate_mem __P((pcitag_t, vm_size_t));
pcireg_t _pci_allocate_io __P((pcitag_t, vm_size_t));
void	 _pci_configure __P((int));
void	 _pci_init __P((void));
pcitag_t _pci_find __P((const struct pci_match *, unsigned int));
vm_offset_t _pci_dmamap (vm_offset_t, unsigned int);
vm_offset_t _pci_cpumap (vm_offset_t, unsigned int);
int	 _pci_cacheline_log2 (void);
int	 _pci_maxburst_log2 (void);

void	 _pci_bdfprintf (int bus, int device, int function, const char *fmt, ...);
void	 _pci_tagprintf (pcitag_t tag, const char *fmt, ...);

/* PCI bus is often accompanied by ISA bus */
void *	_isa_map_io (unsigned int);
void *	_isa_map_mem (vm_offset_t);
vm_offset_t _isa_dmamap (vm_offset_t, unsigned int);
vm_offset_t _isa_cpumap (vm_offset_t, unsigned int);

#if 0
int	 pci_attach_subdev __P((int, int));
#endif

#ifdef IN_PMON
/* sigh... compatibility */
#define pci_hwinit _pci_hwinit
#define pci_hwreinit _pci_hwreinit
#define pci_make_tag _pci_make_tag
#define pci_break_tag _pci_break_tag
#define pci_conf_read _pci_conf_read
#define pci_conf_write _pci_conf_write
#define pci_map_port _pci_map_port
#define pci_map_io _pci_map_io
#define pci_map_mem _pci_map_mem
#define pci_map_int _pci_map_int
#define pci_devinfo _pci_devinfo
#define pci_configure _pci_configure
#define pci_allocate_mem _pci_allocate_mem
#define pci_allocate_io _pci_allocate_io
#endif

/* PCI bus parameters */
struct pci_bus {
    unsigned char	min_gnt;	/* largest min grant */
    unsigned char	max_lat;	/* smallest max latency */
    unsigned char	devsel;		/* slowest devsel */
    char		fast_b2b;	/* support fast b2b */
    char		prefetch;	/* support prefetch */
    char		freq66;		/* support 66MHz */
    char		width64;	/* 64 bit bus */
    int			bandwidth;	/* # of .25us ticks/sec @ 33MHz */
    unsigned char	ndev;		/* # devices on bus */
    unsigned char	def_ltim;	/* default ltim counter */
    unsigned char	max_ltim;	/* maximum ltim counter */
    unsigned char	hasvga;		/* VGA device detected on this bus */
    u_int8_t		primary;	/* primary bus number */
    pcitag_t		tag;		/* tag for this bus */
    u_int32_t		min_io_addr;	/* min I/O address allocated to bus */
    u_int32_t		max_io_addr;	/* max I/O address allocated to bus */
    u_int32_t 		min_mem_addr;	/* min mem address allocated to bus */
    u_int32_t 		max_mem_addr;	/* max mem address allocated to bus */
};

extern struct pci_bus _pci_bus[];
extern int _pci_nbus;
extern int _pci_maxbus;

/*
 * Descriptions of of known vendors and devices ("products").
 */
struct pci_knowndev {
    unsigned short	vendor;
    unsigned short	product;
    int			flags;
    const char		*const vendorname;
    const char		*const productname;
};
#define	PCI_KNOWNDEV_UNSUPP	0x01		/* unsupported device */
#define	PCI_KNOWNDEV_NOPROD	0x02		/* match on vendor only */

const struct pci_knowndev * _pci_lookup (unsigned int, unsigned int);
