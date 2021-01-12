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

#ifndef QB_GRID_H_
#define QB_GRID_H_

/**
 * @file QBGrid.h Grid widget API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>
#include <QBWidgets/QBAsyncLabel.h>

/**
 * @defgroup QBGrid QBGrid: Stores widgets as a vertical or horizontal grid
 * @ingroup CubiTV_widgets
 * @{
 *
 * Creates widget that allows to store other widgets and show them as vertical or horizontal grid.
 **/

typedef void (*QBGridResizeCallback)(SvWidget grid, void *target);

/**
 * Create grid widget using settings from the Settings Manager.
 *
 * This method creates grid widget. Appearance of this widget is
 * controlled by the Settings Manager. It will use following parameters:
 *
 *  - width, height : integer, optional when @c fitSize is true, required anotherways,
 *  - spacing : integer, optional (default 0),
 *  - orientation: string, optional
 *    possible values:
 *    - "vertical" - vertical grid (default),
 *    - "horizontal" - horizontal grid
 *  - horizontalAlignment : string, optional (default left):
 *    - "left"
 *    - "right"
 *    - "none" - xOffsets of attached widgets are left as read from settings
 *  - verticalAlignment : optional (default top),
 *    - "top"
 *    - "bottom"
 *    - "none" - yOffsets of attached widgets are left as read from settings
 *  - fitSize : bool, optional (default false)
 *
 * @param[in] app             CUIT application handle
 * @param[in] name            widget name
 * @return                    newly created widget or NULL in case of error
 **/
SvWidget QBGridCreate(SvApplication app, const char *name);

/**
 * Create grid widget using ratio settings from the Settings Manager.
 *
 * This method creates grid widget. Appearance of this widget is
 * controlled by the Settings Manager. It will use following parameters:
 *
 *  - widthRatio, heightRatio : double, required,
 *  - spacingRatio : double, optional (default 0.0),
 *  - orientation: string, optional (default "vertical")
 *
 * @param[in] app             CUIT application handle
 * @param[in] parentWidth     parent width
 * @param[in] parentHeight    parent height
 * @param[in] name            widget name
 * @return                    newly created widget or NULL in case of error
 **/
SvWidget QBGridCreateWithRatio(SvApplication app,
                               unsigned int parentWidth,
                               unsigned int parentHeight,
                               const char *name);

/**
 * Removes and destroys all the widgets inside the grid.
 *
 * @param[in] grid            QBGrid widget handle
 **/
void QBGridReset(SvWidget grid);

/**
 * Rearranges all the widgets stored in grid.
 *
 * @param[in] grid            QBGrid widget handle
 *
 * @return                    @c true if the grid was rearranged, @c false otherwise
 **/
bool QBGridRearrange(SvWidget grid);

/**
 * Removes widget from the grid.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] w               widget to be removed
 **/
void QBGridRemoveWidget(SvWidget grid, SvWidget w);

/**
 * Shows previously hidden widget.
 *
 * Widget has to be already added to @c QBGrid to be shown.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] w               widget to show
 **/
void QBGridShowWidget(SvWidget grid, SvWidget w);

/**
 * Hides all widgets.
 *
 * Hidden widgets are not shown for user but they're still attached to @c QBGrid instance and will be destroyed by it.
 *
 * @param[in] grid            QBGrid widget handle
 **/
void QBGridHideAllWidgets(SvWidget grid);

/**
 * Add any widget to grid.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] w               widget to be added handle
 **/
void QBGridAddWidget(SvWidget grid, SvWidget w);

/**
 * Add any widget to grid with additional spacing.
 * Additional space will be added below the vertical widget and right to
 * the horizontal one.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] w               widget to be added handle
 * @param[in] spacing         additional spacing
 **/
void QBGridAddWidgetWithSpacing(SvWidget grid, SvWidget w, int spacing);

/**
 * Add async label widget to grid.
 * The label will be constructed from settings inside this function.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] widgetName      name of the widget
 * @param[in] renderer        text renderer handle
 *
 * @return                    label constructed inside the function, @c NULL in case of error
 **/
SvWidget QBGridAddAsyncLabel(SvWidget grid, const char *widgetName, QBTextRenderer renderer);

/**
 * Add subgrid widget to grid.
 * The grid will be constructed from settings inside this function. Grid will handle resize event of subgrid.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] name            name of the widget
 *
 * @return                    grid constructed inside the function, @c NULL in case of error
 **/
SvWidget QBGridAddSubgrid(SvWidget grid, const char *name);

/**
 * Add async label widget to grid with additional spacing.
 * The label will be constructed from settings inside this function.
 * Additional space will be added below the vertical widget and right to
 * the horizontal one.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] widgetName      name of the widget
 * @param[in] renderer        text renderer handle
 * @param[in] spacing         additional spacing
 *
 * @return                    label constructed inside the function, @c NULL in case of error
 **/
SvWidget QBGridAddAsyncLabelWithSpacing(SvWidget grid, const char *widgetName, QBTextRenderer renderer, int spacing);

/**
 * Add async label widget to grid.
 * The widget will be constructed from constructor inside this function.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] constructor     async label constructor
 *
 * @return                    label constructed inside the function, @c NULL in case of error
 **/
SvWidget QBGridAddAsyncLabelFromConstructor(SvWidget grid, QBAsyncLabelConstructor constructor);

/**
 * Add async label widget to grid.
 * The widget will be constructed from constructor inside this function.
 * Additional space will be added below the vertical widget and right to
 * the horizontal one.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] constructor     async label constructor
 * @param[in] spacing         additional spacing
 *
 * @return                    label constructed inside the function, @c NULL in case of error
 **/
SvWidget QBGridAddAsyncLabelFromConstructorWithSpacing(SvWidget grid, QBAsyncLabelConstructor constructor, int spacing);

/**
 * Add icon widget to grid.
 * The widget will be constructed from settings inside this function.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] widgetName      name of the widget
 *
 * @return                    icon constructed inside the function, @c NULL in case of error
 **/
SvWidget QBGridAddIcon(SvWidget grid, const char *widgetName);

/**
 * Add icon widget to grid.
 * The widget will be constructed from settings inside this function.
 * Additional space will be added below the widget.
 *
 * @param[in] grid            QBGrid widget handle
 * @param[in] widgetName      name of the widget
 * @param[in] spacing         additional spacing
 *
 * @return                    icon constructed inside the function, @c NULL in case of error
 **/
SvWidget QBGridAddIconWithSpacing(SvWidget grid, const char *widgetName, int spacing);

/**
 * Sets callback function which will be called on resizing the grid
 *
 * @param[in] grid          QBGrid widget handle
 * @param[in] callback      callback handle
 * @param[in] target        pointer passed to callback as target argument
 *
 * @return                  icon constructed inside the function, @c NULL in case of error
 **/
void QBGridSetResizeCallback(SvWidget grid, QBGridResizeCallback callback, void *target);

/**
 * @}
 **/

#endif /* QB_GRID_H_ */
