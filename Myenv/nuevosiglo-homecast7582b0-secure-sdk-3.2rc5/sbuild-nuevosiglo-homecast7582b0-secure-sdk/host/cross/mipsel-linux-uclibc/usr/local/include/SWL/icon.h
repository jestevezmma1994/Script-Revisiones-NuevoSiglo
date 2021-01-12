/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SWL_ICON_H_
#define SWL_ICON_H_

/**
 * @file SMP/SWL/swl/icon.h Icon widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvString.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>
#include <QBResourceManager/SvRBObject.h>


/**
 * @defgroup SvIcon Icon widget
 * @ingroup SWL_widgets
 * @{
 *
 * Icon widget displays one image from some specified set of images.
 **/

/**
 * Type of UI event sent by icon widget when currently shown bitmap is changed.
 * @class SvIconSwitchedEvent
 * @extends QBUIEvent
 **/
typedef struct SvIconSwitchedEvent_ *SvIconSwitchedEvent;

/**
 * Get runtime type identification object representing icon switched event class.
 *
 * @relates SvIconSwitchedEvent
 *
 * @return icon switched event class
 **/
extern SvType
SvIconSwitchedEvent_getType(void);

/**
 * SvIconDescription is a class that caches svIcon parameters from settings file.
 * Used to reduce calls to settings manager.
 *
 * @class SvIconDescription
 * @extends SvObject
 **/
typedef struct SvIconDescription_ *SvIconDescription;

/**
 * @relates SvIconDescription
 *
 * Get runtime type identification object representing SvIconDescription class.
 *
 * @return SvIconDescription type identification object
 */
extern SvType
SvIconDescription_getType(void);

/**
 * Create frame description object with parameters from the Settings Manager.
 *
 * @see See svIconNew() for the list of parameters.
 *
 * @memberof SvIconDescription
 *
 * @param[in] widgetName    widget name
 * @return                  created description, @c NULL in case of error
 **/
extern SvIconDescription
SvIconDescriptionCreateFromSettings(const char *widgetName);

/**
 * Icon widget class.
 *
 * @class SvIcon
 * @extends QBSWLWidget
 **/
typedef struct SvIcon_ *SvIcon;

/**
 * Create icon widget containing a number of bitmaps.
 *
 * This function creates icon widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 * - width, height - integer, required,
 * - vertAlign - string { top, middle, bottom }, optional (default: middle),
 * - horizAlign - string { left, center, right }, optional (default: center),
 * - effectDuration - double, optional (default: 0.0),
 * - keepAspectRatio - bool, optional (default: true),
 * - upScale - bool, optional (default: false),
 * - bg - bitmap, optional,
 * - bitmaps - array of bitmaps, optional (overrides 'bg').
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    name of the widget used with Settings Manager
 * @return                  newly created widget or @c NULL in case of error
 */
extern SvWidget
svIconNew(SvApplication app, const char *widgetName);

/**
 * Create frame widget using parameters from description object.
 *
 * @param[in] app           CUIT application handle
 * @param[in] description   svIcon description handle
 * @return                  newly created widget or @c NULL in case of error
 */
extern SvWidget
svIconNewFromDescription(SvApplication app, SvIconDescription description);

/**
 * Create icon widget containing a number of bitmaps with Ratio settings.
 *
 * This function creates icon widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 * - widthRatio, heightRatio - double, required,
 * - vertAlign - string { top, middle, bottom }, optional (default: middle),
 * - horizAlign - string { left, center, right }, optional (default: center),
 * - effectDuration - double, optional (default: 0.0),
 * - keepAspectRatio - bool, optional (default: true),
 * - upScale - bool, optional (default: false),
 * - bg - bitmap, optional,
 * - bitmaps - array of bitmaps, optional (overrides 'bg').
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    name of the widget used with Settings Manager
 * @param[in] parentWidth   width of a parent widget in pixels
 * @param[in] parentHeight  height of a parent widget in pixels
 * @return                  newly created widget or @c NULL in case of error
 */
extern SvWidget
svIconNewFromRatio(SvApplication app, const char *widgetName, int parentWidth, int parentHeight);

/**
 * Create icon widget with explicitly specified dimensions.
 *
 * This function creates new icon widget without reading any parameters
 * from the Settings Manager: dimensions are specified explicitly, for all
 * other parameters default values are used.
 *
 * @param[in] app           CUIT application handle
 * @param[in] width         widget width in pixels
 * @param[in] height        widget height in pixels
 * @param[in] upScale       should icon be upscaled
 * @return                  newly created widget or @c NULL in case of error
 */
extern SvWidget
svIconNewWithSize(SvApplication app, unsigned int width, unsigned int height, bool upScale);

/**
 * Add bitmap to the icon widget.
 *
 * @param[in] icon          icon widget handle
 * @param[in] index         position on the list of bitmaps
 * @param[in] bitmap        bitmap handle
 * @return                  @c 0 on success, @c -1 on error
 **/
extern int
svIconSetBitmap(SvWidget icon, unsigned int index, SvBitmap bitmap);

/**
 * Add bitmap managed by Resource Bundle to the icon widget.
 *
 * @param[in] icon          icon widget handle
 * @param[in] index         position on the list of bitmaps
 * @param[in] resourceID    resource ID of the bitmap to be added
 * @return                  @c 0 on success, @c -1 on error
 **/
