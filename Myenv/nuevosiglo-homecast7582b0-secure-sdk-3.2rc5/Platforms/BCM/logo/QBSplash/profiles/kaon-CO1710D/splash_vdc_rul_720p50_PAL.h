/***************************************************************************
 File : splash_vdc_rul.h
 Date  : Jan 23 2015 16:59:26
 Description : RUL list for splash BVN configuration
 ***************************************************************************/

#include "splash_magnum.h"


#define BSPLASH_NUM_MEM			 1

int g_iRulMemIdx = 0;	/* index to ahMem for vdc/rul usage */

#define BSPLASH_NUM_SURFACE		 2
SplashSurfaceInfo  g_SplashSurfaceInfo[BSPLASH_NUM_SURFACE] =
{
	{   /* sur 0: */
		0,							/* ihMemIdx */
		(BPXL_Format)0x06e40565,	/* BPXL_eR5_G6_B5 */
		1280,						/* width */
		720,						/* height */
		2560,						/* pitch */
		"splash.bmp"				/* bmp file in splashgen */
	},
	{   /* sur 1: */
		0,							/* ihMemIdx */
		(BPXL_Format)0x06e40565,	/* BPXL_eR5_G6_B5 */
		720,						/* width */
		576,						/* height */
		1440,						/* pitch */
		"splash.bmp"				/* bmp file in splashgen */
	}
};

#define BSPLASH_NUM_DISPLAY		 2
SplashDisplayInfo  g_SplashDisplayInfo[BSPLASH_NUM_DISPLAY] =
{
	{   /* disp 0: */
		0,					/* iSurIdx */
		(BFMT_VideoFmt)42,	/* BFMT_VideoFmt_e720p_50Hz */
		0x0060289c,			/* ulRdcScratchReg0 */
		0x006028a0 			/* ulRdcScratchReg1 */
	},
	{   /* disp 1: */
		1,					/* iSurIdx */
		(BFMT_VideoFmt)7,	/* BFMT_VideoFmt_ePAL_G */
		0x006028b4,			/* ulRdcScratchReg0 */
		0x00000000 			/* ulRdcScratchReg1 */
	}
};



uint32_t g_aListCount8[] = 
{
	0,
	286,
	289,
	516,
	607,
	677,
	759,
	1012,
	1015,
	1102,
	1184,
	1254,
};

