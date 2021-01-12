/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_EPG_TEXT_RENDERER_H_
#define SV_EPG_TEXT_RENDERER_H_

/**
 * @file SvEPGTextRenderer.h
 * @brief EPG text renderer class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvString.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvFont.h>
#include <CUIT/Core/types.h>


/**
 * @defgroup SvEPGTextRenderer EPG text renderer class
 * @ingroup SvEPG
 * @{
 **/

/**
 * EPG text renderer class.
 * @class SvEPGTextRenderer
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 *
 * This class expects to be initialized by @link QBInitializable @endlink
 * interface init method and has following dependencies that shall be passed in
 * properties map:
 *
 * | Name            | Type              | Description
 * | --------------- | ----------------- | ------------------------------------------
 * | maxMemoryUsage  | size_t            | memory limit, minimum value 65536
 * | channelsCount   | unsigned int      | channels count, minimum value 16
 **/
typedef struct SvEPGTextRenderer_ *SvEPGTextRenderer;

/**
 * Get runtime type identification object representing SvEPGTextRenderer class.
 *
 * @return EPG text renderer class
 **/
extern SvType
SvEPGTextRenderer_getType(void);

/**
 * Pause or unpause EPG text renderer.
 *
 * @param[in] self      EPG text renderer handle
 * @param[in] pause     @c true to pause, @c false to unpause
 * @param[out] errorOut error info
 **/
extern void
SvEPGTextRendererPause(SvEPGTextRenderer self,
                       bool pause,
                       SvErrorInfo *errorOut);

/**
 * Set information used for computing rendering priority.
 *
 * @param[in] self      EPG text renderer handle
 * @param[in] channelIndex index of the channel with highest priority
 * @param[in] timeMarker time point with highest priority
 **/
extern void
SvEPGTextRendererSetPriority(SvEPGTextRenderer self,
                             int channelIndex,
                             time_t timeMarker);

/**
 * Horizontal text alignment.
 **/
typedef enum {
   SvEPGTextAlignment_left,
   SvEPGTextAlignment_center,
   SvEPGTextAlignment_right
} SvEPGTextAlignment;

/**
 * Render some text.
 *
 * @param[in] self      EPG text renderer handle
 * @param[in] text      text to be rendered
 * @param[in] font      font handle
 * @param[in] fontSize  font size
 * @param[in] textColor color of the rendered text
 * @param[in] maxWidth  max width of the text in pixels or @c 0
 * @param[in] maxLines  max number of lines of rendered text or @c 0
 * @param[in] frame     widget this text should be attached to
 * @param[out] errorOut error info
 * @return              bitmap that will contain rendered text,
 *                      @c NULL in case of error
 **/
extern SvBitmap
SvEPGTextRendererRender(SvEPGTextRenderer self,
                        int channelIndex, time_t timeMarker,
                        SvString text, SvFont font, unsigned int fontSize,
                        SvColor textColor,
                        SvEPGTextAlignment alignment, unsigned int maxWidth,
                        unsigned int maxLines,
                        SvWidget frame,
                        SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