extern int
svIconSetBitmapFromRID(SvWidget icon, unsigned int index, SvRID resourceID);


/**
 * Add bitmap identified by an URI to the icon widget with given
 * resource bundle policy.
 *
 * @param[in] icon          icon widget handle
 * @param[in] index         position on the list of bitmaps
 * @param[in] URI           URI of a bitmap to be added
 * @param[in] policy        resource bundle policy
 * @return                  resource ID on success, @c SV_RID_INVALID on error
 **/
extern SvRID
svIconSetBitmapFromURIWithRBPolicy(SvWidget icon, unsigned int index, const char *URI, SvRBPolicy policy);

/**
 * Add bitmap identified by an URI to the icon widget.
 *
 * @param[in] icon          icon widget handle
 * @param[in] index         position on the list of bitmaps
 * @param[in] URI           URI of a bitmap to be added
 * @return                  resource ID on success, @c SV_RID_INVALID on error
 **/
extern SvRID
svIconSetBitmapFromURI(SvWidget icon, unsigned int index, const char *URI);

/**
 * Assign textual tag to one of the bitmaps managed by icon widget.
 *
 * @param[in] icon          icon widget handle
 * @param[in] index         position on the list of bitmaps
 * @param[in] tag           textual tag for bitmap at @a index
 * @return                  @c 0 on success, @c -1 on error
 **/
extern int
SvIconSetBitmapTag(SvWidget icon, unsigned int index, SvString tag);

/**
 * Get index of currently displayed bitmap.
 *
 * @param[in] icon          icon widget handle
 * @return                  index of currently displayed bitmap,
 *                          @c -1 in case of error
 **/
extern int
svIconGetIndex(SvWidget icon);

/**
 * Get currently displayed bitmap.
 *
 * @param[in] self          icon widget handle
 * @param[out] width        width (in pixel) in which current bitmap is shown,
 *                          pass @c NULL if not relevant
 * @param[out] height       height (in pixel) in which current bitmap is shown,
 *                          pass @c NULL if not relevant
 * @return                  currently displayed bitmap, @c NULL if not available
 **/
extern SvBitmap
SvIconGetCurrentBitmap(SvIcon self, unsigned int *width, unsigned int *height);

/// @copydoc SvIconGetCurrentBitmap()
extern SvBitmap
svIconGetCurrentBitmap(SvWidget self, unsigned int *width, unsigned int *height);

/**
 * Get number of bitmaps managed by icon widget.
 *
 * @param[in] icon          icon widget handle
 * @return                  number of bitmaps, @c -1 in case of error
 **/
extern int
svIconGetBitmapsCount(SvWidget icon);

/**
 * Switch bitmap displayed by icon widget.
 *
 * @param[in] icon          icon widget handle
 * @param[in] target        index of a bitmap to be used
 * @param[in] fallback      index of a fallback bitmap, to be used if target
 *                          bitmap is unavailable
 * @param[in] duration      duration of the switch effect,
 *                          pass @c -1.0 to get default value from settings
 * @return                  @c 0 on success, @c -1 on error
 **/
extern int
svIconSwitch(SvWidget icon, unsigned int target, unsigned int fallback, double duration);

/**
 * Switch bitmap displayed by icon widget using bitmap's tag.
 *
 * @param[in] icon          icon widget handle
 * @param[in] target        tag of a bitmap to be used
 * @param[in] fallback      index of a fallback bitmap, to be used if target
 *                          bitmap is unavailable
 * @param[in] duration      duration of the switch effect,
 *                          pass @c -1.0 to get default value from settings
 * @return                  @c 0 on success, @c -1 on error
 **/
extern int
svIconSwitchByTag(SvWidget icon, SvString target, unsigned int fallback, double duration);

/**
 * Change icon's dimensions.
 *
 * @param[in] self      icon widget handle
 * @param[in] width     new icon's width, @c -1 to leave current width
 * @param[in] height    new icon's height, @c -1 to leave current height
 * @return              @c 0 on success, @c -1 on error
 */
extern int
SvIconSetSize(SvIcon self, int width, int height);

/// @copydoc SvIconSetSize()
extern int
svIconSetSize(SvWidget self, int width, int height);

/**
 * Check if target bitmap can be shown immediately.
 *
 * @param[in] icon      icon widget handle
 * @param[in] target    index of queried bitmap
 * @return              @c true if target bitmap can be shown immediately
 **/
extern bool
svIconIsBitmapAvailable(SvWidget icon, unsigned int target);

/**
 * Checks if bitmap is still being loaded from resources.
 *
 * @param[in] icon      icon widget handle
 * @param[in] target    index of queried bitmap
 * @return              @c true if bitmap is still being loaded
 **/
extern bool
SvIconIsBitmapLoading(SvWidget icon, unsigned int target);

/**
 * Set icon state.
 *
 * This method sets active/inactive state of the icon widget.
 * When icon is in inactive state, it will agree to release currently shown
 * bitmap when asked by the @ref SvRBService (supposedly when system is low
 * on graphics memory.) When state is switched back to active, icon widget
 * will ask the @ref SvRBService to load current bitmap again.
 *
 * @param[in] icon      icon widget handle
 * @param[in] isInactive @c true to set state to inactive, @c false to set to active
 * @return              @c 0 on success, @c -1 on error
 **/
extern int
svIconSetState(SvWidget icon, bool isInactive);

/**
 * @}
 **/


#endif