uint32_t g_aList8[] = 
{

	/* LIST: 0 (286 entries) */
	0x01000000,
	0x10683c30,
	0x00000004,
	0x01000000,
	0x10683c88,
	0x00000001,
	0x01000000,
	0x10683c8c,
	0x00000001,
	0x01000000,
	0x10683c90,
	0x00000001,
	0x01000000,
	0x10683c94,
	0x00000001,
	0x01000000,
	0x10683c88,
	0x00000000,
	0x01000000,
	0x10683c8c,
	0x00000000,
	0x01000000,
	0x10683c90,
	0x00000000,
	0x01000000,
	0x10683c94,
	0x00000000,
	0x01000000,
	0x106a0aa4,
	0x04530000,
	0x01000000,
	0x106a0ab0,
	0x00000001,
	0x04000000,
	0xfcffffff,
	0x04000001,
	0x02000000,
	0x03000002,
	0x106a0a98,
	0x0b002002,
	0x0d002042,
	0x02002000,
	0x106a0a98,
	0x04000000,
	0xc00fffff,
	0x04000001,
	0x2a300000,
	0x03000002,
	0x106a0a90,
	0x0b002002,
	0x0d002042,
	0x02002000,
	0x106a0a90,
	0x04000000,
	0xfffffff3,
	0x04000001,
	0x0000000c,
	0x03000002,
	0x106a0a9c,
	0x0b002002,
	0x0d002042,
	0x02002000,
	0x106a0a9c,
	0x01000000,
	0x10642060,
	0xbaadf00d,
	0x01000000,
	0x10642060,
	0xbaadf00d,
	0x01000000,
	0x10642060,
	0xbaadf00d,
	0x04000000,
	0xfffffff3,
	0x04000001,
	0x00000000,
	0x03000002,
	0x106a0a9c,
	0x0b002002,
	0x0d002042,
	0x02002000,
	0x106a0a9c,
	0x01000000,
	0x106a0b04,
	0x00007fd5,
	0x01000000,
	0x106a0b08,
	0x41140001,
	0x01000000,
	0x106a0b14,
	0x06e00000,
	0x01000000,
	0x106a0b18,
	0x01000000,
	0x01000000,
	0x106a0b18,
	0x01002710,
	0x0600003f,
	0x10682980,
	0x00648001,
	0x0064d004,
	0x00657001,
	0x00652013,
	0x006612d0,
	0x00652004,
	0x0065c001,
	0x00840000,
	0x0020401d,
	0x002d0028,
	0x002c00dc,
	0x00241500,
	0x00340185,
	0x00244028,
	0x00250028,
	0x002400dc,
	0x00241500,
	0x00340185,
	0x00204028,
	0x00210028,
	0x002000dc,
	0x00201500,
	0x00300185,
	0x00244028,
	0x00210028,
	0x002000dc,
	0x00201500,
	0x00300185,
	0x00204028,
	0x00210028,
	0x002000dc,
	0x00201500,
	0x00300185,
	0x00206028,
	0x00212028,
	0x002020dc,
	0x00223500,
	0x00302185,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00061209,
	0x7b899c3b,
	0x01000000,
	0x10682808,
	0x0000000b,
	0x01000000,
	0x10683010,
	0x00000249,
	0x01000000,
	0x10683014,
	0x000001d9,
	0x01000000,
	0x10683018,
	0x00000003,
	0x01000000,
	0x1068301c,
	0x00000040,
	0x01000000,
	0x10683020,
	0x02d00500,
	0x01000000,
	0x1068302c,
	0x0000c003,
	0x01000000,
	0x10683030,
	0x0000c000,
	0x01000000,
	0x106830f0,
	0x000005a0,
	0x01000000,
	0x106830fc,
	0x000002d1,
	0x01000000,
	0x10683a0c,
	0x00000000,
	0x06000007,
	0x10683908,
	0x00003fda,
	0x01fff000,
	0x20000000,
	0x00000000,
	0x00002000,
	0x00000000,
	0x00000000,
	0x00002000,
	0x01000000,
	0x10683a10,
	0x00000000,
	0x06000002,
	0x10683928,
	0x00100401,
	0x00000610,
	0x000ef83d,
	0x01000000,
	0x10680018,
	0x00000003,
	0x01000000,
	0x10683c30,
	0x00000000,
	0x01000000,
	0x106028e8,
	0x00000001,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0xfffffffe,
	0x02001000,
	0x106028f0,
	0x01000000,
	0x10683008,
	0x000007c1,
	0x01000000,
	0x106a0b00,
	0x00000900,
	0x01000000,
	0x106a0b04,
	0x00007fd5,
	0x01000000,
	0x106a0b08,
	0x41140001,
	0x01000000,
	0x106a0b14,
	0x06e00000,
	0x01000000,
	0x106a0b18,
	0x01000000,
	0x01000000,
	0x106a0b18,
	0x01002710,
	0x06000007,
	0x10683908,
	0x00003fda,
	0x01fff000,
	0x20000000,
	0x00000000,
	0x00002000,
	0x00000000,
	0x00000000,
	0x00002000,
	0x01000000,
	0x10683a10,
	0x00000000,
	0x06000002,
	0x10683928,
	0x00100401,
	0x00000610,
	0x000ef83d,
	0x06000007,
	0x10683908,
	0x00003fda,
	0x01fff000,
	0x20000000,
	0x00000000,
	0x00002000,
	0x00000000,
	0x00000000,
	0x00002000,
	0x01000000,
	0x10683a10,
	0x00000000,
	0x06000002,
	0x10683928,
	0x00100401,
	0x00000610,
	0x000ef83d,

	/* LIST: 4 (3 entries) */
	0x01000000,
	0x10642060,
	0xbaadf00d,

	/* LIST: 6 (227 entries) */
	0x00000000,
	0x01000000,
	0x10646400,
	0x00000100,
	0x01000000,
	0x10646400,
	0x00000000,
	0x01000000,
	0x10642044,
	0x00000013,
	0x01000000,
	0x10642040,
	0x00000100,
	0x01000000,
	0x10644034,
	0x00000000,
	0x01000000,
	0x10644038,
	0x00000000,
	0x01000000,
	0x10644010,
	0x00000000,
	0x01000000,
	0x10644014,
	0x00000000,
	0x06000002,
	0x10643074,
	0x0000051e,
	0x000001b4,
	0x0000004f,
	0x01000000,
	0x10643080,
	0x02000808,
	0x01000000,
	0x10643084,
	0x015cd0c8,
	0x01000000,
	0x1064308c,
	0x00000012,
	0x01000000,
	0x10643090,
	0x00032032,
	0x06000002,
	0x10643094,
	0x0000051e,
	0x000001b4,
	0x0000004f,
	0x01000000,
	0x106430a0,
	0x00007c7c,
	0x01000000,
	0x10643404,
	0x00000000,
	0x01000000,
	0x10643408,
	0x00000000,
	0x01000000,
	0x1064340c,
	0x00080008,
	0x01000000,
	0x10643410,
	0x00001111,
	0x01000000,
	0x10643414,
	0x04040404,
	0x01000000,
	0x10643418,
	0x04040404,
	0x01000000,
	0x10643018,
	0x00000008,
	0x06000001,
	0x1064200c,
	0x050002d0,
	0x0017b87a,
	0x01000000,
	0x10642014,
	0x0002ff34,
	0x01000000,
	0x10642018,
	0x000f0000,
	0x01000000,
	0x10642028,
	0x00000000,
	0x01000000,
	0x1064205c,
	0x00000000,
	0x01000000,
	0x106420d0,
	0x00000000,
	0x01000000,
	0x106420dc,
	0x00000000,
	0x01000000,
	0x106420d4,
	0x00000000,
	0x01000000,
	0x106420d8,
	0x00000000,
	0x06000003,
	0x10642080,
	0x050002d0,
	0x00000000,
	0x050002d0,
	0x00000000,
	0x01000000,
	0x10642090,
	0x00000000,
	0x01000000,
	0x10642094,
	0x00000001,
	0x01000000,
	0x10646400,
	0x00010000,
	0x01000000,
	0x10646400,
	0x00000000,
	0x01000000,
	0x1064105c,
	0x050002d0,
	0x06000003,
	0x10641030,
	0x00000000,
	0x00000000,
	0x00000000,
	0x0000ff00,
	0x01000000,
	0x10641028,
	0x00000000,
	0x06000003,
	0x106410f0,
	0x00000000,
	0x10000000,
	0x00000000,
	0x08000000,
	0x06000001,
	0x10641210,
	0x10000000,
	0x08000000,
	0x01000000,
	0x1064101c,
	0x00100000,
	0x01000000,
	0x1064121c,
	0x00000000,
	0x01000000,
	0x1064120c,
	0x00100000,
	0x01000000,
	0x10641204,
	0x000002d0,
	0x01000000,
	0x10641218,
	0x00000000,
	0x01000000,
	0x10641200,
	0x00000031,
	0x04000003,
	0x00000000,
	0x06000001,
	0x10641020,
	0x00000000,
	0x00000000,
	0x06000004,
	0x10641040,
	0x00000000,
	0x00000500,
	0x00000000,
	0x00000000,
	0x00000a00,
	0x01000000,
	0x10641208,
	0x00000000,
	0x04000003,
	0x00000000,
	0x03000000,
	0x106028ac,
	0x03000001,
	0x106028a0,
	0x0b000041,
	0x11000000,
	0x03000002,
	0x1060289c,
	0x0b000082,
	0x0d001082,
	0x130020c2,
	0x02002000,
	0x10641048,
	0x01000000,
	0x1064104c,
	0x00000000,
	0x03000002,
	0x10602898,
	0x14002002,
	0x00000001,
	0x02002000,
	0x10602898,
	0x06000001,
	0x10641014,
	0x00020565,
	0x000b0500,
	0x0600000e,
	0x10641100,
	0x000005db,
	0x000013a6,
	0x000001fc,
	0x00000200,
	0x00000000,
	0x0000fcc5,
	0x0000f52f,
	0x00000e0c,
	0x00001000,
	0x00000000,
	0x00000e0c,
	0x0000f33b,
	0x0000feb8,
	0x00001000,
	0x00000000,
	0x01000000,
	0x10641010,
	0x00000010,
	0x01000000,
	0x10641008,
	0x00000001,
	0x01000000,
	0x10642008,
	0x00000001,

	/* LIST: 8 (91 entries) */
	0x00000000,
	0x01000000,
	0x10643018,
	0x00000008,
	0x01000000,
	0x10644080,
	0x00000001,
	0x01000000,
	0x10644084,
	0x00000001,
	0x01000000,
	0x10644018,
	0x00000001,
	0x01000000,
	0x1064401c,
	0x00000001,
	0x06000001,
	0x1064200c,
	0x050002d0,
	0x0017b87a,
	0x01000000,
	0x10642014,
	0x0002ff34,
	0x01000000,
	0x10642018,
	0x000f0000,
	0x01000000,
	0x10642028,
	0x00000000,
	0x01000000,
	0x1064205c,
	0x00000000,
	0x01000000,
	0x106420d0,
	0x00000000,
	0x01000000,
	0x106420dc,
	0x00000000,
	0x01000000,
	0x106420d4,
	0x00000000,
	0x01000000,
	0x106420d8,
	0x00000000,
	0x06000003,
	0x10642080,
	0x050002d0,
	0x00000000,
	0x050002d0,
	0x00000000,
	0x01000000,
	0x10642090,
	0x00000000,
	0x01000000,
	0x10642094,
	0x00000001,
	0x04000003,
	0x00000000,
	0x04000003,
	0x00000000,
	0x03000000,
	0x106028ac,
	0x03000001,
	0x106028a0,
	0x0b000041,
	0x11000000,
	0x03000002,
	0x1060289c,
	0x0b000082,
	0x0d001082,
	0x130020c2,
	0x02002000,
	0x10641048,
	0x01000000,
	0x1064104c,
	0x00000000,
	0x03000002,
	0x10602898,
	0x14002002,
	0x00000001,
	0x02002000,
	0x10602898,
	0x01000000,
	0x10641010,
	0x00000010,
	0x01000000,
	0x10641008,
	0x00000001,
	0x01000000,
	0x10642008,
	0x00000001,

	/* LIST: 9 (70 entries) */
	0x00000000,
	0x01000000,
	0x10643018,
	0x00000008,
	0x06000001,
	0x1064200c,
	0x050002d0,
	0x0017b87a,
	0x01000000,
	0x10642014,
	0x0002ff34,
	0x01000000,
	0x10642018,
	0x000f0000,
	0x01000000,
	0x10642028,
	0x00000000,
	0x01000000,
	0x1064205c,
	0x00000000,
	0x01000000,
	0x106420d0,
	0x00000000,
	0x06000003,
	0x10642080,
	0x050002d0,
	0x00000000,
	0x050002d0,
	0x00000000,
	0x01000000,
	0x10642090,
	0x00000000,
	0x01000000,
	0x10642094,
	0x00000001,
	0x04000003,
	0x00000000,
	0x04000003,
	0x00000000,
	0x03000000,
	0x106028ac,
	0x03000001,
	0x106028a0,
	0x0b000041,
	0x11000000,
	0x03000002,
	0x1060289c,
	0x0b000082,
	0x0d001082,
	0x130020c2,
	0x02002000,
	0x10641048,
	0x01000000,
	0x1064104c,
	0x00000000,
	0x03000002,
	0x10602898,
	0x14002002,
	0x00000001,
	0x02002000,
	0x10602898,
	0x01000000,
	0x10641010,
	0x00000010,
	0x01000000,
	0x10641008,
	0x00000001,
	0x01000000,
	0x10642008,
	0x00000001,

	/* LIST: 10 (82 entries) */
	0x00000000,
	0x01000000,
	0x10643018,
	0x00000008,
	0x01000000,
	0x10644080,
	0x00000001,
	0x01000000,
	0x10644084,
	0x00000001,
	0x01000000,
	0x10644018,
	0x00000001,
	0x01000000,
	0x1064401c,
	0x00000001,
	0x06000001,
	0x1064200c,
	0x050002d0,
	0x0017b87a,
	0x01000000,
	0x10642014,
	0x0002ff34,
	0x01000000,
	0x10642018,
	0x000f0000,
	0x01000000,
	0x10642028,
	0x00000000,
	0x01000000,
	0x1064205c,
	0x00000000,
	0x01000000,
	0x106420d0,
	0x00000000,
	0x06000003,
	0x10642080,
	0x050002d0,
	0x00000000,
	0x050002d0,
	0x00000000,
	0x01000000,
	0x10642090,
	0x00000000,
	0x01000000,
	0x10642094,
	0x00000001,
	0x04000003,
	0x00000000,
	0x04000003,
	0x00000000,
	0x03000000,
	0x106028ac,
	0x03000001,
	0x106028a0,
	0x0b000041,
	0x11000000,
	0x03000002,
	0x1060289c,
	0x0b000082,
	0x0d001082,
	0x130020c2,
	0x02002000,
	0x10641048,
	0x01000000,
	0x1064104c,
	0x00000000,
	0x03000002,
	0x10602898,
	0x14002002,
	0x00000001,
	0x02002000,
	0x10602898,
	0x01000000,
	0x10641010,
	0x00000010,
	0x01000000,
	0x10641008,
	0x00000001,
	0x01000000,
	0x10642008,
	0x00000001,

	/* LIST: 12 (253 entries) */
	0x01000000,
	0x10683c30,
	0x00000004,
	0x01000000,
	0x10683c88,
	0x00000001,
	0x01000000,
	0x10683c8c,
	0x00000001,
	0x01000000,
	0x10683c90,
	0x00000001,
	0x01000000,
	0x10683c94,
	0x00000001,
	0x01000000,
	0x10683c88,
	0x00000000,
	0x01000000,
	0x10683c8c,
	0x00000000,
	0x01000000,
	0x10683c90,
	0x00000000,
	0x01000000,
	0x10683c94,
	0x00000000,
	0x01000000,
	0x106a0aa4,
	0x04530000,
	0x01000000,
	0x106a0ab0,
	0x00000001,
	0x04000000,
	0xfcffffff,
	0x04000001,
	0x02000000,
	0x03000002,
	0x106a0a98,
	0x0b002002,
	0x0d002042,
	0x02002000,
	0x106a0a98,
	0x04000000,
	0xc00fffff,
	0x04000001,
	0x2a300000,
	0x03000002,
	0x106a0a90,
	0x0b002002,
	0x0d002042,
	0x02002000,
	0x106a0a90,
	0x04000000,
	0xfffffff3,
	0x04000001,
	0x0000000c,
	0x03000002,
	0x106a0a9c,
	0x0b002002,
	0x0d002042,
	0x02002000,
	0x106a0a9c,
	0x01000000,
	0x10642060,
	0xbaadf00d,
	0x01000000,
	0x10642060,
	0xbaadf00d,
	0x01000000,
	0x10642060,
	0xbaadf00d,
	0x04000000,
	0xfffffff3,
	0x04000001,
	0x00000000,
	0x03000002,
	0x106a0a9c,
	0x0b002002,
	0x0d002042,
	0x02002000,
	0x106a0a9c,
	0x01000000,
	0x106a0b04,
	0x00007fd5,
	0x01000000,
	0x106a0b08,
	0x41140001,
	0x01000000,
	0x106a0b14,
	0x06e00000,
	0x01000000,
	0x106a0b18,
	0x01000000,
	0x01000000,
	0x106a0b18,
	0x01002710,
	0x0600003f,
	0x10682980,
	0x00648001,
	0x0064d004,
	0x00657001,
	0x00652013,
	0x006612d0,
	0x00652004,
	0x0065c001,
	0x00840000,
	0x0020401d,
	0x002d0028,
	0x002c00dc,
	0x00241500,
	0x00340185,
	0x00244028,
	0x00250028,
	0x002400dc,
	0x00241500,
	0x00340185,
	0x00204028,
	0x00210028,
	0x002000dc,
	0x00201500,
	0x00300185,
	0x00244028,
	0x00210028,
	0x002000dc,
	0x00201500,
	0x00300185,
	0x00204028,
	0x00210028,
	0x002000dc,
	0x00201500,
	0x00300185,
	0x00206028,
	0x00212028,
	0x002020dc,
	0x00223500,
	0x00302185,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00061209,
	0x7b899c3b,
	0x01000000,
	0x10682808,
	0x0000000b,
	0x01000000,
	0x10683010,
	0x00000249,
	0x01000000,
	0x10683014,
	0x000001d9,
	0x01000000,
	0x10683018,
	0x00000003,
	0x01000000,
	0x1068301c,
	0x00000040,
	0x01000000,
	0x10683020,
	0x02d00500,
	0x01000000,
	0x1068302c,
	0x0000c003,
	0x01000000,
	0x10683030,
	0x0000c000,
	0x01000000,
	0x106830f0,
	0x000005a0,
	0x01000000,
	0x106830fc,
	0x000002d1,
	0x01000000,
	0x10683a0c,
	0x00000000,
	0x06000007,
	0x10683908,
	0x00003fda,
	0x01fc0010,
	0x2000fa24,
	0x1482f15b,
	0x20003a0e,
	0xc5f20000,
	0x20000000,
	0xcebb3144,
	0x01000000,
	0x10683a10,
	0x00008000,
	0x06000002,
	0x10683928,
	0x00100401,
	0x00000610,
	0x000ef83d,
	0x01000000,
	0x10680018,
	0x00000003,
	0x01000000,
	0x10683c30,
	0x00000000,
	0x01000000,
	0x106028e8,
	0x00000001,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0xfffffffe,
	0x02001000,
	0x106028f0,
	0x01000000,
	0x10683008,
	0x000007c1,
	0x01000000,
	0x106a0b00,
	0x00000900,
	0x06000007,
	0x10683908,
	0x00003fda,
	0x01fc0010,
	0x2000fa24,
	0x1482f15b,
	0x20003a0e,
	0xc5f20000,
	0x20000000,
	0xcebb3144,
	0x01000000,
	0x10683a10,
	0x00008000,
	0x06000002,
	0x10683928,
	0x00100401,
	0x00000610,
	0x000ef83d,

	/* LIST: 13 (3 entries) */
	0x01000000,
	0x10642060,
	0xbaadf00d,

	/* LIST: 16 (87 entries) */
	0x00000000,
	0x01000000,
	0x10643018,
	0x00000008,
	0x06000001,
	0x1064200c,
	0x050002d0,
	0x0017b87a,
	0x01000000,
	0x10642014,
	0x0002ff34,
	0x01000000,
	0x10642018,
	0x000f0000,
	0x01000000,
	0x10642028,
	0x00000000,
	0x01000000,
	0x1064205c,
	0x00000000,
	0x01000000,
	0x106420d0,
	0x00000000,
	0x06000003,
	0x10642080,
	0x050002d0,
	0x00000000,
	0x050002d0,
	0x00000000,
	0x01000000,
	0x10642090,
	0x00000000,
	0x01000000,
	0x10642094,
	0x00000001,
	0x04000003,
	0x00000000,
	0x04000003,
	0x00000000,
	0x03000000,
	0x106028ac,
	0x03000001,
	0x106028a0,
	0x0b000041,
	0x11000000,
	0x03000002,
	0x1060289c,
	0x0b000082,
	0x0d001082,
	0x130020c2,
	0x02002000,
	0x10641048,
	0x01000000,
	0x1064104c,
	0x00000000,
	0x03000002,
	0x10602898,
	0x14002002,
	0x00000001,
	0x02002000,
	0x10602898,
	0x0600000e,
	0x10641100,
	0x000005db,
	0x000013a6,
	0x000001fc,
	0x00000200,
	0x00000000,
	0x0000fcc5,
	0x0000f52f,
	0x00000e0c,
	0x00001000,
	0x00000000,
	0x00000e0c,
	0x0000f33b,
	0x0000feb8,
	0x00001000,
	0x00000000,
	0x01000000,
	0x10641010,
	0x00000010,
	0x01000000,
	0x10641008,
	0x00000001,
	0x01000000,
	0x10642008,
	0x00000001,

	/* LIST: 17 (82 entries) */
	0x00000000,
	0x01000000,
	0x10643018,
	0x00000008,
	0x01000000,
	0x10644080,
	0x00000001,
	0x01000000,
	0x10644084,
	0x00000001,
	0x01000000,
	0x10644018,
	0x00000001,
	0x01000000,
	0x1064401c,
	0x00000001,
	0x06000001,
	0x1064200c,
	0x050002d0,
	0x0017b87a,
	0x01000000,
	0x10642014,
	0x0002ff34,
	0x01000000,
	0x10642018,
	0x000f0000,
	0x01000000,
	0x10642028,
	0x00000000,
	0x01000000,
	0x1064205c,
	0x00000000,
	0x01000000,
	0x106420d0,
	0x00000000,
	0x06000003,
	0x10642080,
	0x050002d0,
	0x00000000,
	0x050002d0,
	0x00000000,
	0x01000000,
	0x10642090,
	0x00000000,
	0x01000000,
	0x10642094,
	0x00000001,
	0x04000003,
	0x00000000,
	0x04000003,
	0x00000000,
	0x03000000,
	0x106028ac,
	0x03000001,
	0x106028a0,
	0x0b000041,
	0x11000000,
	0x03000002,
	0x1060289c,
	0x0b000082,
	0x0d001082,
	0x130020c2,
	0x02002000,
	0x10641048,
	0x01000000,
	0x1064104c,
	0x00000000,
	0x03000002,
	0x10602898,
	0x14002002,
	0x00000001,
	0x02002000,
	0x10602898,
	0x01000000,
	0x10641010,
	0x00000010,
	0x01000000,
	0x10641008,
	0x00000001,
	0x01000000,
	0x10642008,
	0x00000001,

	/* LIST: 19 (70 entries) */
	0x00000000,
	0x01000000,
	0x10643018,
	0x00000008,
	0x06000001,
	0x1064200c,
	0x050002d0,
	0x0017b87a,
	0x01000000,
	0x10642014,
	0x0002ff34,
	0x01000000,
	0x10642018,
	0x000f0000,
	0x01000000,
	0x10642028,
	0x00000000,
	0x01000000,
	0x1064205c,
	0x00000000,
	0x01000000,
	0x106420d0,
	0x00000000,
	0x06000003,
	0x10642080,
	0x050002d0,
	0x00000000,
	0x050002d0,
	0x00000000,
	0x01000000,
	0x10642090,
	0x00000000,
	0x01000000,
	0x10642094,
	0x00000001,
	0x04000003,
	0x00000000,
	0x04000003,
	0x00000000,
	0x03000000,
	0x106028ac,
	0x03000001,
	0x106028a0,
	0x0b000041,
	0x11000000,
	0x03000002,
	0x1060289c,
	0x0b000082,
	0x0d001082,
	0x130020c2,
	0x02002000,
	0x10641048,
	0x01000000,
	0x1064104c,
	0x00000000,
	0x03000002,
	0x10602898,
	0x14002002,
	0x00000001,
	0x02002000,
	0x10602898,
	0x01000000,
	0x10641010,
	0x00000010,
	0x01000000,
	0x10641008,
	0x00000001,
	0x01000000,
	0x10642008,
	0x00000001,
};

