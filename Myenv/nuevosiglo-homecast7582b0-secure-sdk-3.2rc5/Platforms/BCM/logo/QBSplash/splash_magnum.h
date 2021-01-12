#ifndef SPLASH_MAGNUM_H
#define SPLASH_MAGNUM_H

typedef void * BREG_Handle ;
typedef void * BMEM_Handle ;

/* Private pixel format types */
#define BPXL_P_ALPHA           0x01000000
#define BPXL_P_COLOR           0x02000000
#define BPXL_P_RGB             0x04000000
#define BPXL_P_YCbCr           0x08000000
#define BPXL_P_PALETTE         0x10000000
#define BPXL_P_SPECIAL         0x20000000
#define BPXL_P_WINDOW          0x40000000
#define BPXL_P_TYPE_MASK       0x7F000000

/* Private pixel component information */
#define BPXL_P_COMP_LOC_SHIFT  16

/* Private pixel component locations */
#define BPXL_P_LOC_A(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 3 * 2))
#define BPXL_P_LOC_R(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 2 * 2))
#define BPXL_P_LOC_G(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 1 * 2))
#define BPXL_P_LOC_B(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 0 * 2))
#define BPXL_P_LOC_Y(p)        ((p) << (BPXL_P_COMP_LOC_SHIFT + 2 * 2))
#define BPXL_P_LOC_Cb(p)       ((p) << (BPXL_P_COMP_LOC_SHIFT + 1 * 2))
#define BPXL_P_LOC_Cr(p)       ((p) << (BPXL_P_COMP_LOC_SHIFT + 0 * 2))
#define BPXL_P_LOC_ARGB        (BPXL_P_LOC_A(3)  | BPXL_P_LOC_R(2)  | BPXL_P_LOC_G(1)  | BPXL_P_LOC_B(0))
#define BPXL_P_LOC_AYCbCr      (BPXL_P_LOC_A(3)  | BPXL_P_LOC_Y(2)  | BPXL_P_LOC_Cb(1) | BPXL_P_LOC_Cr(0))
#define BPXL_P_YCbCr           0x08000000

typedef enum BPXL_Format
{
	/* RGB */
	BPXL_eA8_R8_G8_B8      = BPXL_P_ALPHA | BPXL_P_COLOR | BPXL_P_RGB | BPXL_P_LOC_ARGB | 0x8888,

	BPXL_eR5_G6_B5         = BPXL_P_COLOR | BPXL_P_RGB | BPXL_P_LOC_ARGB | 0x0565,
	BPXL_eA8_Y8_Cb8_Cr8    = BPXL_P_ALPHA | BPXL_P_COLOR | BPXL_P_YCbCr | BPXL_P_LOC_AYCbCr | 0x8888,

	BPXL_INVALID           = 0
} BPXL_Format;

typedef int BFMT_VideoFmt ;


#endif // SPLASH_MAGNUM_H
