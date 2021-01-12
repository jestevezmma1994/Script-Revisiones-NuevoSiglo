/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_TTML_SUBTITLE_H_
#define QB_TTML_SUBTITLE_H_

#include "QBTTMLSubsParser.h"
#include <QBSubsManager.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <CAGE/Text/SvFont.h>
#include <QBSMPTETTPngDecoder.h>

/**
 * @defgroup QBTTMLSubtitle TTML Subtitle
 * @ingroup QBTTMLSubtitle
 * @{
 **/

/**
 *  TTML Subtitle Render parameters.
 **/
typedef struct QBTTMLSubtitleRenderParams_ {
    SvFont font;                                //< font used to render subtitles
    unsigned int baseFontSize;                  //< font size used to render subtitles
    double baseFontAspect;                      //< font aspect used to render subtitles
    unsigned int nativeWidth;
    unsigned int nativeHeight;

    unsigned int lineHeight;
    unsigned int baseLinePos;

    struct QBSubsManualFontParams_ manual_;    //< manually configured subtitles font parameters
} *QBTTMLSubtitleRenderParams;

typedef struct QBTTMLSubtitle_ *QBTTMLSubtitle;

/**
 * Create QBTTMLSubtitle class instance.
 *
 * @memberof QBTTMLSubtitle
 *
 * @param[in]  paragraphs      array of QBTTMLParagraph instances
 * @param[in]  divs            array of QBTTMLDiv instances
 * @param[in]  pngDecoder      QBSMPTETTPngDecoder instance
 * @param[in]  params          render parameters
 * @param[out] errorOut        error information
 * @return                     QBTTMLSubtitle instance
 **/
QBTTMLSubtitle
QBTTMLSubtitleCreate(SvArray paragraphs,
                     SvArray divs,
                     QBSMPTETTPngDecoder pngDecoder,
                     QBTTMLSubtitleRenderParams params,
                     SvErrorInfo *errorOut);

/**
  * @}
  **/

#endif
