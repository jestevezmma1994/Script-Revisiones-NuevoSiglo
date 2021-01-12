/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/***************************************************************************
 File : splash_vdc_reg.h
 Date  : Nov 12 2015 17:43:26
 Description : Register list for splash BVN configuration
 ***************************************************************************/

#include "splash_magnum.h"

uint32_t g_aulReg[] =
{

	0x00404318, 0x0005a700, /* SUN_TOP_CTRL_SW_RESET */

	0x0040431c, 0x0005a700, /* SUN_TOP_CTRL_SW_INIT_0_CLEAR */

	0x0042004c, 0x00000014, /* CLKGEN */
	0x00420050, 0x00000010,
	0x00420168, 0x0000001f,
	0x00420174, 0x00000002,
	0x00420174, 0x00000000,
	0x00420350, 0x0000007f,
	0x0042029c, 0x00000007,
	0x00420054, 0x00000010,
	0x0042029c, 0x00000007,
	0x004202a8, 0x00000002,
	0x004202a8, 0x00000000,
	0x004201f0, 0x0000000f,
	0x00420190, 0x00000007,
	0x004201f4, 0x00000001,

	0x006a0a80, 0x0000003f, /* HDMI TX Control */

	0x00420190, 0x00000007, /* CLKGEN */
	0x00420198, 0x00000002,
	0x00420198, 0x00000000,
	0x0042034c, 0x00000000,
	0x00420360, 0x00000002,
	0x00420360, 0x00000000,
	0x004201dc, 0x00000007,
	0x004201dc, 0x00000007,
	0x004201d8, 0x00000000,
	0x004201e8, 0x00000002,
	0x004201e8, 0x00000000,
	0x00420200, 0x00000002,
	0x00420200, 0x00000000,
	0x0042022c, 0x0000000f,
	0x0042022c, 0x0000000f,
	0x00420228, 0x00000000,
	0x00420238, 0x00000002,
	0x00420238, 0x00000000,
	0x00420158, 0x00000000,
	0x00420154, 0x00000000,
	0x00420134, 0x00000096,
	0x00420130, 0x00000096,
	0x004200a4, 0x00000000,
	0x004200a0, 0x00000000,
	0x00420080, 0x00000050,
	0x00420320, 0x00000000,
	0x00420084, 0x00000050,
	0x00420320, 0x00000000,
	0x00420014, 0x00000000,
	0x00420010, 0x00000000,
	0x00420038, 0x00000000,
	0x00420034, 0x00000000,

	0x00060080, 0x00000000, /* ?? */

	0x00400008, 0x00000000, /* SUN_GISB_ARB_... */

	0x004040a8, 0x000001c1, /* SUN_TOP_CTRL ... */

	0x0042024c, 0x00000001, /* CLKGEN */

	0x00626010, 0x00000001, /* BVNM_INTR2_0 */
	0x00626010, 0x00000002,
	0x00626010, 0x00000004,
	0x00626010, 0x00000008,
	0x00626010, 0x00000010,
	0x00626010, 0x00000020,
	0x00626010, 0x00000040,
	0x00626010, 0x00000080,
	0x00626010, 0x00000100,
	0x00626010, 0x00000200,
	0x00626010, 0x00000400,
	0x00626010, 0x00000800,
	0x00626010, 0x00001000,
	0x00626010, 0x00002000,
	0x00626010, 0x00004000,
	0x00626010, 0x00008000,
	0x00626010, 0x00010000,
	0x00626010, 0x00020000,
	0x00626010, 0x00040000,
	0x00626010, 0x00080000,
	0x00626010, 0x00100000,
	0x00626010, 0x00200000,
	0x00626010, 0x00400000,
	0x00626010, 0x00800000,
	0x00626010, 0x01000000,
	0x00626010, 0x02000000,
	0x00626010, 0x04000000,
	0x00626010, 0x08000000,
	0x00626010, 0x10000000,
	0x00626010, 0x20000000,
	0x00626010, 0x40000000,
	0x00626010, 0x80000000,

	0x00424010, 0x00000001, /* CLKGEN_INTR2 */
	0x00424010, 0x00000002,
	0x00424010, 0x00000004,
	0x00424010, 0x00000008,
	0x00424010, 0x00000010,
	0x00424010, 0x00000020,
	0x00424010, 0x00000040,
	0x00424010, 0x00000080,
	0x00424010, 0x00000100,
	0x00424010, 0x00000200,
	0x00424010, 0x00000400,
	0x00424010, 0x00000800,
	0x00424010, 0x00001000,
	0x00424010, 0x00002000,
	0x00424010, 0x00004000,
	0x00424010, 0x00008000,
	0x00424010, 0x00010000,
	0x00424010, 0x00020000,
	0x00424010, 0x00040000,
	0x00424010, 0x00080000,
	0x00424010, 0x00100000,
	0x00424010, 0x00200000,
	0x00424010, 0x00400000,
	0x00424010, 0x00800000,
	0x00424010, 0x01000000,
	0x00424010, 0x02000000,
	0x00424010, 0x04000000,
	0x00424010, 0x08000000,
	0x00424010, 0x10000000,
	0x00424010, 0x20000000,
	0x00424010, 0x40000000,
	0x00424010, 0x80000000,

	0x00706310, 0x00000001, /* ?? */
	0x00706310, 0x00000002,
	0x00706310, 0x00000004,
	0x00706310, 0x00000008,
	0x00706310, 0x00000010,
	0x00706310, 0x00000020,
	0x00706310, 0x00000040,
	0x00706310, 0x00000080,
	0x00706310, 0x00000100,
	0x00706310, 0x00000200,
	0x00706310, 0x00000400,
	0x00706310, 0x00000800,
	0x00706310, 0x00001000,
	0x00706310, 0x00002000,
	0x00706310, 0x00004000,
	0x00706310, 0x00008000,
	0x00706310, 0x00010000,
	0x00706310, 0x00020000,
	0x00706310, 0x00040000,
	0x00706310, 0x00080000,
	0x00706310, 0x00100000,
	0x00706310, 0x00200000,
	0x00706310, 0x00400000,
	0x00706310, 0x00800000,
	0x00706310, 0x01000000,
	0x00706310, 0x02000000,
	0x00706310, 0x04000000,
	0x00706310, 0x08000000,
	0x00706310, 0x10000000,
	0x00706310, 0x20000000,
	0x00706310, 0x40000000,
	0x00706310, 0x80000000,
	0x00451010, 0x00000001,
	0x00451010, 0x00000002,
	0x00451010, 0x00000004,
	0x00451010, 0x00000008,
	0x00451010, 0x00000010,
	0x00451010, 0x00000020,
	0x00451010, 0x00000040,
	0x00451010, 0x00000080,
	0x00451010, 0x00000100,
	0x00451010, 0x00000200,
	0x00451010, 0x00000400,
	0x00451010, 0x00000800,
	0x00451010, 0x00001000,
	0x00451010, 0x00002000,
	0x00451010, 0x00004000,
	0x00451010, 0x00008000,
	0x00451010, 0x00010000,
	0x00451010, 0x00020000,
	0x00451010, 0x00040000,
	0x00451010, 0x00080000,
	0x00451010, 0x00100000,
	0x00451010, 0x00200000,
	0x00451010, 0x00400000,
	0x00451010, 0x00800000,
	0x00451010, 0x01000000,
	0x00451010, 0x02000000,
	0x00451010, 0x04000000,
	0x00451010, 0x08000000,
	0x00451010, 0x10000000,
	0x00451010, 0x20000000,
	0x00451010, 0x40000000,
	0x00451010, 0x80000000,

	0x003b3010, 0x00000001, /* MEMC0... */
	0x003b3010, 0x00000002,
	0x003b3010, 0x00000004,
	0x003b3010, 0x00000008,
	0x003b3010, 0x00000010,
	0x003b3010, 0x00000020,
	0x003b3010, 0x00000040,
	0x003b3010, 0x00000080,
	0x003b3010, 0x00000100,
	0x003b3010, 0x00000200,
	0x003b3010, 0x00000400,
	0x003b3010, 0x00000800,
	0x003b3010, 0x00001000,
	0x003b3010, 0x00002000,
	0x003b3010, 0x00004000,
	0x003b3010, 0x00008000,
	0x003b3010, 0x00010000,
	0x003b3010, 0x00020000,
	0x003b3010, 0x00040000,
	0x003b3010, 0x00080000,
	0x003b3010, 0x00100000,
	0x003b3010, 0x00200000,
	0x003b3010, 0x00400000,
	0x003b3010, 0x00800000,
	0x003b3010, 0x01000000,
	0x003b3010, 0x02000000,
	0x003b3010, 0x04000000,
	0x003b3010, 0x08000000,
	0x003b3010, 0x10000000,
	0x003b3010, 0x20000000,
	0x003b3010, 0x40000000,
	0x003b3010, 0x80000000,

	0x00822410, 0x00000001, /* RAAGA... */
	0x00822410, 0x00000002,
	0x00822410, 0x00000004,
	0x00822410, 0x00000008,
	0x00822410, 0x00000010,
	0x00822410, 0x00000020,
	0x00822410, 0x00000040,
	0x00822410, 0x00000080,
	0x00822410, 0x00000100,
	0x00822410, 0x00000200,
	0x00822410, 0x00000400,
	0x00822410, 0x00000800,
	0x00822410, 0x00001000,
	0x00822410, 0x00002000,
	0x00822410, 0x00004000,
	0x00822410, 0x00008000,
	0x00822410, 0x00010000,
	0x00822410, 0x00020000,
	0x00822410, 0x00040000,
	0x00822410, 0x00080000,
	0x00822410, 0x00100000,
	0x00822410, 0x00200000,
	0x00822410, 0x00400000,
	0x00822410, 0x00800000,
	0x00822410, 0x01000000,
	0x00822410, 0x02000000,
	0x00822410, 0x04000000,
	0x00822410, 0x08000000,
	0x00822410, 0x10000000,
	0x00822410, 0x20000000,
	0x00822410, 0x40000000,
	0x00822410, 0x80000000,

	0x00704810, 0x00000001, /* ?? */
	0x00704810, 0x00000002,
	0x00704810, 0x00000004,
	0x00704810, 0x00000008,
	0x00704810, 0x00000010,
	0x00704810, 0x00000020,
	0x00704810, 0x00000040,
	0x00704810, 0x00000080,
	0x00704810, 0x00000100,
	0x00704810, 0x00000200,
	0x00704810, 0x00000400,
	0x00704810, 0x00000800,
	0x00704810, 0x00001000,
	0x00704810, 0x00002000,
	0x00704810, 0x00004000,
	0x00704810, 0x00008000,
	0x00704810, 0x00010000,
	0x00704810, 0x00020000,
	0x00704810, 0x00040000,
	0x00704810, 0x00080000,
	0x00704810, 0x00100000,
	0x00704810, 0x00200000,
	0x00704810, 0x00400000,
	0x00704810, 0x00800000,
	0x00704810, 0x01000000,
	0x00704810, 0x02000000,
	0x00704810, 0x04000000,
	0x00704810, 0x08000000,
	0x00704810, 0x10000000,
	0x00704810, 0x20000000,
	0x00704810, 0x40000000,
	0x00704810, 0x80000000,
	0x00702210, 0x00000001,
	0x00702210, 0x00000002,
	0x00702210, 0x00000004,
	0x00702210, 0x00000008,
	0x00702210, 0x00000010,
	0x00702210, 0x00000020,
	0x00702210, 0x00000040,
	0x00702210, 0x00000080,
	0x00702210, 0x00000100,
	0x00702210, 0x00000200,
	0x00702210, 0x00000400,
	0x00702210, 0x00000800,
	0x00702210, 0x00001000,
	0x00702210, 0x00002000,
	0x00702210, 0x00004000,
	0x00702210, 0x00008000,
	0x00702210, 0x00010000,
	0x00702210, 0x00020000,
	0x00702210, 0x00040000,
	0x00702210, 0x00080000,
	0x00702210, 0x00100000,
	0x00702210, 0x00200000,
	0x00702210, 0x00400000,
	0x00702210, 0x00800000,
	0x00702210, 0x01000000,
	0x00702210, 0x02000000,
	0x00702210, 0x04000000,
	0x00702210, 0x08000000,
	0x00702210, 0x10000000,
	0x00702210, 0x20000000,
	0x00702210, 0x40000000,
	0x00702210, 0x80000000,
	0x00700a10, 0x00000001,
	0x00700a10, 0x00000002,
	0x00700a10, 0x00000004,
	0x00700a10, 0x00000008,
	0x00700a10, 0x00000010,
	0x00700a10, 0x00000020,
	0x00700a10, 0x00000040,
	0x00700a10, 0x00000080,
	0x00700a10, 0x00000100,
	0x00700a10, 0x00000200,
	0x00700a10, 0x00000400,
	0x00700a10, 0x00000800,
	0x00700a10, 0x00001000,
	0x00700a10, 0x00002000,
	0x00700a10, 0x00004000,
	0x00700a10, 0x00008000,
	0x00700a10, 0x00010000,
	0x00700a10, 0x00020000,
	0x00700a10, 0x00040000,
	0x00700a10, 0x00080000,
	0x00700a10, 0x00100000,
	0x00700a10, 0x00200000,
	0x00700a10, 0x00400000,
	0x00700a10, 0x00800000,
	0x00700a10, 0x01000000,
	0x00700a10, 0x02000000,
	0x00700a10, 0x04000000,
	0x00700a10, 0x08000000,
	0x00700a10, 0x10000000,
	0x00700a10, 0x20000000,
	0x00700a10, 0x40000000,
	0x00700a10, 0x80000000,
	0x00700b10, 0x00000001,
	0x00700b10, 0x00000002,
	0x00700b10, 0x00000004,
	0x00700b10, 0x00000008,
	0x00700b10, 0x00000010,
	0x00700b10, 0x00000020,
	0x00700b10, 0x00000040,
	0x00700b10, 0x00000080,
	0x00700b10, 0x00000100,
	0x00700b10, 0x00000200,
	0x00700b10, 0x00000400,
	0x00700b10, 0x00000800,
	0x00700b10, 0x00001000,
	0x00700b10, 0x00002000,
	0x00700b10, 0x00004000,
	0x00700b10, 0x00008000,
	0x00700b10, 0x00010000,
	0x00700b10, 0x00020000,
	0x00700b10, 0x00040000,
	0x00700b10, 0x00080000,
	0x00700b10, 0x00100000,
	0x00700b10, 0x00200000,
	0x00700b10, 0x00400000,
	0x00700b10, 0x00800000,
	0x00700b10, 0x01000000,
	0x00700b10, 0x02000000,
	0x00700b10, 0x04000000,
	0x00700b10, 0x08000000,
	0x00700b10, 0x10000000,
	0x00700b10, 0x20000000,
	0x00700b10, 0x40000000,
	0x00700b10, 0x80000000,

	0x00403410, 0x00000001, /* SYS_CTRL... */
	0x00403410, 0x00000002,
	0x00403410, 0x00000004,
	0x00403410, 0x00000008,
	0x00403410, 0x00000010,
	0x00403410, 0x00000020,
	0x00403410, 0x00000040,
	0x00403410, 0x00000080,
	0x00403410, 0x00000100,
	0x00403410, 0x00000200,
	0x00403410, 0x00000400,
	0x00403410, 0x00000800,
	0x00403410, 0x00001000,
	0x00403410, 0x00002000,
	0x00403410, 0x00004000,
	0x00403410, 0x00008000,
	0x00403410, 0x00010000,
	0x00403410, 0x00020000,
	0x00403410, 0x00040000,
	0x00403410, 0x00080000,
	0x00403410, 0x00100000,
	0x00403410, 0x00200000,
	0x00403410, 0x00400000,
	0x00403410, 0x00800000,
	0x00403410, 0x01000000,
	0x00403410, 0x02000000,
	0x00403410, 0x04000000,
	0x00403410, 0x08000000,
	0x00403410, 0x10000000,
	0x00403410, 0x20000000,
	0x00403410, 0x40000000,
	0x00403410, 0x80000000,
	0x00403010, 0x00000001,
	0x00403010, 0x00000002,
	0x00403010, 0x00000004,
	0x00403010, 0x00000008,
	0x00403010, 0x00000010,
	0x00403010, 0x00000020,
	0x00403010, 0x00000040,
	0x00403010, 0x00000080,
	0x00403010, 0x00000100,
	0x00403010, 0x00000200,
	0x00403010, 0x00000400,
	0x00403010, 0x00000800,
	0x00403010, 0x00001000,
	0x00403010, 0x00002000,
	0x00403010, 0x00004000,
	0x00403010, 0x00008000,
	0x00403010, 0x00010000,
	0x00403010, 0x00020000,
	0x00403010, 0x00040000,
	0x00403010, 0x00080000,
	0x00403010, 0x00100000,
	0x00403010, 0x00200000,
	0x00403010, 0x00400000,
	0x00403010, 0x00800000,
	0x00403010, 0x01000000,
	0x00403010, 0x02000000,
	0x00403010, 0x04000000,
	0x00403010, 0x08000000,
	0x00403010, 0x10000000,
	0x00403010, 0x20000000,
	0x00403010, 0x40000000,
	0x00403010, 0x80000000,

	0x00408410, 0x00000001, /* ?? */
	0x00408410, 0x00000002,
	0x00408410, 0x00000004,
	0x00408410, 0x00000008,
	0x00408410, 0x00000010,
	0x00408410, 0x00000020,
	0x00408410, 0x00000040,
	0x00408410, 0x00000080,
	0x00408410, 0x00000100,
	0x00408410, 0x00000200,
	0x00408410, 0x00000400,
	0x00408410, 0x00000800,
	0x00408410, 0x00001000,
	0x00408410, 0x00002000,
	0x00408410, 0x00004000,
	0x00408410, 0x00008000,
	0x00408410, 0x00010000,
	0x00408410, 0x00020000,
	0x00408410, 0x00040000,
	0x00408410, 0x00080000,
	0x00408410, 0x00100000,
	0x00408410, 0x00200000,
	0x00408410, 0x00400000,
	0x00408410, 0x00800000,
	0x00408410, 0x01000000,
	0x00408410, 0x02000000,
	0x00408410, 0x04000000,
	0x00408410, 0x08000000,
	0x00408410, 0x10000000,
	0x00408410, 0x20000000,
	0x00408410, 0x40000000,
	0x00408410, 0x80000000,

	0x00408e50, 0x00000001, /* AON UPG... */
	0x00408e50, 0x00000002,
	0x00408e50, 0x00000004,
	0x00408e50, 0x00000008,
	0x00408e50, 0x00000010,
	0x00408e50, 0x00000020,
	0x00408e50, 0x00000040,
	0x00408e50, 0x00000080,
	0x00408e50, 0x00000100,
	0x00408e50, 0x00000200,
	0x00408e50, 0x00000400,
	0x00408e50, 0x00000800,
	0x00408e50, 0x00001000,
	0x00408e50, 0x00002000,
	0x00408e50, 0x00004000,
	0x00408e50, 0x00008000,
	0x00408e50, 0x00010000,
	0x00408e50, 0x00020000,
	0x00408e50, 0x00040000,
	0x00408e50, 0x00080000,
	0x00408e50, 0x00100000,
	0x00408e50, 0x00200000,
	0x00408e50, 0x00400000,
	0x00408e50, 0x00800000,
	0x00408e50, 0x01000000,
	0x00408e50, 0x02000000,
	0x00408e50, 0x04000000,
	0x00408e50, 0x08000000,
	0x00408e50, 0x10000000,
	0x00408e50, 0x20000000,
	0x00408e50, 0x40000000,
	0x00408e50, 0x80000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,
	0x00408b80, 0x00000000,

	0x00080010, 0x00000001, /* ?? */
	0x00080010, 0x00000002,
	0x00080010, 0x00000004,
	0x00080010, 0x00000008,
	0x00080010, 0x00000010,
	0x00080010, 0x00000020,
	0x00080010, 0x00000040,
	0x00080010, 0x00000080,
	0x00080010, 0x00000100,
	0x00080010, 0x00000200,
	0x00080010, 0x00000400,
	0x00080010, 0x00000800,
	0x00080010, 0x00001000,
	0x00080010, 0x00002000,
	0x00080010, 0x00004000,
	0x00080010, 0x00008000,
	0x00080010, 0x00010000,
	0x00080010, 0x00020000,
	0x00080010, 0x00040000,
	0x00080010, 0x00080000,
	0x00080010, 0x00100000,
	0x00080010, 0x00200000,
	0x00080010, 0x00400000,
	0x00080010, 0x00800000,
	0x00080010, 0x01000000,
	0x00080010, 0x02000000,
	0x00080010, 0x04000000,
	0x00080010, 0x08000000,
	0x00080010, 0x10000000,
	0x00080010, 0x20000000,
	0x00080010, 0x40000000,
	0x00080010, 0x80000000,

	0x003b3008, 0x00000001, /* MEMC0... */
	0x003b3008, 0x00000001,
	0x003b3008, 0x00000001,
	0x003b3014, 0x00000001,
	0x003b3008, 0x00000002,
	0x003b3008, 0x00000002,
	0x003b3008, 0x00000002,
	0x003b3014, 0x00000002,
	0x003b3008, 0x00000004,
	0x003b3008, 0x00000004,
	0x003b3008, 0x00000004,
	0x003b3014, 0x00000004,
	0x003b0290, 0x00000001,
	0x003b0290, 0x00000000,
	0x003b3008, 0x00000008,
	0x003b3008, 0x00000008,
	0x003b3008, 0x00000008,
	0x003b3014, 0x00000008,

	0x0040000c, 0x00a4cb80, /* SUN_GISB_ARB_... */

	0x00403008, 0x00000001, /* SYS_CTRL... */

	0x004000c8, 0x00000001, /* SUN_GISB_ARB_... */

	0x00403014, 0x00000001, /* SYS_CTRL... */

	0x00404100, 0x11100011, /* SUN_TOP_CTRL ... */
	0x00404118, 0x11111111,
	0x0040411c, 0x11111111,
	0x00404120, 0x11111111,
	0x00404124, 0x10011111,
	0x00404128, 0x01111111,
	0x0040412c, 0x33332210,
	0x00404130, 0x10000011,
	0x00404134, 0x22201111,
	0x0040413c, 0x11110000,

	0x00408500, 0x00011000, /* AON_PIN_MUX */
	0x00408508, 0x10011000,
	0x0040850c, 0x00000111,

	0x0040439c, 0x00000230, /* SUN_TOP_CTRL ... */
	0x00404380, 0x00000010,

	0x00406228, 0x00000090, /* ?? */
	0x00406250, 0x00000040,
	0x00406228, 0x000000d0,
	0x00406250, 0x00000040,

	0x00604000, 0x01000000, /* FMISC */
	0x00604000, 0x00000000,

	0x00400008, 0x00000000, /* SUN_GISB_ARB_... */

	0x00604000, 0xfeffffff, /* FMISC */
	0x00604000, 0x00000000,
	0x00604018, 0x00000000,

	0x00622800, 0xffffffff, /* MMISC */
	0x00622800, 0x00000000,
	0x00622820, 0x00000000,

	0x00646400, 0xffffffff, /* BMISC */
	0x00646400, 0x00000000,
	0x00646418, 0x00000000,

	0x00404318, 0x00000200, /* SUN_TOP_CTRL_SW_RESET */

	0x0040431c, 0x00000200, /* SUN_TOP_CTRL_SW_INIT_0_CLEAR */

	0x00685070, 0x00000001, /* VEC... */
	0x00685070, 0x00000000,
	0x00680004, 0x00000007,
	0x00680008, 0x00000007,
	0x0068000c, 0x00000007,
	0x00680010, 0x00000007,
	0x00680048, 0x0bff42d8,
	0x00680058, 0x228708b3,

	0x006a0834, 0x40000000, /* HDMI TX Control */
	0x006a0834, 0x00000000,

	0x006a00bc, 0x00000001, /* ?? */

	0x006a0868, 0x00002002, /* HDMI TX Control */
	0x006a0a84, 0x0000000f,
	0x006a0a80, 0x0000003f,
	0x006a0890, 0x76235410,
	0x006a0a84, 0x0000000f,
	0x006a0a84, 0x0000000f,

	0x00682058, 0x00003003, /* VEC... */
	0x0068205c, 0x0000310a,
	0x00681058, 0x00003003,
	0x0068105c, 0x00002000,
	0x00681058, 0x00003003,
	0x0068105c, 0x00002000,

	0x006a00bc, 0x00000001, /* ?? */

	0x006a0868, 0x00002002, /* HDMI TX Control */
	0x006a0a84, 0x0000000f,
	0x006a0a80, 0x0000003f,
	0x006a0a84, 0x00000007,
	0x006a0a80, 0x00000030,
	0x006a0a84, 0x00000000,
	0x006a0a88, 0x00a0280a,
	0x006a0a8c, 0x0009180a,
	0x006a0a90, 0x2a300000,
	0x006a095c, 0x00000003,
	0x006a08d8, 0x006ec500,
	0x006a08dc, 0x00dc0028,
	0x006a08e0, 0x050502d0,
	0x006a08e4, 0x00000014,
	0x006a08e8, 0x050502d0,
	0x006a08ec, 0x00000014,
	0x006a08d0, 0x00000083,
	0x006a0874, 0x00000000,
	0x006a0868, 0x00002002,
	0x006a0868, 0x00002042,
	0x006a0868, 0x00006002,
	0x006a0868, 0x00006042,
	0x006a0868, 0x00006042,
	0x006a091c, 0xffffffff,
	0x006a091c, 0x00000000,
	0x006a091c, 0xffffffff,
	0x006a091c, 0x00000000,
};