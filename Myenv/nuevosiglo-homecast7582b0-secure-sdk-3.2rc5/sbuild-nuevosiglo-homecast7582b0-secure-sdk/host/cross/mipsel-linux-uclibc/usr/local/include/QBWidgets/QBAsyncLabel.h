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

#ifndef QBASYNCLABEL_H_
#define QBASYNCLABEL_H_

#include <stdbool.h>
#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>

/**
 * @file QBAsyncLabel.h QBAsyncLabel class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBAsyncLabel QBAsyncLabel: Asynchronous label widget
 * @ingroup QBWidgets
 * @{
 *
 * Asynchronous label widget created from the Settings Manager
 **/


/**
 * Create label widget using settings from the Settings Manager.
 *
 * This method creates asynchronous label widget. Appearance of this widget is
 * controlled by the Settings Manager. If smallFontSize and fontSizeSwitchLength is set
 * smallFontSize will be used when text length is equal to or exceeds fontSizeSwitchLength.
 *
 * It will use following parameters:
 *
 *  - width, height : integer, required,
 *  - font : font, required,
 *  - fontSize : integer, required,
 *  - smallFontSize : integer, optional,
 *  - fontSizeSwitchLength : integer, optional,
 *  - textColor : color, required,
 *  - adjustWidth : boolean, optional,
 *  - maxWidth : integer, optional,
 *  - isMarkup : boolean, optional.
 *
 * @param[in] app             CUIT application handle
 * @param[in] widgetName      widget name
 * @param[in] renderer        QBTextRenderer for async text rendering
 * @return                    newly created widget or NULL in case of error
 **/
SvWidget QBAsyncLabelNew(SvApplication app, char const *widgetName, QBTextRenderer renderer);

/**
 * Create label widget using ratio settings from the Settings Manager.
 *
 * This method creates asynchronous label widget. Appearance of this widget is
 * controlled by the Settings Manager. If smallFontSizeRatio and fontSizeSwitchLength is set
 * smallFontSizeRatio will be used when text length is equal to or exceeds fontSizeSwitchLength.
 * If ratio font sizes are invalid or not found, it will try to use normal font sizes.
 *
 * It will use following parameters:
 *
 *  - widthRatio, heightRatio : double, required,
 *  - font : font, required,
 *  - fontSizeRatio : double, required,
 *  - fontSize : integer, required,
 *  - smallFontSizeRatio : double, optional,
 *  - smallFontSize : integer, optional,
 *  - fontSizeSwitchLength : integer, optional,
 *  - textColor : color, required,
 *  - adjustWidth : boolean, optional,
 *  - maxWidth : integer, optional,
 *  - isMarkup : boolean, optional.
 *
 * @param[in] app             CUIT application handle
 * @param[in] parentWidth     parent width
 * @param[in] parentHeight    parent height
 * @param[in] name            widget name
 * @param[in] renderer        QBTextRenderer for async text rendering
 * @param[out] errorOut       error handle
 * @return                    newly created widget or NULL in case of error
 **/
SvWidget
QBAsyncLabelNewFromRatioSettings(SvApplication app,
                                 unsigned int parentWidth,
                                 unsigned int parentHeight,
                                 const char *name,
                                 QBTextRenderer renderer,
                                 SvErrorInfo *errorOut);

/**
 * Set Label contents.
 *
 * @param[in] label          Label widget handle
 * @param[in] text           text to be displayed
 **/
void QBAsyncLabelSetText(SvWidget label, SvString text);

/**
 * Set label context (same as QBAsyncLabelSetText(), but different type of parameters)
 * @param[in] label          Label widget handle
 * @param[in] ctext          text to be displayed
 **/
void QBAsyncLabelSetCText(SvWidget label, const char *ctext);

/**
 * Set label text color
 * @param[in] label          Label widget handle
 * @param[in] textColor      text color to be displayed
 **/
void QBAsyncLabelSetTextColor(SvWidget label, SvColor textColor);

/**
 * Clear the widget, it will not display nothing.
 * @param[in] label          Label widget handle
 **/
void QBAsyncLabelClear(SvWidget label);

/**
 * Sets maximal label width. If text exceedes it, it will either be multilined
 * (if settings allowed) or just cut off.
 * @param[in] label          Label widget handle
 * @param[in] width          new maximal width
 **/
void QBAsyncLabelSetWidth(SvWidget label, int width);

