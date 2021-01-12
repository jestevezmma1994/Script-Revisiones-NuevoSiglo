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
 * rc3233x.h: IDT RC3233x PCI and system controller
 */


#ifndef __RC3233X_H__
#define __RC3233X_H__

#ifdef __ASSEMBLER__

/* offsets from base register, assume already base reg already loaded */
#define RC3233X(offs)		(offs)

#else /* !__ASSEMBLER */

/* offsets from base pointer, this construct allows optimisation */
#ifdef RC3233X_BASE
static char * const _rc3233xp = PA_TO_KVA1(RC3233X_BASE);
#else
static char * const _rc3233xp = PA_TO_KVA1(0x18000000);
#endif

#define RC3233X(offs)		*(volatile unsigned int *)(_rc3233xp + (offs))

#endif

/* Bus Turnaround */
#define RC33X_BTA		RC3233X(0x0000)

/* Address Latch Timing */
#define RC33X_ALT		RC3233X(0x0004)	
# define RC33X_ALT_DRAM		 0x00000004
# define RC33X_ALT_MEM		 0x00000002
# define RC33X_ALT_IP		 0x00000001

/* Arbitration */
#define RC33X_ARB		RC3233X(0x0008)	
# define RC33X_ARB_ROUNDROBIN	 0x00000001
# define RC33X_ARB_FIXED	 0x00000000

/* BusError Control */
#define RC33X_BERRC		RC3233X(0x0010)
# define RC33X_BERRC_EXC_DIS	 0x00000080 	/* disable buserr exception */
# define RC33X_BERRC_WDOG_ENB	 0x00000040	/* enable watchdog */
# define RC33X_BERRC_WDOG_RST	 0x00000020	/* watchdog caused reset */
# define RC33X_BERRC_CPU_ENB	 0x00000010	/* enable CPU bus errs */
# define RC33X_BERRC_IP_ENB	 0x00000008	/* enable IP bus errs */
# define RC33X_BERRC_CPU_SRC	 0x00000004	/* CPU cause error */
# define RC33X_BERRC_IP_SRC	 0x00000002	/* IP caused error */
# define RC33X_BERRC_READ	 0x00000001	/* was a read (not a write) */

/* BusError Address */
#define RC33X_BERRA		RC3233X(0x0014)

/* SysID */
#define RC33X_SYSID		RC3233X(0x0018)
#define  RC33X_SYSID_VENDOR	 0xfff00000
#define   RC33X_SYSID_VEND_IDT	  (0x000 << 20)
#define  RC33X_SYSID_PART	 0x000fff00
#define   RC33X_SYSID_PART_RC334  (0x002 << 8)
#define   RC33X_SYSID_PART_RC332  (0x004 << 8)
#define  RC33X_SYSID_MAJOR	 0x000000f0
#define  RC33X_SYSID_MINOR	 0x0000000f

/* EPROM/Flash/SRAM memory region Base and Mask */
#define RC33X_MEM_ADR_0		RC3233X(0x0080)
#define RC33X_MEM_MSK_0		RC3233X(0x0084)
#define RC33X_MEM_ADR_1		RC3233X(0x0088)
#define RC33X_MEM_MSK_1		RC3233X(0x008c)

/* Device region control registers */
#define RC33X_MEM_CTL_0		RC3233X(0x0200)
#define RC33X_MEM_CTL_1		RC3233X(0x0204)
#define RC33X_MEM_CTL_2		RC3233X(0x0208)
#define RC33X_MEM_CTL_3		RC3233X(0x020c)
#define RC33X_MEM_CTL_4		RC3233X(0x0210)
#define RC33X_MEM_CTL_5		RC3233X(0x0214)

