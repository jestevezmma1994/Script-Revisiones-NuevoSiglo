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

#ifndef SWL_CONTAINER_H_
#define SWL_CONTAINER_H_

#include <CUIT/Core/types.h>

/**
 * @file SMP/SWL/swl/container.h Container widgets interfaces
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvContainerWidgets Container widgets
 * @ingroup SWL_widgets
 * @{
 *
 * Container widgets align child widgets following the specified alignment rules.
 **/


/**
 * Alignment of child widgets in a container.
 **/
typedef enum {
   /// top alignment for vertical boxes
   SvContainerAlignment_top = 0,
   /// middle alignment for vertical boxes
   SvContainerAlignment_middle,
   /// bottom alignment for vertical boxes
   SvContainerAlignment_bottom,
   /// left alignment for horizontal boxes
   SvContainerAlignment_left,
   /// center alignment for horizontal boxes
   SvContainerAlignment_center,
   /// right alignment for horizontal boxes
   SvContainerAlignment_right,
} SvContainerAlignment;


/**
 * Create vertical box widget using settings from Settings Manager.
 *
 * Vertical box is a container that arranges its children vertically.
 * Its appearance is controlled by following settings:
 *
 *  - bg : bitmap, optional,
 *  - bgColor : color, optional,
 *  - borderSpacing : integer, optional (default: 0),
 *  - verticalPadding : integer, optional (default: 0),
 *  - horizAlign : left, right, center (string, optional, default: center),
 *  - homogenous : boolean, optional (default: false).
 *
 * @param[in] app             CUIT application handle
 * @param[in] widgetName      widget name
 * @param[in] widgets         an array of widgets to arrange
 * @param[in] alignments      horizontal alignment values for widgets in @a widgets array
 * @param[in] count           number of widgets in @a widgets array
 * @return                    newly created widget or @c NULL in case of error
 **/
extern SvWidget
svVerticalBoxNew(SvApplication app, const char *widgetName,
                 SvWidget *widgets, const SvContainerAlignment *alignments, unsigned int count);

/**
 * Create vertical box widget using provided arguments. See svVerticalBoxNew() for details.
 * @param[in] app             CUIT application handle
 * @param[in] widgets         an array of widgets to arrange
 * @param[in] alignments      horizontal alignment values for widgets in @a widgets array
 * @param[in] count           number of widgets in @a widgets array
 * @param[in] minWidth        width of the widget, if 0 then the widget will have automatically
                              chosen width.
 * @param[in] minHeight       height of the widget, if 0 then the widget will have automatically
                              chosen height.
 * @param[in] defaultAlign    if alignments array is not provided, then all widgets will be
                              aligned as defaultAlign says.
 * @return                    newly created widget or @c NULL in case of error
 **/
extern SvWidget
svVerticalBoxSimpleNew(SvApplication app,
                       SvWidget *widgets, const SvContainerAlignment *alignments, unsigned int count,
                       int minWidth, int minHeight, SvContainerAlignment defaultAlign);

/**
 * Create horizontal box widget using settings from Settings Manager.
 *
 * Vertical box is a container that arranges its children horizontally.
 * Its appearance is controlled by following settings:
 *
 *  - bg : bitmap, optional,
 *  - bgColor : color, optional,
 *  - borderSpacing : integer, optional (default: 0),
 *  - horizontalPadding : integer, optional (default: 0),
 *  - vertAlign : top, bottom, middle (string, optional, default: middle),
 *  - homogenous : boolean, optional (default: false).
 *
 * @param[in] app             CUIT application handle
 * @param[in] widgetName      widget name
 * @param[in] widgets         array of widgets to arrange
 * @param[in] alignments      vertical alignment values for widgets in @a widgets array
 * @param[in] count           number of widgets in @a widgets array
 * @return                    newly created widget or @c NULL in case of error
 **/
extern SvWidget
svHorizontalBoxNew(SvApplication app, const char *widgetName,
                   SvWidget *widgets, const SvContainerAlignment *alignments, unsigned int count);

/**
 * Create horizontal box widget using provided arguments. See svHorizontalBoxNew() for details.
 * @param[in] app             CUIT application handle
 * @param[in] widgets         an array of widgets to arrange
 * @param[in] alignments      horizontal alignment values for widgets in @a widgets array
 * @param[in] count           number of widgets in @a widgets array
 * @param[in] minWidth        width of the widget, if 0 then the widget will have automatically
                              chosen width.
 * @param[in] minHeight       height of the widget, if 0 then the widget will have automatically
                              chosen height.
 * @param[in] defaultAlign    if alignments array is not provided, then all widgets will be
                              aligned as defaultAlign says.
 * @return                    newly created widget or @c NULL in case of error
 **/
extern SvWidget
svHorizontalBoxSimpleNew(SvApplication app,
                         SvWidget *widgets, const SvContainerAlignment *alignments, unsigned int count,
                         int minWidth, int minHeight, SvContainerAlignment defaultAlign);


/**
 * @}
 **/

#endif