/**
 * Set Label text color.
 *
 * @param[in] label          Label widget handle
 * @param[in] color          Label text color to be set
 **/
void QBAsyncLabelSetColor(SvWidget label, SvColor color);

/**
 * Caches async label parameters from settings file. Used to reduce
 * calls to settings manager.
 * @class QBAsyncLabelConstructor
 **/
typedef struct QBAsyncLabelConstructor_t *QBAsyncLabelConstructor;

/**
 * Creates a new instance of async label constructor. It will cache
 * all parameters from settings that are used to create a defined
 * label.
 *
 * @memberof QBAsyncLabelConstructor
 * @param[in] app            application handle
 * @param[in] widgetName     name of the widget in current settings file
 * @param[in] renderer       async text renderer handle
 * @param[out] errorOut      error handle
 * @return new instance of QBAsyncLabelConstructor
 **/
QBAsyncLabelConstructor
QBAsyncLabelConstructorCreate(SvApplication app, char const *widgetName, QBTextRenderer renderer, SvErrorInfo *errorOut);

/**
 * Creates a new instance of async label constructor from ratio settings. It will cache
 * all parameters from settings that are used to create a defined
 * label.
 *
 * @memberof QBAsyncLabelConstructor
 * @param[in] app            application handle
 * @param[in] parentWidth    parent width
 * @param[in] parentHeight   parent height
 * @param[in] name           widget name
 * @param[in] renderer       async text renderer handle
 * @param[out] errorOut      error handle
 * @return new instance of QBAsyncLabelConstructor
 **/
QBAsyncLabelConstructor
QBAsyncLabelConstructorCreateFromRatioSettings(SvApplication app,
                                               unsigned int parentWidth,
                                               unsigned int parentHeight,
                                               const char *name,
                                               QBTextRenderer renderer,
                                               SvErrorInfo *errorOut);

/**
 * Creates a new instance of async label based on constructor and not
 * directly on settings.
 * @param[in] app            application handle
 * @param[in] constructor    cached async label settings
 * @return new instance of async label
 **/
SvWidget
QBAsyncLabelNewFromConstructor(SvApplication app, QBAsyncLabelConstructor constructor);

/**
 * Returns real text width. If the widget is resized to this
 * width it will not cut off text.
 * @param[in] label          Label widget handle
 * @return real text width
 **/
int
QBAsyncLabelGetWidth(SvWidget label);

/**
 * Returns real text height. If the widget is resized to this
 * height it will not cut off text.
 * @param[in] label          Label widget handle
 * @return real text height
 **/
int
QBAsyncLabelGetHeight(SvWidget label);

/**
* @brief Callback used to notify that label has finished rendering.
*
* @param[in] ptr            callback data
* @param[in] label          label widget handle
*/
typedef void (*QBAsyncLabelCallback)(void *ptr, SvWidget label);

/**
 * When async label finishes rendering a callback can be called. It is usually
 * used to correctly position this widget and all further ones, as
 * QBAsyncLabelGetWidth() and QBAsyncLabelGetHeight() return real sizes.
 * Before the text is rendered these values are not known, so only after that
 * the application can position it correctly.
 *
 * @param[in] label          Label widget handle
 * @param[in] callback       callback called after text has been rendered
 * @param[in] callbackData   user controlled first argument to callback
 **/
void
QBAsyncLabelSetCallback(SvWidget label, QBAsyncLabelCallback callback, void *callbackData);

/**
 * Async label uses asynchronous text renderer, which also provides a text cache.
 * The user may know that certain texts are so rare, that there is no point in
 * cacheing them. In such case he may call this function. By default all
 * texts are cached.
 *
 * @param[in] label          Label widget handle
 * @param[in] enabled        true iff cache should be used
 **/
void
QBAsyncLabelToggleCache(SvWidget label, bool enabled);

/**
 * Start rendering text inside the label.
 *
 * @param[in] label          label widget handle
 **/
void
QBAsyncLabelStartTextRendering(SvWidget label);

/**
 * Set max lines parameter for given label and re-render the label.
 * Function runs @ref QBAsyncLabelStartTextRendering directly.
 * Remember that multiline label parameter should be set to true.
 *
 * @param[in] label          Label widget handle
 * @param[in] maxLines       maximum number of text lines
 * @param[out] errorOut      error handle
 **/
void
QBAsyncLabelSetMaxLines(SvWidget label, int maxLines, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBASYNCLABEL_H_ */