#define RC33X_MEM_CTL_TYPE	0xc000	/* memory type */
# define RC33X_MEM_CTL_TYPE_FLASH	0x0000	/* flash memory */
# define RC33X_MEM_CTL_TYPE_SRAM	0x0000	/* sram */
# define RC33X_MEM_CTL_TYPE_INTEL	0x4000	/* intel signalling */
# define RC33X_MEM_CTL_TYPE_MOTO	0x8000	/* motorola signalling */
# define RC33X_MEM_CTL_TYPE_DUALPORT	0xc000	/* dual-port ram */
#define RC33X_MEM_CTL_245_ENB	0x2000	/* mem_345_oe_n signal used */
#define RC33X_MEM_CTL_WP	0x1000	/* write-protect */
#define RC33X_MEM_CTL_WIDTH	0x0c00	/* width */
#define  RC33X_MEM_CTL_WIDTH_8	 0x0000
#define  RC33X_MEM_CTL_WIDTH_16	 0x0400
#define  RC33X_MEM_CTL_WIDTH_32	 0x0800
#define RC33X_MEM_CTL_WRWS_MASK	0x03e0	/* write wait states */
#define RC33X_MEM_CTL_WRWS_SHFT	5
#define RC33X_MEM_CTL_RDWS_MASK	0x001f	/* read wait states */
#define RC33X_MEM_CTL_RDWS_SHFT	0

/* DRAM memory region Base and Mask */
#define RC33X_DRAM_ADR_0	RC3233X(0x00c0)
#define RC33X_DRAM_MSK_0	RC3233X(0x00c4)
#define RC33X_DRAM_ADR_1	RC3233X(0x00c8)
#define RC33X_DRAM_MSK_1	RC3233X(0x00cc)
#define RC33X_DRAM_ADR_2	RC3233X(0x00d0)
#define RC33X_DRAM_MSK_2	RC3233X(0x00d4)
#define RC33X_DRAM_ADR_3	RC3233X(0x00d8)
#define RC33X_DRAM_MSK_3	RC3233X(0x00dc)

/* SDRAM Control Register */
#define RC33X_SDRAM		RC3233X(0x0300)
#define  RC33X_SDRAM_ENB	 0x80000000	/* enable SDRAM */
#define  RC33X_SDRAM_RASMUX	 0x60000000	/* RAS Mux: */
#define   RC33X_SDRAM_RASMUX_ED0  0x60000000	  /* shift 0: EDO */
#define   RC33X_SDRAM_RASMUX_4M4  0x40000000	  /* shift 1: 4Mx4 */
#define   RC33X_SDRAM_RASMUX_2M8  0x20000000	  /* shift 2: 2Mx8 */
#define   RC33X_SDRAM_RASMUX_1M16 0x00000000	  /* shift 3: 1Mx16 */
#define  RC33X_SDRAM_BANKSZ	 0x10000000	/* bank size: */
#define  RC33X_SDRAM_BANKSZ_16M	  0x10000000	  /* 16Mbit (2 banks) */
#define  RC33X_SDRAM_BANKSZ_64M	  0x00000000	  /* 64Mbit (4 banks) */
#define  RC33X_SDRAM_CASLAT_MASK 0x0c000000	/* CAS latency (1-3 clocks) */
#define  RC33X_SDRAM_CASLAT_SHFT 26
#define  RC33X_SDRAM_TRCD_MASK	 0x03000000	/* tRCD (1-3 clocks) */
#define  RC33X_SDRAM_TRCD_SHFT	 24
#define  RC33X_SDRAM_AUTO_PRECHG 0x00400000	/* auto precharge/page mode */
#define  RC33X_SDRAM_TRP_MASK	 0x00300000	/* tRP (2-4 clocks) */
#define  RC33X_SDRAM_TRP_SHFT	 20
#define  RC33X_SDRAM_TRC_MASK	 0x000f0000	/* tRC (3-11 clocks) */
#define  RC33X_SDRAM_TRC_SHFT	 16
#define  RC33X_SDRAM_SODIMM	 0x00008000	/* SODIMM enable */
#define  RC33X_SDRAM_OPCLK	 0x00004000	/* enable output_clk */
#define  RC33X_SDRAM_CKE	 0x00000080	/* pin control */
#define  RC33X_SDRAM_RAS_N	 0x00000040
#define  RC33X_SDRAM_CAS_N	 0x00000020
#define  RC33X_SDRAM_WE_N	 0x00000010
#define  RC33X_SDRAM_CMD	 0x00000070
#define   RC33X_SDRAM_CMD_MODE	  0x00000000
#define   RC33X_SDRAM_CMD_REFRESH 0x00000010
#define   RC33X_SDRAM_CMD_PRECHG  0x00000020
#define   RC33X_SDRAM_CMD_ACTIVE  0x00000030
#define   RC33X_SDRAM_CMD_WRITE	  0x00000040
#define   RC33X_SDRAM_CMD_READ	  0x00000050
#define   RC33X_SDRAM_CMD_NOP	  0x00000070
#define  RC33X_SDRAM_CS_N	 0x0000000f
#define  RC33X_SDRAM_CS3_N	 0x00000008
#define  RC33X_SDRAM_CS2_N	 0x00000004
#define  RC33X_SDRAM_CS1_N	 0x00000002
#define  RC33X_SDRAM_CS0_N	 0x00000001

