/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef DSPLINK_ENGINE_H_
#define DSPLINK_ENGINE_H_

/**
 * @file dsplink_engine.h Compatibility dsplink interface
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup dsplink Compatibility dsplink interface
 * @ingroup SvGfxHAL
 * @{
 *
 * Wrapper for GFX HAL API that provides compatibility for code using old @a dsplink API.
 **/

/**
 * GFX output standard.
 **/
typedef enum {
    SV_OUTPUT_STANDARD_CURRENT = 0,
    SV_OUTPUT_STANDARD_NONE,
    SV_OUTPUT_STANDARD_1080i59,
    SV_OUTPUT_STANDARD_1080i50,
    SV_OUTPUT_STANDARD_720p59,
    SV_OUTPUT_STANDARD_720p50,
    SV_OUTPUT_STANDARD_480i59,
    SV_OUTPUT_STANDARD_480p59,
    SV_OUTPUT_STANDARD_576i50,
    SV_OUTPUT_STANDARD_576p50,
    SV_OUTPUT_STANDARD_1080p50,
    SV_OUTPUT_STANDARD_1080p59,
    SV_OUTPUT_STANDARD_2160p24,
    SV_OUTPUT_STANDARD_2160p25,
    SV_OUTPUT_STANDARD_2160p30,
    SV_OUTPUT_STANDARD_4096x2160p24,
    SV_OUTPUT_STANDARD_CNT // this must be the last one
} SvOutputStandardType;

/**
 * GFX and video aspect ratio.
 **/
typedef enum {
    SV_ASPECT_RATIO_16x9, /**< 16:9 aspect ratio */
    SV_ASPECT_RATIO_4x3   /**< 4:3 aspect ratio */
} SvAspectRatioType;


/**
 * Initialize the GFX engine.
 *
 * @deprecated This function is deprecated and will be removed in the future.
 *
 * @param[in] width     desired GFX display width
 * @param[in] height    desired GFX display height
 * @param[in] gui_ar    aspect ratio of GFX framebuffer
 * @param[in] sd_mode   output mode of SD output
 * @param[in] sd_ar     aspect ratio of SD output
 * @param[in] hd_mode   output mode of HD output
 * @param[in] hd_ar     aspect ratio of HD output
 * @return              non-zero on success, 0 otherwise
 **/
extern int
svDspAreaInit(unsigned int width, unsigned int height,
              SvAspectRatioType gui_ar,
              SvOutputStandardType sd_mode,
              SvAspectRatioType sd_ar,
              SvOutputStandardType hd_mode,
              SvAspectRatioType hd_ar);

/**
 * Uninitialize the GFX engine and free all resources.
 *
 * @deprecated This function is deprecated and will be removed in the future.
 **/
extern void
svDspDeinit(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
