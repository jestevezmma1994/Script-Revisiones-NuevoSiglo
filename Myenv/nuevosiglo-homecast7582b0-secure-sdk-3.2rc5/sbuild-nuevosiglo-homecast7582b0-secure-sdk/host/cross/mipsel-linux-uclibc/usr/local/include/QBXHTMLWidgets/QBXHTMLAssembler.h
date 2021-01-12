/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_XHTML_ASSEMBLER_H
#define QB_XHTML_ASSEMBLER_H

/**
 * @file QBXHTMLAssembler.h XHTML Widgets assembler class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDataModel3/QBActiveTree.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBXHTMLAssembler XHTML Widgets assembler
 * @ingroup GUI
 * @{
 *
 * A class assembling widget represented by XHTML
 **/

/**
 * QBXHTMLAssembler widgets assembler class.
 * This class is responsible for XHTML widgets assembly.
 *
 * @class QBXHTMLAssembler
 * @extends SvObject
 **/
typedef struct QBXHTMLAssembler_ *QBXHTMLAssembler;

/**
 * Create Assembler object.
 * The object created with this method will be able to manufacture widgets represented by constructor parameters.
 *
 * @param[in] tree                  tree representing XHTML document (created with QBXHTMLTreeBuilder)
 * @param[in] widgetFactoryVisitor  factory object for creating basic widgets and containers.
 *                                  This object must implement @c QBXHTMLWidgetFactory and @c QBNodeVisitor
 * @param[out] errorOut             error output
 * @return                          Newly created Assembler object or @c NULL in case of error.
 **/
extern QBXHTMLAssembler
QBXHTMLAssemblerCreate(QBActiveTree tree,
                       SvObject widgetFactoryVisitor,
                       SvErrorInfo *errorOut);

/**
 * Assemble widget of given width and height represented by XML provided in constructor using factory provided in costructor.
 * WARNING! Ownership of the widget is passed to the caller.
 * This is a multi step function. Its algorithm is as follows:
 * 1. Use the widgetFactoryVisitor to create all the basic nodes widgets (all the inputs, checkboxes images etc)
 * 2. Collect all the sizes of basic widgets
 * 3. Layout the widgets using its sizes, XML document (represented by tree), and max width and height given in parameters
 * 4. create all the intermediate containers for widgets using widget factory
 * 5. put all widgets and containers together according to layout data found in step 3
 * 6. remove any unused widgets (not embeded into the body container in step 5. This may happen for some malformed XML)
 * 7. return the container representing XHTML body element
 *
 * WARNING! This function is fully synchronous, so widgetFactoryVisitor cannot delay creation of widgets.
 * This means that if you have to perform some async functionality to create widgets or containers do it before calling this function.
 * If for example you have to download an image to construct a widget with a background do it prior to this function, and pass this
 * downloaded image to your widgetFactoryVisitor so that it can create widget synchronously on demand.
 *
 * To get more details on widget creatoed with this method call @c QBXHTMLAssemblerGetWidgetsInfo.
 *
 * @param[in] self                  Assembler handle
 * @param[in] bodyWidth             width of a widget to create
 * @param[in] bodyHeight            height of a widget to create
 *                                  This object must implement @c QBXHTMLWidgetData and @c QBNodeVisitor
 * @param[out] errorOut             error output
 * @return                          created widget or @c NULL if no widget was created
 **/
extern void *
QBXHTMLAssemblerAssemble(QBXHTMLAssembler self,
                         int bodyWidth,
                         int bodyHeight,
                         SvErrorInfo *errorOut);

/**
 * Get the widgets info hashtable constructed using @c QBXHTMLAssemblerAssemble function.
 * The hashtable contains @c QBXHTMLWidgetInfo objects with @c QBXHTMLVisitableNode nodes as keys.
 * To make use of this data you should create a visitor that will visit the document tree in some way
 * and obtain @c QBXHTMLWidgetInfo from this table using visited node as a key.
 *
 * @param[in] self                  Assembler handle
 * @return                          Info hashtable describing all widgets created with previous call to
 *                                  @c QBXHTMLAssemblerAssemble or @c NULL if no widget was created
 **/
extern SvHashTable
QBXHTMLAssemblerGetWidgetsInfo(QBXHTMLAssembler self);

/**
 * @}
 **/

#endif //QB_XHTML_ASSEMBLER_H