/* EDO controller (rc32134 compatibility only) */
#define RC33X_EDO		RC3233X(0x0310)

#define RC33X_EDO_ENB	 	0x80000000	/* enable EDO */
#define RC33X_EDO_RASWIDTH	0x00000c00
#define  RC33X_EDO_RASWIDTH_12	 0x00000c00
#define  RC33X_EDO_RASWIDTH_11	 0x00000800
#define  RC33X_EDO_RASWIDTH_10	 0x00000400
#define  RC33X_EDO_RASWIDTH_9	 0x00000000
#define RC33X_EDO_CASWIDTH	0x00000300
#define  RC33X_EDO_CASWIDTH_11	 0x00000300
#define  RC33X_EDO_CASWIDTH_10	 0x00000200
#define RC33X_EDO_TRAS		0x000000c0
#define  RC33X_EDO_TRAS_5	 0x000000c0
#define  RC33X_EDO_TRAS_4	 0x00000080
#define  RC33X_EDO_TRAS_3	 0x00000040
#define RC33X_EDO_TRP		0x00000030
#define  RC33X_EDO_TRP_3	 0x00000030
#define  RC33X_EDO_TRP_2	 0x00000020
#define  RC33X_EDO_TRP_1	 0x00000010
#define RC33X_EDO_STAG_RFSH	0x00000008
#define RC33X_EDO_PAGEMODE	0x00000004
#define RC33X_EDO_EXT_WAIT_EN	0x00000002

/* Expansion Interrupt Control */
#define RC33X_INT_PEND(N)	RC3233X(0x0500 + ((N) * 0x10) + 0x00)
#define RC33X_INT_MASK(N)	RC3233X(0x0500 + ((N) * 0x10) + 0x04)
#define RC33X_INT_CLEAR(N)	RC3233X(0x0500 + ((N) * 0x10) + 0x08)

/* Interrupt Group numbers */
#define RC33X_INTGRP_TOP	0	/* main interrupt multiplexor */
#define RC33X_INTGRP_BUSERR	1	/* bus error i/us */
# define RC33X_INTGRP_BUSERR_ERR	0x00000001
#define RC33X_INTGRP_PIOLOW	2	/* pio is low i/us */
#define RC33X_INTGRP_PIOHIGH	3	/* pio is high i/us */
# define RC33X_INTGRP_PIO_PIO0		0x00000001
# define RC33X_INTGRP_PIO_PIO1		0x00000002
/* note skip */
# define RC33X_INTGRP_PIO_PIO2		0x00000008
# define RC33X_INTGRP_PIO_PIO3		0x00000010
# define RC33X_INTGRP_PIO_PIO4		0x00000020
# define RC33X_INTGRP_PIO_PIO5		0x00000040
# define RC33X_INTGRP_PIO_PIO6		0x00000080
# define RC33X_INTGRP_PIO_PIO7		0x00000100
# define RC33X_INTGRP_PIO_PIO8		0x00000200
# define RC33X_INTGRP_PIO_PIO9		0x00000400
# define RC33X_INTGRP_PIO_PIO10		0x00000800
#define RC33X_INTGRP_TIMER	4	/* timer rollover i/us */
#define RC33X_INTGRP_UART0	5	/* uart #0 i/us */
#define RC33X_INTGRP_UART1	6	/* uart #1 i/us */
# define RC33X_INTGRP_UART_RXRDY	0x00000004
# define RC33X_INTGRP_UART_TXRDY	0x00000002
# define RC33X_INTGRP_UART_IIR		0x00000001
#define RC33X_INTGRP_DMA0	7	/* DMA #0 i/us */
#define RC33X_INTGRP_DMA1	8	/* DMA #1 i/us */
#define RC33X_INTGRP_DMA2	9	/* DMA #2 i/us */
#define RC33X_INTGRP_DMA3	10	/* DMA #3 i/us */
# define RC33X_INTGRP_DMA_CLEAR	0x00000010
# define RC33X_INTGRP_DMA_COMPLETE	0x00000008
# define RC33X_INTGRP_DMA_NOT_OWNED	0x00000004
# define RC33X_INTGRP_DMA_EARLY_END	0x00000002
# define RC33X_INTGRP_DMA_DONE		0x00000001
#define RC33X_INTGRP_PCIERR	11	/* PCI controller errors */
# define RC33X_INTGRP_PCIERR_TARGET_PAR	0x00000008
# define RC33X_INTGRP_PCIERR_MASTER_PAR	0x00000004
# define RC33X_INTGRP_PCIERR_MASTER_RD	0x00000002
# define RC33X_INTGRP_PCIERR_MASTER_WR	0x00000001
#define RC33X_INTGRP_CPU2PCI	12	/* for external host */
#define RC33X_INTGRP_PCIMBOX	13	/* PCI mailbox i/us */
#define RC33X_INTGRP_SPI	14	/* SPI interrupts */
#define  RC33X_INTGRP_SPI_INT		0x00000001

