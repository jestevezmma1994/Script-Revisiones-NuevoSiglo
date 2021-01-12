/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SWL_VIEWPORT_H_
#define SWL_VIEWPORT_H_

/**
 * @file SMP/SWL/swl/viewport.h Viewport widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>


/**
 * @defgroup SvViewport Viewport widget
 * @ingroup SWL_widgets
 * @{
 *
 * Viewport widget implements scrolled window showing some part of larger widget.
 **/

/**
 * Viewport widget orientation.
 **/
typedef enum {
    /// vertical viewport orientation
    SvViewPortRangeOrientation_VERTICAL = 0,
    /// horizontal viewport orientation
    SvViewPortRangeOrientation_HORIZONTAL
} SvViewPortRangeOrientation;


/**
 * Create viewport widget using settings from the Settings Manager.
 *
 * This method creates viewport widget. Appearance of the widget is
 * controlled by the Settings Manager. It will use following parameters:
 *
 *  - width, height : integer, required,
 *  - xStep, yStep : grid size, integer > 0, optional (default: 1),
 *  - xMaxVelocity, yMaxVelocity : maximum scroll velocity, double > 0, optional (default: 200.0),
 *  - xAcceleration, yAcceleration : scroll acceleration, double > 0, optional (default: 5.0),
 *  - screenAcceleration : full-screen scroll acceleration, double > 0, optional (default: yAcceleration).
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @return                 created widget or @c NULL in case of error
 **/
extern SvWidget
svViewPortNew(SvApplication app, const char *widgetName);

/**
 * Create viewport widget using settings from the Settings Manager and provided width and height.
 *
 * This method creates viewport widget. Appearance of the widget is
 * controlled by the Settings Manager and by width and height passed as arguments for this function.
 * It will use following parameters from settings manager:
 *
 *  - xStep, yStep : grid size, integer > 0, optional (default: 1),
 *  - xMaxVelocity, yMaxVelocity : maximum scroll velocity, double > 0, optional (default: 200.0),
 *  - xAcceleration, yAcceleration : scroll acceleration, double > 0, optional (default: 5.0),
 *  - screenAcceleration : full-screen scroll acceleration, double > 0, optional (default: yAcceleration).
 *
 * @param[in] app          CUIT application handle
 * @param[in] width        desired viewports width
 * @param[in] height       desired viewports height
 * @param[in] widgetName   widget name
 * @return                 created widget or @c NULL in case of error
 **/
SvWidget
svViewPortNewWithSize(SvApplication app, int width, int height, const char *widgetName);

/**
 * Create viewport widget using ratio settings from the Settings Manager.
 *
 * This method creates viewport widget. Appearance of the widget is
 * controlled by the Settings Manager. It will use following parameters:
 *
 *  - widthRatio, heightRatio : double, required,
 *  - xStepRatio, yStepRatio : grid size, double > 0, optional (default: 1 in px),
 *  - xMaxVelocityRatio, yMaxVelocityRatio : maximum scroll velocity, double > 0, optional (default: 200.0 in px),
 *  - xAccelerationRato, yAccelerationRatio : scroll acceleration, double > 0, optional (default: 5.0 in px),
 *  - screenAccRatio : full-screen scroll acceleration, double > 0, optional (default: yAccelerationRatio).
 *
 * @param[in] app          CUIT application handle
 * @param[in] parentWidth  parent width
 * @param[in] parentHeight parent height
 * @param[in] widgetName   widget name
 * @return                 created widget or @c NULL in case of error
 **/
extern SvWidget
svViewPortNewWithRatio(SvApplication app,
                       unsigned int parentWidth,
                       unsigned int parentHeight,
                       const char *widgetName);

/**
 * Set the view port content.
 *
 * This method sets the viewport contents. The old content, if any, is replaced with
 * the new one. If there are any pending scroll effects, they are cancelled.
 *
 * @param[in] viewport        viewport widget handle
 * @param[in] contents        new viewport's contents
 **/