uint32_t g_aListCount9[] = 
{
	0,
	3,
};

uint32_t g_aList9[] = 
{

	/* LIST: 14 (3 entries) */
	0x01000000,
	0x10642060,
	0xbaadf00d,
};

uint32_t g_aListCount10[] = 
{
	0,
	880,
	883,
	1152,
	1228,
	1304,
};

uint32_t g_aList10[] = 
{

	/* LIST: 1 (880 entries) */
	0x01000000,
	0x10682304,
	0x00000021,
	0x01000000,
	0x10682308,
	0x00266180,
	0x01000000,
	0x1068230c,
	0x00200120,
	0x06000003,
	0x1068228c,
	0x00139400,
	0x00000018,
	0x2a098acb,
	0x0400003b,
	0x01000000,
	0x106822ac,
	0x00050000,
	0x01000000,
	0x10682204,
	0x00000000,
	0x06000013,
	0x10682600,
	0xffc0ffb0,
	0x01f0fa80,
	0x0b10ee50,
	0x1710e7c0,
	0x1310f820,
	0xfab00e90,
	0xf21003e0,
	0x0ba0eeb0,
	0x09101480,
	0xe9401180,
	0x5a602840,
	0xf9e0f530,
	0xfcd00050,
	0x0130fc60,
	0x0560f930,
	0x0680fb20,
	0x02a0ff60,
	0xff200180,
	0xfe700110,
	0xff500040,
	0x06000017,
	0x10682650,
	0x000c0004,
	0xffec0028,
	0xffc4003c,
	0xffd80000,
	0x002cffb8,
	0x0044ffe8,
	0xffd80064,
	0xff8c004c,
	0x000cff90,
	0x00b0ff64,
	0x00340064,
	0xff1c0100,
	0xff64ffd4,
	0x0100fe88,
	0x013cffb0,
	0xff0c01f4,
	0xfde4012c,
	0x0098fd98,
	0x035cfd4c,
	0x006402cc,
	0xfa9405e8,
	0xfce0fcf4,
	0x0b74ebf0,
	0x1a806320,
	0x01000000,
	0x106826b0,
	0x00007ff8,
	0x01000000,
	0x106826b4,
	0x00007ffc,
	0x01000000,
	0x106826b8,
	0x00000000,
	0x01000000,
	0x106826bc,
	0x12e02000,
	0x01000000,
	0x106826c0,
	0x10000000,
	0x01000000,
	0x106826c4,
	0x00000000,
	0x01000000,
	0x106826c8,
	0x00001f12,
	0x01000000,
	0x106826cc,
	0x00004dac,
	0x01000000,
	0x106826d0,
	0x07db073b,
	0x01000000,
	0x106826d4,
	0x00000000,
	0x01000000,
	0x106826d8,
	0x00000000,
	0x01000000,
	0x106826dc,
	0x00000000,
	0x01000000,
	0x106826e0,
	0x00000080,
	0x01000000,
	0x10683c28,
	0x00000000,
	0x06000007,
	0x10682388,
	0x00003fda,
	0x01fff000,
	0x140e0000,
	0xfd7e0000,
	0x00001106,
	0xeefa0000,
	0x00000000,
	0xe7fe1802,
	0x06000002,
	0x106823a8,
	0x00100401,
	0x00000535,
	0x000ffc01,
	0x06000025,
	0x10682008,
	0x0025c000,
	0x0401153e,
	0x0003ec10,
	0x0008966b,
	0x542a1500,
	0x64482e14,
	0x000000ef,
	0x00000415,
	0x00000c65,
	0x00004c44,
	0x000085b7,
	0x000046cd,
	0x00009f0c,
	0x0004acca,
	0x0009ab77,
	0x00a5f5af,
	0x00000085,
	0x000000f0,
	0x00000409,
	0x0000173e,
	0x00008656,
	0x00002940,
	0x0004d8b3,
	0x00013b0a,
	0x000adea5,
	0x000f0ffa,
	0x00000085,
	0x000000f0,
	0x00000409,
	0x0000173e,
	0x00008656,
	0x00002940,
	0x0004d8b3,
	0x00013b0a,
	0x000adea5,
	0x000f0ffa,
	0x000000ff,
	0x00000403,
	0x01000000,
	0x10682120,
	0x000040fb,
	0x01000000,
	0x1068211c,
	0x01c54000,
	0x01000000,
	0x1068212c,
	0x00000000,
	0x01000000,
	0x10683c1c,
	0x00000000,
	0x01000000,
	0x10683c20,
	0x00000000,
	0x01000000,
	0x10683c24,
	0x00000000,
	0x01000000,
	0x10683c2c,
	0x00000004,
	0x01000000,
	0x10683c58,
	0x00000001,
	0x01000000,
	0x10683c58,
	0x00000000,
	0x06000010,
	0x10681008,
	0xfdfd1400,
	0x00fdfdfd,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000006,
	0x03885332,
	0x0120c148,
	0x110036e5,
	0x05899b8b,
	0x04cc000c,
	0x00000015,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x01000000,
	0x106812b4,
	0x00000480,
	0x01000000,
	0x106812c0,
	0x00000241,
	0x01000000,
	0x1068106c,
	0x00000010,
	0x01000000,
	0x1068104c,
	0x012002d0,
	0x06000001,
	0x10681058,
	0x0000c003,
	0x0000c13c,
	0x060000ff,
	0x10681400,
	0x0020400c,
	0x00a24000,
	0x00288058,
	0x00240028,
	0x00207040,
	0x00205040,
	0x00204018,
	0x0021403e,
	0x00204039,
	0x0028400d,
	0x0028c00f,
	0x0028c1b5,
	0x00288075,
	0x002c800b,
	0x0028e040,
	0x0028c28a,
	0x00801000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00462003,
	0x00ad8000,
	0x00461001,
	0x00441004,
	0x00442005,
	0x00444002,
	0x00454022,
	0x0041c23c,
	0x0040c002,
	0x004c2005,
	0x004c1005,
	0x004c2005,
	0x004c4001,
	0x004d4022,
	0x0049c23c,
	0x0048c002,
	0x00404001,
	0x00442004,
	0x00462001,
	0x00a98000,
	0x00461001,
	0x00441004,
	0x00442005,
	0x00454024,
	0x0041c23c,
	0x0041c002,
	0x004c2005,
	0x004c1005,
	0x004c2005,
	0x004c4003,
	0x004d4020,
	0x002d4018,
	0x0049c23e,
	0x00404001,
	0x00442004,
	0x00462001,
	0x00815000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00200012,
	0x00a00006,
	0x0085f001,
	0x00400001,
	0x00200101,
	0x0065a008,
	0x00000000,
	0x00000000,
	0x0020000b,
	0x00400801,
	0x0085c002,
	0x0030007a,
	0x00200019,
	0x0020102d,
	0x00308034,
	0x00400001,
	0x002000c5,
	0x00666008,
	0x00000000,
	0x00000000,
	0x0020000b,
	0x00400801,
	0x00868004,
	0x0030009a,
	0x00200035,
	0x00201031,
	0x00308034,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00400806,
	0x00a00019,
	0x0068000f,
	0x00400801,
	0x00c08008,
	0x00c08008,
	0x00c01010,
	0x00c01010,
	0x00400928,
	0x00a00019,
	0x0068000f,
	0x00400801,
	0x00c08008,
	0x00c08008,
	0x00c01010,
	0x00c01010,
	0x00400923,
	0x0086e000,
	0x00400801,
	0x00c08008,
	0x00d01010,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00400003,
	0x00400126,
	0x0040000b,
	0x006a0002,
	0x0069c002,
	0x00400003,
	0x00400127,
	0x0040000b,
	0x006a0002,
	0x0069c002,
	0x00400003,
	0x00400126,
	0x0040000b,
	0x006a0002,
	0x0069c002,
	0x00400003,
	0x00400127,
	0x0040000b,
	0x006a0002,
	0x0069c002,
	0x00887000,
	0x00400001,
	0x0020003f,
	0x00202036,
	0x00300018,
	0x00400001,
	0x0020007f,
	0x00202036,
	0x00300018,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00400003,
	0x00400126,
	0x00400009,
	0x006c3004,
	0x006bf002,
	0x00400003,
	0x00400127,
	0x00400009,
	0x006c3004,
	0x006bf002,
	0x00400003,
	0x00400126,
	0x00400009,
	0x006c3004,
	0x006bf002,
	0x00400003,
	0x00400127,
	0x00400009,
	0x006c3004,
	0x006bf002,
	0x008aa000,
	0x00400001,
	0x0020003f,
	0x00202036,
	0x00300018,
	0x00400001,
	0x0020007f,
	0x00202036,
	0x00300018,
	0x00000000,
	0x00400003,
	0x00400126,
	0x0040000a,
	0x006e1003,
	0x006dd002,
	0x00400003,
	0x00400127,
	0x0040000a,
	0x006e1003,
	0x006dd002,
	0x00400003,
	0x00400126,
	0x0040000a,
	0x006e1003,
	0x006dd002,
	0x00400003,
	0x00400127,
	0x0040000a,
	0x006e1003,
	0x006dd002,
	0x008c8000,
	0x00400001,
	0x0020003f,
	0x00202036,
	0x00300018,
	0x00400001,
	0x0020007f,
	0x00202036,
	0x00300018,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x008fd000,
	0x00060110,
	0xb8006632,
	0x06000003,
	0x10682404,
	0x00007ffc,
	0x00000001,
	0x00200000,
	0x00000000,
	0x01000000,
	0x10680014,
	0x00000000,
	0x01000000,
	0x10683c2c,
	0x00000001,
	0x01000000,
	0x10683c1c,
	0x00000001,
	0x01000000,
	0x10683c20,
	0x00000001,
	0x01000000,
	0x10683c24,
	0x00000001,
	0x01000000,
	0x10683c5c,
	0x00000001,
	0x01000000,
	0x10683c5c,
	0x00000000,
	0x01000000,
	0x10683c60,
	0x00000001,
	0x01000000,
	0x10683c60,
	0x00000000,
	0x01000000,
	0x10683c64,
	0x00000001,
	0x01000000,
	0x10683c64,
	0x00000000,
	0x01000000,
	0x10682304,
	0x00000021,
	0x01000000,
	0x10682308,
	0x00266180,
	0x01000000,
	0x1068230c,
	0x00200120,
	0x06000003,
	0x1068228c,
	0x00139400,
	0x00000018,
	0x2a098acb,
	0x0400003b,
	0x01000000,
	0x106822ac,
	0x00050000,
	0x01000000,
	0x10682204,
	0x00000000,
	0x06000013,
	0x10682600,
	0xffc0ffb0,
	0x01f0fa80,
	0x0b10ee50,
	0x1710e7c0,
	0x1310f820,
	0xfab00e90,
	0xf21003e0,
	0x0ba0eeb0,
	0x09101480,
	0xe9401180,
	0x5a602840,
	0xf9e0f530,
	0xfcd00050,
	0x0130fc60,
	0x0560f930,
	0x0680fb20,
	0x02a0ff60,
	0xff200180,
	0xfe700110,
	0xff500040,
	0x06000017,
	0x10682650,
	0x000c0004,
	0xffec0028,
	0xffc4003c,
	0xffd80000,
	0x002cffb8,
	0x0044ffe8,
	0xffd80064,
	0xff8c004c,
	0x000cff90,
	0x00b0ff64,
	0x00340064,
	0xff1c0100,
	0xff64ffd4,
	0x0100fe88,
	0x013cffb0,
	0xff0c01f4,
	0xfde4012c,
	0x0098fd98,
	0x035cfd4c,
	0x006402cc,
	0xfa9405e8,
	0xfce0fcf4,
	0x0b74ebf0,
	0x1a806320,
	0x01000000,
	0x106826b0,
	0x00007ff8,
	0x01000000,
	0x106826b4,
	0x00007ffc,
	0x01000000,
	0x106826b8,
	0x00000000,
	0x01000000,
	0x106826bc,
	0x12e02000,
	0x01000000,
	0x106826c0,
	0x10000000,
	0x01000000,
	0x106826c4,
	0x00000000,
	0x01000000,
	0x106826c8,
	0x00001f12,
	0x01000000,
	0x106826cc,
	0x00004dac,
	0x01000000,
	0x106826d0,
	0x07db073b,
	0x01000000,
	0x106826d4,
	0x00000000,
	0x01000000,
	0x106826d8,
	0x00000000,
	0x01000000,
	0x106826dc,
	0x00000000,
	0x01000000,
	0x106826e0,
	0x00000080,
	0x01000000,
	0x10683c28,
	0x00000000,
	0x06000007,
	0x10682388,
	0x00003fda,
	0x01fff000,
	0x140e0000,
	0xfd7e0000,
	0x00001106,
	0xeefa0000,
	0x00000000,
	0xe7fe1802,
	0x06000002,
	0x106823a8,
	0x00100401,
	0x00000535,
	0x000ffc01,
	0x06000025,
	0x10682008,
	0x0025c000,
	0x0401153e,
	0x0003ec10,
	0x0008966b,
	0x542a1500,
	0x64482e14,
	0x000000ef,
	0x00000415,
	0x00000c65,
	0x00004c44,
	0x000085b7,
	0x000046cd,
	0x00009f0c,
	0x0004acca,
	0x0009ab77,
	0x00a5f5af,
	0x00000085,
	0x000000f0,
	0x00000409,
	0x0000173e,
	0x00008656,
	0x00002940,
	0x0004d8b3,
	0x00013b0a,
	0x000adea5,
	0x000f0ffa,
	0x00000085,
	0x000000f0,
	0x00000409,
	0x0000173e,
	0x00008656,
	0x00002940,
	0x0004d8b3,
	0x00013b0a,
	0x000adea5,
	0x000f0ffa,
	0x000000ff,
	0x00000403,
	0x01000000,
	0x10682120,
	0x000040fb,
	0x01000000,
	0x1068211c,
	0x01c54000,
	0x01000000,
	0x1068212c,
	0x00000000,
	0x01000000,
	0x10683c1c,
	0x00000000,
	0x01000000,
	0x10683c20,
	0x00000000,
	0x01000000,
	0x10683c24,
	0x00000000,
	0x01000000,
	0x106028ec,
	0x00000001,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0xfffffffd,
	0x02001000,
	0x106028f0,
	0x01000000,
	0x106812b4,
	0x00000480,
	0x01000000,
	0x106812c0,
	0x00000241,
	0x01000000,
	0x10680004,
	0x00001001,
	0x01000000,
	0x10680038,
	0x00000002,
	0x01000000,
	0x10680030,
	0x0000f04a,
	0x01000000,
	0x10680034,
	0x00000000,
	0x01000000,
	0x1068003c,
	0x00000003,
	0x01000000,
	0x10680054,
	0x00000e41,
	0x01000000,
	0x10681004,
	0x06017f0b,
	0x01000000,
	0x10682400,
	0x00000841,
	0x06000003,
	0x10682404,
	0x00007ffc,
	0x00000001,
	0x00200000,
	0x00000000,
	0x01000000,
	0x10682304,
	0x00000021,
	0x01000000,
	0x10682308,
	0x00266180,
	0x01000000,
	0x1068230c,
	0x00200120,
	0x06000003,
	0x1068228c,
	0x00139400,
	0x00000018,
	0x2a098acb,
	0x0400003b,
	0x01000000,
	0x106822ac,
	0x00050000,
	0x01000000,
	0x10682204,
	0x00000000,
	0x06000013,
	0x10682600,
	0xffc0ffb0,
	0x01f0fa80,
	0x0b10ee50,
	0x1710e7c0,
	0x1310f820,
	0xfab00e90,
	0xf21003e0,
	0x0ba0eeb0,
	0x09101480,
	0xe9401180,
	0x5a602840,
	0xf9e0f530,
	0xfcd00050,
	0x0130fc60,
	0x0560f930,
	0x0680fb20,
	0x02a0ff60,
	0xff200180,
	0xfe700110,
	0xff500040,
	0x06000017,
	0x10682650,
	0x000c0004,
	0xffec0028,
	0xffc4003c,
	0xffd80000,
	0x002cffb8,
	0x0044ffe8,
	0xffd80064,
	0xff8c004c,
	0x000cff90,
	0x00b0ff64,
	0x00340064,
	0xff1c0100,
	0xff64ffd4,
	0x0100fe88,
	0x013cffb0,
	0xff0c01f4,
	0xfde4012c,
	0x0098fd98,
	0x035cfd4c,
	0x006402cc,
	0xfa9405e8,
	0xfce0fcf4,
	0x0b74ebf0,
	0x1a806320,
	0x01000000,
	0x106826b0,
	0x00007ff8,
	0x01000000,
	0x106826b4,
	0x00007ffc,
	0x01000000,
	0x106826b8,
	0x00000000,
	0x01000000,
	0x106826bc,
	0x12e02000,
	0x01000000,
	0x106826c0,
	0x10000000,
	0x01000000,
	0x106826c4,
	0x00000000,
	0x01000000,
	0x106826c8,
	0x00001f12,
	0x01000000,
	0x106826cc,
	0x00004dac,
	0x01000000,
	0x106826d0,
	0x07db073b,
	0x01000000,
	0x106826d4,
	0x00000000,
	0x01000000,
	0x106826d8,
	0x00000000,
	0x01000000,
	0x106826dc,
	0x00000000,
	0x01000000,
	0x106826e0,
	0x00000080,
	0x01000000,
	0x10683c28,
	0x00000000,
	0x06000007,
	0x10682388,
	0x00003fda,
	0x01fff000,
	0x140e0000,
	0xfd7e0000,
	0x00001106,
	0xeefa0000,
	0x00000000,
	0xe7fe1802,
	0x06000002,
	0x106823a8,
	0x00100401,
	0x00000535,
	0x000ffc01,

	/* LIST: 2 (3 entries) */
	0x01000000,
	0x10642060,
	0xbaadf00d,

	/* LIST: 7 (269 entries) */
	0x01000000,
	0x10682304,
	0x00000021,
	0x01000000,
	0x10682308,
	0x00266180,
	0x01000000,
	0x1068230c,
	0x00200120,
	0x06000003,
	0x1068228c,
	0x00139400,
	0x00000018,
	0x2a098acb,
	0x0400003b,
	0x01000000,
	0x106822ac,
	0x00050000,
	0x01000000,
	0x10682204,
	0x00000000,
	0x06000013,
	0x10682600,
	0xffc0ffb0,
	0x01f0fa80,
	0x0b10ee50,
	0x1710e7c0,
	0x1310f820,
	0xfab00e90,
	0xf21003e0,
	0x0ba0eeb0,
	0x09101480,
	0xe9401180,
	0x5a602840,
	0xf9e0f530,
	0xfcd00050,
	0x0130fc60,
	0x0560f930,
	0x0680fb20,
	0x02a0ff60,
	0xff200180,
	0xfe700110,
	0xff500040,
	0x06000017,
	0x10682650,
	0x000c0004,
	0xffec0028,
	0xffc4003c,
	0xffd80000,
	0x002cffb8,
	0x0044ffe8,
	0xffd80064,
	0xff8c004c,
	0x000cff90,
	0x00b0ff64,
	0x00340064,
	0xff1c0100,
	0xff64ffd4,
	0x0100fe88,
	0x013cffb0,
	0xff0c01f4,
	0xfde4012c,
	0x0098fd98,
	0x035cfd4c,
	0x006402cc,
	0xfa9405e8,
	0xfce0fcf4,
	0x0b74ebf0,
	0x1a806320,
	0x01000000,
	0x106826b0,
	0x00007ff8,
	0x01000000,
	0x106826b4,
	0x00007ffc,
	0x01000000,
	0x106826b8,
	0x00000000,
	0x01000000,
	0x106826bc,
	0x12e02000,
	0x01000000,
	0x106826c0,
	0x10000000,
	0x01000000,
	0x106826c4,
	0x00000000,
	0x01000000,
	0x106826c8,
	0x00001f12,
	0x01000000,
	0x106826cc,
	0x00004dac,
	0x01000000,
	0x106826d0,
	0x07db073b,
	0x01000000,
	0x106826d4,
	0x00000000,
	0x01000000,
	0x106826d8,
	0x00000000,
	0x01000000,
	0x106826dc,
	0x00000000,
	0x01000000,
	0x106826e0,
	0x00000080,
	0x01000000,
	0x10683c28,
	0x00000000,
	0x06000007,
	0x10682388,
	0x00003fda,
	0x01fff000,
	0x140e0000,
	0xfd7e0000,
	0x00001106,
	0xeefa0000,
	0x00000000,
	0xe7fe1802,
	0x06000002,
	0x106823a8,
	0x00100401,
	0x00000535,
	0x000ffc01,
	0x00000000,
	0x01000000,
	0x10646400,
	0x00000200,
	0x01000000,
	0x10646400,
	0x00000000,
	0x01000000,
	0x10642844,
	0x00000013,
	0x01000000,
	0x10642840,
	0x00000100,
	0x06000001,
	0x1064280c,
	0x02d00120,
	0x001cb876,
	0x01000000,
	0x10642814,
	0x0002ff34,
	0x01000000,
	0x10642818,
	0x000f0000,
	0x01000000,
	0x10642828,
	0x00000000,
	0x01000000,
	0x1064285c,
	0x00000000,
	0x01000000,
	0x106428d0,
	0x00000000,
	0x01000000,
	0x106428dc,
	0x00000000,
	0x01000000,
	0x106428d4,
	0x00000000,
	0x01000000,
	0x106428d8,
	0x00000000,
	0x06000003,
	0x10642880,
	0x02d00120,
	0x00000000,
	0x02d00120,
	0x00000000,
	0x01000000,
	0x10642890,
	0x00000000,
	0x01000000,
	0x10642894,
	0x00000001,
	0x01000000,
	0x10646400,
	0x00020000,
	0x01000000,
	0x10646400,
	0x00000000,
	0x01000000,
	0x1064145c,
	0x02d00120,
	0x06000003,
	0x10641430,
	0x00000000,
	0x00000000,
	0x00000000,
	0x0000ff00,
	0x01000000,
	0x10641428,
	0x00000000,
	0x06000003,
	0x106414f0,
	0x00000000,
	0x10000000,
	0x00000000,
	0x08000000,
	0x01000000,
	0x1064141c,
	0x00100000,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0x00000002,
	0x12041002,
	0x14002003,
	0xffffffff,
	0x11003003,
	0x06000001,
	0x10641420,
	0x00000000,
	0x00000000,
	0x06000004,
	0x10641440,
	0x00000000,
	0x000002d0,
	0x00000000,
	0x00000000,
	0x00000b40,
	0x04000000,
	0x000005a0,
	0x0b0000c3,
	0x03000000,
	0x106028b4,
	0x130000c0,
	0x02000000,
	0x10641448,
	0x03000002,
	0x106028b0,
	0x14002002,
	0x00000001,
	0x02002000,
	0x106028b0,
	0x06000001,
	0x10641414,
	0x00020565,
	0x000b0500,
	0x0600000e,
	0x10641500,
	0x00000839,
	0x00001021,
	0x00000323,
	0x00000200,
	0x00000000,
	0x0000fb44,
	0x0000f6b0,
	0x00000e0c,
	0x00001000,
	0x00000000,
	0x00000e0c,
	0x0000f439,
	0x0000fdba,
	0x00001000,
	0x00000000,
	0x01000000,
	0x10641410,
	0x00000010,
	0x01000000,
	0x10641408,
	0x00000001,
	0x01000000,
	0x10642808,
	0x00000001,

	/* LIST: 15 (76 entries) */
	0x03000000,
	0x106028f0,
	0x10000001,
	0x00000002,
	0x02001000,
	0x106028f0,
	0x00000000,
	0x06000001,
	0x1064280c,
	0x02d00120,
	0x001cb876,
	0x01000000,
	0x10642814,
	0x0002ff34,
	0x01000000,
	0x10642818,
	0x000f0000,
	0x01000000,
	0x10642828,
	0x00000000,
	0x01000000,
	0x1064285c,
	0x00000000,
	0x01000000,
	0x106428d0,
	0x00000000,
	0x06000003,
	0x10642880,
	0x02d00120,
	0x00000000,
	0x02d00120,
	0x00000000,
	0x01000000,
	0x10642890,
	0x00000000,
	0x01000000,
	0x10642894,
	0x00000001,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0x00000002,
	0x12041002,
	0x14002003,
	0xffffffff,
	0x11003003,
	0x06000004,
	0x10641440,
	0x00000000,
	0x000002d0,
	0x00000000,
	0x00000000,
	0x00000b40,
	0x04000000,
	0x000005a0,
	0x0b0000c3,
	0x03000000,
	0x106028b4,
	0x130000c0,
	0x02000000,
	0x10641448,
	0x03000002,
	0x106028b0,
	0x14002002,
	0x00000001,
	0x02002000,
	0x106028b0,
	0x01000000,
	0x10641410,
	0x00000010,
	0x01000000,
	0x10641408,
	0x00000001,
	0x01000000,
	0x10642808,
	0x00000001,

	/* LIST: 20 (76 entries) */
	0x03000000,
	0x106028f0,
	0x10000001,
	0x00000002,
	0x02001000,
	0x106028f0,
	0x00000000,
	0x06000001,
	0x1064280c,
	0x02d00120,
	0x001cb876,
	0x01000000,
	0x10642814,
	0x0002ff34,
	0x01000000,
	0x10642818,
	0x000f0000,
	0x01000000,
	0x10642828,
	0x00000000,
	0x01000000,
	0x1064285c,
	0x00000000,
	0x01000000,
	0x106428d0,
	0x00000000,
	0x06000003,
	0x10642880,
	0x02d00120,
	0x00000000,
	0x02d00120,
	0x00000000,
	0x01000000,
	0x10642890,
	0x00000000,
	0x01000000,
	0x10642894,
	0x00000001,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0x00000002,
	0x12041002,
	0x14002003,
	0xffffffff,
	0x11003003,
	0x06000004,
	0x10641440,
	0x00000000,
	0x000002d0,
	0x00000000,
	0x00000000,
	0x00000b40,
	0x04000000,
	0x000005a0,
	0x0b0000c3,
	0x03000000,
	0x106028b4,
	0x130000c0,
	0x02000000,
	0x10641448,
	0x03000002,
	0x106028b0,
	0x14002002,
	0x00000001,
	0x02002000,
	0x106028b0,
	0x01000000,
	0x10641410,
	0x00000010,
	0x01000000,
	0x10641408,
	0x00000001,
	0x01000000,
	0x10642808,
	0x00000001,
};