/* Programmable/Parallel I/O */
#define RC33X_PIO0_DATA		RC3233X(0x0600)
#define RC33X_PIO0_DIR		RC3233X(0x0604)
#define RC33X_PIO0_FUNC		RC3233X(0x0608)
#define RC33X_PIO1_DATA		RC3233X(0x0610)
#define RC33X_PIO1_DIR		RC3233X(0x0614)
#define RC33X_PIO1_FUNC		RC3233X(0x0618)

/* PIO #0 internal function bits */
#define RC33X_PIO0_SPI_MOSI	0x00000800
#define RC33X_PIO0_SPI_SCK	0x00000400
#define RC33X_PIO0_SPI_SS_N	0x00000200
#define RC33X_PIO0_SPI_MISO	0x00000100
#define RC33X_PIO0_UART0_RX	0x00000080
#define RC33X_PIO0_UART0_TX	0x00000040
#define RC33X_PIO0_UART1_RX	0x00000020
#define RC33X_PIO0_UART1_TX	0x00000010
#define RC33X_PIO0_TMR0_TC	0x00000008
#define RC33X_PIO0_DMA0_RDY	0x00000002
#define RC33X_PIO0_DMA1_RDY	0x00000001

/* PIO #1 internal function bits */
#define RC33X_PIO1_UART0_CTS	0x00000010
#define RC33X_PIO1_UART0_DSR	0x00000008
#define RC33X_PIO1_UART0_DTR	0x00000004
#define RC33X_PIO1_UART0_RTS	0x00000002
#define RC33X_PIO1_PCI_EE_CS	0x00000001

/* Timers */
#define RC33X_TMR_CTRL(N)	RC3233X(0x0700 + ((N) * 0x10) + 0x00)
# define RC33X_TMR_CTRL_GATED	 0x00000002
# define RC33X_TMR_CTRL_ENB	 0x00000001
#define RC33X_TMR_COUNT(N)	RC3233X(0x0700 + ((N) * 0x10) + 0x04)
#define RC33X_TMR_COMPARE(N)	RC3233X(0x0700 + ((N) * 0x10) + 0x08)

/* Timer Numbers */
#define RC33X_GP0_TMR		0	/* general purpose #0 */
#define RC33X_GP1_TMR		1	/* general purpose #1 */
#define RC33X_GP2_TMR		2	/* general purpose #2 */
#define RC33X_WDOG_TMR		3	/* watchdog */
#define RC33X_CPUBUS_TMR	4	/* CPU bus timeout */
#define RC33X_IPBUS_TMR		5	/* IP bus timeout */
#define RC33X_RFRSH_TMR		6	/* dram refresh */
#define RC33X_RESET_TMR		7	/* warm reset timing */

/* Internal UARTS (see ns16550.h for detailed register map) */
#define RC33X_UART0		RC3233X(0x0800)
#define RC33X_UART1		RC3233X(0x0820)

