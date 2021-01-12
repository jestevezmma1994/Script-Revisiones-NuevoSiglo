/*
 * Copyright (c) 1999-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * elfmips.h: MIPS-specific ELF definitions
 */


#include <sys/elf.h>

/* MIPSABI program header special */
#define PT_MIPS_REGINFO		(PT_LOPROC + 0)

/* MIPSABI relocation (rel type) */
#define R_MIPS_NONE		0
#define R_MIPS_16		1
#define R_MIPS_32		2
#define R_MIPS_REL32		3
#define R_MIPS_26		4
#define R_MIPS_HI16		5
#define R_MIPS_LO16		6
#define R_MIPS_GPREL16		7
#define R_MIPS_LITERAL		8
#define R_MIPS_GOT16		9
#define R_MIPS_PC16		10
#define R_MIPS_CALL16		11
#define R_MIPS_GPREL32		12

/* old MIPS SDE relocation (rela type) */
#define R_SDE_8			0
#define R_SDE_16		1
#define R_SDE_32		2
#define R_SDE_ILA		35
#define R_SDE_IGPREL		36
#define R_SDE_ILAHI		37
#define R_SDE_ILALO		38
#define R_SDE_IBRA		39
#define R_SDE_IJMP		40
#define R_SDE_ILITERAL		41

/* MIPSABI special section numbers */
#define SHN_MIPS_ACOMMON	(SHN_LOPROC + 0)
#define SHN_MIPS_SCOMMON	(SHN_LOPROC + 3)
#define SHN_MIPS_SUNDEFINED	(SHN_LOPROC + 4)

/* MIPSABI special section types */
#define SHT_MIPS_LIBLIST	(SHT_LOPROC + 0)
#define SHT_MIPS_CONFLICT	(SHT_LOPROC + 2)
#define SHT_MIPS_GPTAB		(SHT_LOPROC + 3)
#define SHT_MIPS_UCODE		(SHT_LOPROC + 4)
#define SHT_MIPS_DEBUG		(SHT_LOPROC + 5)
#define SHT_MIPS_REGINFO	(SHT_LOPROC + 6)

/* MIPSABI special section flags */
#define SHF_MIPS_GPREL		0x10000000

/* MIPSABI processor specific flags */
#define EF_MIPS_NOREORDER	0x00000001
#define EF_MIPS_PIC		0x00000002
#define EF_MIPS_CPIC		0x00000004
#define EF_MIPS_ARCH		0xf0000000
#define  E_MIPS_ARCH_1		0x00000000 /* -mips1 */
#define  E_MIPS_ARCH_2		0x10000000 /* -mips2 */
#define  E_MIPS_ARCH_3		0x20000000 /* -mips3 */
#define  E_MIPS_ARCH_4		0x30000000 /* -mips4 */
#define  E_MIPS_ARCH_5		0x40000000 /* -mips5 */
#define  E_MIPS_ARCH_32		0x40000000 /* -mips32 */
#define  E_MIPS_ARCH_64		0x40000000 /* -mips64 */
/* The following are not MIPSABI, but MIPS SDE only */
#define EF_MIPS_CPU		0x00ff0000
#define  E_MIPS_CPU_STD		0x00000000       /* Base MIPS ISA */          
#define  E_MIPS_CPU_CW4		0x00010000       /* LSI CW401x */             
#define  E_MIPS_CPU_R4100	0x00020000       /* NEC Vr41xx */             
#define  E_MIPS_CPU_R4650	0x00030000       /* IDT R4650/4640 */         
#define  E_MIPS_CPU_R3900	0x00040000       /* Toshiba/Philips Rx900 */  
#define  E_MIPS_CPU_RM52XX	0x00050000       /* QED RM52xx/RM7000 */      
#define  E_MIPS_CPU_R54XX	0x00060000       /* NEC Vr54xx */             
#define  E_MIPS_CPU_R4320	0x00070000       /* NEC Vr4320 */             
#define  E_MIPS_CPU_RC3236X	0x00080000       /* IDT RC3236X */            
#define  E_MIPS_CPU_CW4001	0x00090000       /* LSI CW400x */             
#define EF_MIPS_NOGPCOUNT	0x00001000
#define EF_MIPS_GP32		0x04000000	/* -mips3+ -mgp32 */
#define EF_MIPS_FP32		0x08000000	/* -mips3+ -mfp32 */

/* MIPSABI special sections */

/* .reginfo */
typedef struct {
    Elf32_Word		ri_gprmask;
    Elf32_Word		ri_cprmask[4];
    Elf32_Sword		ri_gp_value;
} Elf32_Reginfo;

#define ELFREGINFOSZ	sizeof(Elf32_Reginfo)

size_t elfReginfoWrite(FILE *,const Elf32_Ehdr *,const Elf32_Shdr *,
		       const Elf32_Reginfo *,size_t);
size_t elfReginfoRead(FILE *,const Elf32_Ehdr *,const Elf32_Shdr *,
		      Elf32_Reginfo *,size_t);

/* .gptab.xxx */
typedef union {
    struct {
	Elf32_Word		gt_current_g_value;
	Elf32_Word		gt_unused;
    } gt_header;
    struct {
	Elf32_Word		gt_g_value;
	Elf32_Word		gt_bytes;
    } gt_entry;
} Elf32_Gptab;

#define ELFGPTABSZ	sizeof(Elf32_Gptab)

size_t elfGptabRead(FILE *,const Elf32_Ehdr *,const Elf32_Shdr *,
		    Elf32_Gptab *,size_t);
size_t elfGptabWrite(FILE *,const Elf32_Ehdr *,const Elf32_Shdr *,
		   const Elf32_Gptab *,size_t);

