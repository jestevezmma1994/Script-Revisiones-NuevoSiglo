/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TEXT_RENDERER_H_
#define QB_TEXT_RENDERER_H_

#include <stddef.h>
#include <stdbool.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvFont.h>
#include <CAGE/Text/SvTextLayout.h>
#include <sv-list.h>

/**
 * Text renderer service.
 *
 * @class QBTextRenderer QBTextRenderer.h <Services/core/QBTextRenderer.h>
 * @extends SvObject
 * @implements QBAsyncService
 **/
typedef struct QBTextRenderer_ *QBTextRenderer;
typedef struct QBTextRendererTask_ *QBTextRendererTask;
typedef struct QBTextRendererKey_ *QBTextRendererKey;

typedef void (*QBTextRendererCallback)(void *self, QBTextRendererTask task);
typedef void (*QBTextRendererLayoutCallback)(void *self, QBTextRendererTask task, SvTextLayout layout);

typedef enum QBTextRendererTaskType_t {
    QBTextRendererTask_Auto,
    QBTextRendererTask_Freeable,
} QBTextRendererTaskType;

/**
 * QBTextRenderer rendering mode.
 */
typedef enum {
    QBTextRendererMode_renderAll,
    QBTextRendererMode_onlyLayout,
} QBTextRendererMode;

/**
 * QBTextRenderer parameters.
 */
typedef struct {
    SvString text;                         ///< text to be rendered
    QBTextRendererCallback callback;       ///< callback called after element is rendered
    void *callbackData;                    ///< callback data
    QBTextRendererLayoutCallback layoutCallback; ///< callback called after layout is prepared
    void *layoutCallbackData;              ///< callback data used with callbackLayout
    SvFont font;                           ///< handle to a font to be used for @a text
    unsigned int fontSize;                 ///< font size in pixels
    SvColor textColor;                     ///< text color to be displayed
    int maxWidth;                          ///< max width of text box in pixels
    QBTextRendererMode mode;               ///< rendering mode
} QBTextRendererRenderParams;

/**
 * @relates QBTextRenderer
 *
 * Get runtime type identification object representing text renderer class.
 *
 * @return QBTextRenderer type identification object
 */
SvType QBTextRenderer_getType(void);

/**
 * Create text renderer object
 *
 * QBTextRenderer should be started before use, see QBAsyncService::start
 *
 * @param[in]  limit            limit size
 * @param[in]  persistentLimit  persistent limit size
 * @param[out] errorOut         error info
 * @return                      text renderer object or @c NULL on failure
 */
QBTextRenderer QBTextRendererCreate(size_t limit, size_t persistentLimit, SvErrorInfo *errorOut);

/**
 * Render a text string to a bitmap.
 *
 * @param[in] self                  text renderer
 * @param[in] params                QBTextRendererRenderParams handle
 * @return                          bitmap with rendered text
 */
SvBitmap QBTextRendererRenderFromParams(QBTextRenderer self, const QBTextRendererRenderParams *params);

/**
 * Render a text string to a bitmap.
 *
 * @deprecated This method is deprecated and you should not to use it in new code.
 *
 * @param[in] self                  text renderer
 * @param[in] text                  text to be rendered
 * @param[in] callback              callback called after element is rendered
 * @param[in] callbackData          callback data
 * @param[in] font                  handle to a font to be used for @a text
 * @param[in] fontSize              font size in pixels
 * @param[in] textColor             text color to be displayed
 * @param[in] maxWidth              max width of text box in pixels
 * @return                          bitmap with rendered text
 */
SvBitmap QBTextRendererRender(QBTextRenderer self, SvString text,
                              QBTextRendererCallback callback, void *callbackData,
                              SvFont font, unsigned int fontSize, SvColor textColor, int maxWidth);

struct QBTextRendererTask_ {
    struct SvObject_ super_;

    SvColor textColor;
    SvString text;
    unsigned int fontSize;
    SvFont font;
    unsigned int maxWidth; ///< max bitmap width in pixels, @c 0 if there is no limit
    QBTextRendererCallback callback; ///< callback called after all label is prepared
    void* callbackData; ///< callback data
    QBTextRendererLayoutCallback layoutCallback; ///< callback called after layout is prepared
    void* layoutCallbackData; ///< layout callback data
    QBTextRenderer renderer;
    unsigned int maxLines;
    SvString textEnd;
    bool isMarkup;
    QBTextRendererMode mode; ///< rendering mode

    QBTextRendererKey key;
    SvBitmap surface;
    SvTextAlignment alignment;

    QBTextRendererTaskType type;
    bool persistent;

    list_linkage_t lru;

    // attributes set when task is rendered
    bool truncated;
    int linesCount;
    Sv2DRect box;
    int realSizeLastLineMaxDescender;
};

SvBitmap QBTextRendererTaskStart(QBTextRendererTask task);

/**
 * Create QBTextRendererTask.
 *
 * @param[in] renderer              text renderer
 * @param[in] params                QBTextRendererRenderParams handle
 * @return                          new renderer task or @c NULL in case of error
 */
QBTextRendererTask QBTextRendererTaskNewFromParams(QBTextRenderer renderer, const QBTextRendererRenderParams *params);

/**
 * Create QBTextRendererTask.
 *
 * @deprecated This method is deprecated and you should not to use it in new code.
 *
 * @param[in] renderer              text renderer
 * @param[in] text                  text to be rendered
 * @param[in] callback              callback called after element is rendered
 * @param[in] callbackData          callback data
 * @param[in] font                  handle to a font to be used for @a text
 * @param[in] fontSize              font size in pixels
 * @param[in] textColor             text color to be displayed
 * @param[in] maxWidth              max width of text box in pixels
 * @return                          new renderer task or @c NULL in case of error
 */
QBTextRendererTask QBTextRendererTaskNew(QBTextRenderer renderer, SvString text,
                                         QBTextRendererCallback callback, void *callbackData,
                                         SvFont font, unsigned int fontSize, SvColor textColor, int maxWidth);

void QBTextRendererAddPersistentText(QBTextRenderer self, SvString text);


#endif