/* Serial Perpheral Interface (SPI) */
#define RC33X_SPCNT		RC3233X(0x0900)	/* SPI clock divisor */
#define RC33X_SPCNTL		RC3233X(0x0904)	/* SPI control */
# define RC33X_SPCNTL_SPIE	 0x00000080	  /* interrupt enable */
# define RC33X_SPCNTL_SPE	 0x00000040	  /* SPI system enable */
# define RC33X_SPCNTL_MSTR	 0x00000010	  /* SPI is master */
# define RC33X_SPCNTL_CPOL	 0x00000008	  /* clock polarity */
# define RC33X_SPCNTL_CPHA	 0x00000004	  /* clock phase */
# define RC33X_SPCNTL_SPR	 0x00000003	  /* bit rate */
#  define RC33X_SPCNTL_SPR_BY2	  0x0
#  define RC33X_SPCNTL_SPR_BY4	  0x1
#  define RC33X_SPCNTL_SPR_BY16	  0x2
#  define RC33X_SPCNTL_SPR_BY32	  0x3
#define RC33X_SPSR		RC3233X(0x0908)	/* SPI status */
# define RC33X_SPSR_SPIF	 0x00000080	  /* transfer complete */
# define RC33X_SPSR_WCOL	 0x00000040	  /* write collision */
# define RC33X_SPSR_MODF	 0x00000010	  /* error */
#define RC33X_SPDR		RC3233X(0x090c)	/* SPI data */

/* DMA Controllers */
#define RC33X_DMA0_CONF		RC3233X(0x1400 + 0x00)
# define RC33X_DMA_CONF_ENB	 0x80000000	/* enable channel */
# define RC33X_DMA_CONF_CONT	 0x40000000	/* restart failed descriptor */
# define RC33X_DMA_CONF_RDY	 0x10000000	/* wait for dma_ready_n ip */
# define RC33X_DMA_CONF_DONE	 0x08000000	/* wait for dma_ready_n ip */
# define RC33X_DMA_CONF_MAXBRST	 0x07000000	/* log2(burst size) */
# define RC33X_DMA_CONF_MAXBRST_SHFT 24
#define RC33X_DMA0_BASE		RC3233X(0x1400 + 0x04)
#define RC33X_DMA0_ADDR		RC3233X(0x1400 + 0x08)
#define RC33X_DMA0_STAT		RC3233X(0x1400 + 0x10)
# define RC33X_DMA_STAT_DMA_OWN	 0x80000000	/* dma owns descriptor */
# define RC33X_DMA_STAT_ERR_OWN	 0x40000000	/* err in own bit */
# define RC33X_DMA_STAT_ERR_BUS	 0x20000000	/* bus error / timeout */
# define RC33X_DMA_STAT_LAST	 0x10000000	/* last descriptor */
# define RC33X_DMA_STAT_DONE	 0x08000000	/* dma done interrupt */
# define RC33X_DMA_STAT_SRC_DIR	 0x06000000	/* source direction */
# define RC33X_DMA_STAT_SRC_DIR_SHFT 25
#  define RC33X_DMA_STAT_DIR_INC	0	 /* increment */
#  define RC33X_DMA_STAT_DIR_DEC	1	 /* decrement */	
#  define RC33X_DMA_STAT_DIR_CONST	2	 /* static address */
# define RC33X_DMA_STAT_DST_DIR	 0x01800000	/* dest direction */
# define RC33X_DMA_STAT_DST_DIR_SHFT 23
# define RC33X_DMA_SRC_BIGEND	 0x00400000	/* source big endian */
# define RC33X_DMA_DST_BIGEND	 0x00200000	/* dest big endian */
# define RC33X_DMA_BLKSIZE	 0x0000ffff	/* data size */
#define RC33X_DMA0_DSTADDR	RC3233X(0x1400 + 0x18)
#define RC33X_DMA0_NEXT		RC3233X(0x1400 + 0x1c)

/* DMA descriptor format */
#ifdef __ASSEMBLER__
	.struct 0
dma_stat:	.word 0
dma_srcaddr:	.word 0
dma_dstaddr:	.word 0
dma_next:	.word 0
	.previous
#else
struct rc33X_dma_desc {
    unsigned int	dma_stat;
    unsigned int	dma_srcaddr;
    unsigned int	dma_dstaddr;
    unsigned int	dma_next;
};
#endif