extern void
svViewPortSetContents(SvWidget viewport, SvWidget contents);

/**
 * Get the view port content.
 *
 * @param[in] viewport        viewport widget handle
 * @return                    viewport's contents
 **/
extern SvWidget
svViewPortGetContents(SvWidget viewport);

/**
 * Add new range listener for the view port.
 *
 * @param[in] viewport       viewport widget handle
 * @param[in] listener       instance of the SvRangeListener implementation
 * @param[in] orientation    range oriantation
 * @return                   @c 0 in case of an error, otherwise any other value
 */
extern int
svViewPortAddRangeListener(SvWidget viewport,
                           SvObject listener,
                           SvViewPortRangeOrientation orientation);

/**
 * Remove the specific listener from the viewport.
 *
 * @param[in] viewport      viewport widget handle
 * @param[in] listener      instance of the SvRangeListener to be removed
 * @param[in] orientation   range orientation
 * @return                  @c 0 in case of an error, otherwise any other value
 */
extern int
svViewPortRemoveRangeListener(SvWidget viewport,
                              SvObject listener,
                              SvViewPortRangeOrientation orientation);

/**
 * Inform viewport that size of the contents changed.
 *
 * This method inform viewport that size of the contents changed, so that view port can
 * invoke range listeners functions (viewport does not keep track of the
 * contents size by itself). Call to this function is @em OBLIGATORY and should be
 * done immediatelly after the content's size change. Otherwise viewport might
 * get corrupted.
 *
 * @param[in] viewport      viewport widget handle
 * @return                  @c 0 in case of an error, otherwise any other value
 **/
extern void
svViewPortContentsSizeChanged(SvWidget viewport);

/**
 * Set the contents widget coordinates.
 *
 * This method sets the contents widget coordinates that appear in the upper left
 * corner of the viewport. The coordinates should not be negative and
 * should not be larger than "contentsSize - viewPortSize" as well. If the
 * contents widget is smaller than the viewport in one of the dimensions,
 * the corresponding coordinate argument will be ignored.
 *
 * @param[in] viewport      viewport widget handle
 * @param[in] x             X coordinate
 * @param[in] y             Y coordinate
 * @param[in] immediate     @c true to change the coordinates immediately,
 *                          @c false to change with effect
 **/
extern void
svViewPortSetContentsPosition(SvWidget viewport,
                              int x, int y, bool immediate);

/**
 * Return the contents widget coordinates.
 *
 * This method returns the contents widget coordinates that appear in the upper left
 * corner of the viewport. If there is no contents widget, it returns (0, 0).
 *
 * @param[in] viewport      viewport widget handle
 * @param[out] x            X coordinate (pass @c NULL if you don't care)
 * @param[out] y            Y coordinate (pass @c NULL if you don't care)
 **/
extern void
svViewPortGetContentsPosition(SvWidget viewport,
                              int *x, int *y);

/**
 * Start scrolling content vertically.
 *
 * @param[in] viewPort      viewport widget handle
 * @param[in] downDirection @c true to start scrolling downwards
 **/
extern void
svViewPortStartScrollingVertically(SvWidget viewPort, bool downDirection);

/**
 * Stop scrolling content vertically.
 *
 * @param[in] viewPort      viewport widget handle
 **/
extern void
svViewPortStopScrollingVertically(SvWidget viewPort);

/**
 * Check if viewport can be scrolled vertically.
 *
 * @param[in] viewPort      viewport widget handle
 * @param[in] downDirection @c true to check if viewport can be scrolled downwards
 * @return                  @c true if viewport can be scrolled
 **/
extern bool
svViewPortCanBeScrolledVertically(SvWidget viewPort, bool downDirection);

/**
 * Use viewport to handle input event.
 *
 * @param[in] viewPort      viewport widget handle
 * @param[in] e             event to be handled
 *
 * @return                  flag indicating if the event was handled
 **/
bool svViewPortHandleInputEvent(SvWidget viewPort, SvInputEvent e);

/**
 * @}
 **/


#endif