uint32_t g_aListCount11[] = 
{
	0,
	85,
	161,
	237,
};

uint32_t g_aList11[] = 
{

	/* LIST: 11 (85 entries) */
	0x03000000,
	0x106028f0,
	0x10000001,
	0x00000002,
	0x02001000,
	0x106028f0,
	0x00000000,
	0x06000001,
	0x1064280c,
	0x02d00120,
	0x001cb876,
	0x01000000,
	0x10642814,
	0x0002ff34,
	0x01000000,
	0x10642818,
	0x000f0000,
	0x01000000,
	0x10642828,
	0x00000000,
	0x01000000,
	0x1064285c,
	0x00000000,
	0x01000000,
	0x106428d0,
	0x00000000,
	0x01000000,
	0x106428dc,
	0x00000000,
	0x01000000,
	0x106428d4,
	0x00000000,
	0x01000000,
	0x106428d8,
	0x00000000,
	0x06000003,
	0x10642880,
	0x02d00120,
	0x00000000,
	0x02d00120,
	0x00000000,
	0x01000000,
	0x10642890,
	0x00000000,
	0x01000000,
	0x10642894,
	0x00000001,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0x00000002,
	0x12041002,
	0x14002003,
	0xffffffff,
	0x11003003,
	0x06000004,
	0x10641440,
	0x00000000,
	0x000002d0,
	0x00000000,
	0x00000000,
	0x00000b40,
	0x04000000,
	0x000005a0,
	0x0b0000c3,
	0x03000000,
	0x106028b4,
	0x130000c0,
	0x02000000,
	0x10641448,
	0x03000002,
	0x106028b0,
	0x14002002,
	0x00000001,
	0x02002000,
	0x106028b0,
	0x01000000,
	0x10641410,
	0x00000010,
	0x01000000,
	0x10641408,
	0x00000001,
	0x01000000,
	0x10642808,
	0x00000001,

	/* LIST: 18 (76 entries) */
	0x03000000,
	0x106028f0,
	0x10000001,
	0x00000002,
	0x02001000,
	0x106028f0,
	0x00000000,
	0x06000001,
	0x1064280c,
	0x02d00120,
	0x001cb876,
	0x01000000,
	0x10642814,
	0x0002ff34,
	0x01000000,
	0x10642818,
	0x000f0000,
	0x01000000,
	0x10642828,
	0x00000000,
	0x01000000,
	0x1064285c,
	0x00000000,
	0x01000000,
	0x106428d0,
	0x00000000,
	0x06000003,
	0x10642880,
	0x02d00120,
	0x00000000,
	0x02d00120,
	0x00000000,
	0x01000000,
	0x10642890,
	0x00000000,
	0x01000000,
	0x10642894,
	0x00000001,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0x00000002,
	0x12041002,
	0x14002003,
	0xffffffff,
	0x11003003,
	0x06000004,
	0x10641440,
	0x00000000,
	0x000002d0,
	0x00000000,
	0x00000000,
	0x00000b40,
	0x04000000,
	0x000005a0,
	0x0b0000c3,
	0x03000000,
	0x106028b4,
	0x130000c0,
	0x02000000,
	0x10641448,
	0x03000002,
	0x106028b0,
	0x14002002,
	0x00000001,
	0x02002000,
	0x106028b0,
	0x01000000,
	0x10641410,
	0x00000010,
	0x01000000,
	0x10641408,
	0x00000001,
	0x01000000,
	0x10642808,
	0x00000001,

	/* LIST: 21 (76 entries) */
	0x03000000,
	0x106028f0,
	0x10000001,
	0x00000002,
	0x02001000,
	0x106028f0,
	0x00000000,
	0x06000001,
	0x1064280c,
	0x02d00120,
	0x001cb876,
	0x01000000,
	0x10642814,
	0x0002ff34,
	0x01000000,
	0x10642818,
	0x000f0000,
	0x01000000,
	0x10642828,
	0x00000000,
	0x01000000,
	0x1064285c,
	0x00000000,
	0x01000000,
	0x106428d0,
	0x00000000,
	0x06000003,
	0x10642880,
	0x02d00120,
	0x00000000,
	0x02d00120,
	0x00000000,
	0x01000000,
	0x10642890,
	0x00000000,
	0x01000000,
	0x10642894,
	0x00000001,
	0x03000000,
	0x106028f0,
	0x0c000001,
	0x00000002,
	0x12041002,
	0x14002003,
	0xffffffff,
	0x11003003,
	0x06000004,
	0x10641440,
	0x00000000,
	0x000002d0,
	0x00000000,
	0x00000000,
	0x00000b40,
	0x04000000,
	0x000005a0,
	0x0b0000c3,
	0x03000000,
	0x106028b4,
	0x130000c0,
	0x02000000,
	0x10641448,
	0x03000002,
	0x106028b0,
	0x14002002,
	0x00000001,
	0x02002000,
	0x106028b0,
	0x01000000,
	0x10641410,
	0x00000010,
	0x01000000,
	0x10641408,
	0x00000001,
	0x01000000,
	0x10642808,
	0x00000001,
};


struct stTriggerMap
{
	int TriggerHwNum;             /* The hardware position of the trigger */
	int SlotNum;                  /* The RDMA slot correposponding to the trigger */
	int ListCountMaxIndex;       /* The Max Index of the ListCount Array */
	uint32_t *aListCountArray; /* The list count array pointer */
	uint32_t *aListArray;      /* The list array pointer */
	} g_aTriggerMap[] = 
{
 { -1, -1, 0, NULL, NULL },
 { -1, -1, 0, NULL, NULL },
 { -1, -1, 0, NULL, NULL },
 { -1, -1, 0, NULL, NULL },
 { -1, -1, 0, NULL, NULL },
 { -1, -1, 0, NULL, NULL },
 { -1, -1, 0, NULL, NULL },
 { -1, -1, 0, NULL, NULL },
 { 8, 4, 11, g_aListCount8, g_aList8 },
 { 9, 5, 1, g_aListCount9, g_aList9 },
 { 10, 6, 5, g_aListCount10, g_aList10 },
 { 11, 7, 3, g_aListCount11, g_aList11 },

};