#define RC33X_DMA1_CONF		RC3233X(0x1440 + 0x00)
#define RC33X_DMA1_BASE		RC3233X(0x1440 + 0x04)
#define RC33X_DMA1_ADDR		RC3233X(0x1440 + 0x08)
#define RC33X_DMA1_STAT		RC3233X(0x1440 + 0x10)
#define RC33X_DMA1_SRCADDR	RC3233X(0x1440 + 0x14)
#define RC33X_DMA1_DSTADDR	RC3233X(0x1440 + 0x18)
#define RC33X_DMA1_NEXT		RC3233X(0x1440 + 0x1c)

#define RC33X_DMA2_CONF		RC3233X(0x1900 + 0x00)
#define RC33X_DMA2_BASE		RC3233X(0x1900 + 0x04)
#define RC33X_DMA2_ADDR		RC3233X(0x1900 + 0x08)
#define RC33X_DMA2_STAT		RC3233X(0x1900 + 0x10)
#define RC33X_DMA2_SRCADDR	RC3233X(0x1900 + 0x14)
#define RC33X_DMA2_DSTADDR	RC3233X(0x1900 + 0x18)
#define RC33X_DMA2_NEXT		RC3233X(0x1900 + 0x1c)
					    
#define RC33X_DMA3_CONF		RC3233X(0x1940 + 0x00)
#define RC33X_DMA3_BASE		RC3233X(0x1940 + 0x04)
#define RC33X_DMA3_ADDR		RC3233X(0x1940 + 0x08)
#define RC33X_DMA3_STAT		RC3233X(0x1940 + 0x10)
#define RC33X_DMA3_SRCADDR	RC3233X(0x1940 + 0x14)
#define RC33X_DMA3_DSTADDR	RC3233X(0x1940 + 0x18)
#define RC33X_DMA3_NEXT		RC3233X(0x1940 + 0x1c)

/* PCI Interface Control */
#define RC33X_PCI_MEM1_BASE	RC3233X(0x20b0)
#define RC33X_PCI_MEM2_BASE	RC3233X(0x20b8)
#define RC33X_PCI_MEM3_BASE	RC3233X(0x20c0)
# define RC33X_PCI_MEM_MASK	 0xf0000000
# define RC33X_PCI_MEM_BSWAP	 0x00000001

#define RC33X_PCI_IO_BASE	RC3233X(0x20c8)
# define RC33X_PCI_IO_MASK	 0xfff00000
# define RC33X_PCI_IO_BSWAP	 0x00000001

#define RC33X_PCI_ARB		RC3233X(0x20e0)
# define RC33X_PCI_ARB_NOT_RDY	 0x00000004
# define RC33X_PCI_ARB_TYPE	 0x00000002
#  define RC33X_PCI_ARB_TYPE_EXT  0x00000002
#  define RC33X_PCI_ARB_TYPE_INT  0x00000000
# define RC33X_PCI_ARB_MODE	 0x00000001
#  define RC33X_PCI_ARB_MODE_FIX  0x00000001
#  define RC33X_PCI_ARB_MODE_RR   0x00000000

#define RC33X_PCI_CPU_MEM1_BASE	RC3233X(0x20e8)
#define RC33X_PCI_CPU_IO_BASE	RC3233X(0x2100)
# define RC33X_PCI_CPU_MASK 	 0xf0000000
# define RC33X_PCI_CPU_BSWAP	 0x00000001

#define RC33X_PCI_CONF_ADDR	RC3233X(0x2cf8)
#define  RC33X_PCI_CONF_ENABLE	 0x80000000
#define  RC33X_PCI_CONF_BUS	 0x00ff0000
#define  RC33X_PCI_CONF_BUS_SHFT 16
#define  RC33X_PCI_CONF_DEV	 0x0000f800
#define  RC33X_PCI_CONF_DEV_SHFT 11
#define  RC33X_PCI_CONF_FUNC	 0x00000700
#define  RC33X_PCI_CONF_FUNC_SHFT 8
#define  RC33X_PCI_CONF_REG	 0x000000fc
#define  RC33X_PCI_CONF_REG_SHFT 0

#define RC33X_PCI_CONF_DATA	RC3233X(0x2cfc)

#endif /* __RC3233X_H__ */
