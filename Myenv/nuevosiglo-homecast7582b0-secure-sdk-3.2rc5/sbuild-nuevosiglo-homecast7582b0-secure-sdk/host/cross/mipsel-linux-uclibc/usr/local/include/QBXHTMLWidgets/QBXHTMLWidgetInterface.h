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

#ifndef QB_XHTML_WIDGET_INTARFACE_H
#define QB_XHTML_WIDGET_INTARFACE_H

/**
 * @file QBXHTMLWidgetInterface.h XHTML widgets interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBXHTMLWidgets/QBXHTMLNodes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBXHTMLWidgetInterface XHTML Widgets interface
 * @ingroup GUI
 * @{
 *
 * This is an interface that needs to be implemented in order to provide a visual representation of XHTML widgets.
 * The example implementation is presented in @c QBXHTMLTxtWidget.h and @c QBXHTMLTxtWidget.c.
 * Basically the visual representation provider must implement two things:
 * 1. General container widget that can hold children widgets at given positions
 * 2. Factory class that will be able to manufacture basic widgets and tell their sizes
 *
 * When implementing this interface you should keep in mind that:
 * 1. A container class will have to be able to contain any widget that will be manufactured by Factory class
 * 2. A container will be given children widgets by @c void * so all the widgets must have a common superclass,
 *    so that no class distinction is needed when adding children to container [*]
 * 3. A container can be passed as a child widget to another container so it must share a common superclass with widgets
 *    manufactured by factory [*]
 * 4. Containers and widgets can all be of the same class (just like @c QBXHTMLTxtWidget)
 * 5. Containers and widgets have to provide a mean to manage their lifetime.
 *    The library can handle refcounted objects and objects destroyed with a destructor function.
 *    It is up to implementer to choose which method is best for his widgets implementation
 * 6. When widget is inserted into the container, the container becomes its owner and is responsible for its deletion upon its own deletion.
 *
 * [*] As a matter of fact you will be passed the nodes representing both widget and a container that you mount inside one another,
 *     so technically speaking you can distinguish their types by the types of nodes and their attributes, but that would be not
 *     elegant to say the least
 *
 **/


/**
 * Function that will destroy widget given to it as a parameter
 *
 * @param[in] widget        widget to destroy
 **/
typedef void (*XHTMLWidgetDestroyFunction)(void *widget);

/**
 * Function that will decrease reference count of a widget
 *
 * @param[in] widget        widget to deref
 **/
typedef void (*XHTMLWidgetDerefFunction)(void *widget);


/**
 * QBXHTMLWidgetData class.
 * The result of widget factory visitor is a hash table of objects of this type.
 * This kind of object should be created for every widget created by factory.
 * This object holds widget pointer and some basic data about the widget (width, height etc)
 *
 * @class QBXHTMLWidgetData
 * @extends SvObject
 **/
typedef struct QBXHTMLWidgetData_ *QBXHTMLWidgetData;

/**
 * Create new QBXHTMLWidgetData object.
 * This method should be called just after a factory visitor creates a widget for a node.
 * The result of this method should be inserted into the results hash table of factory visitor
 * witch the node as the key.
 *
 * @param[in] widget            the widget pointer
 * @param[in] width             widget width
 * @param[in] height            widget height
 * @param[in] destroyFunction   the pointer to the function that will be called when widget needs to be destroyed.
 *                              Set this pointer or @c derefFunctuion. NEVER set both.
 * @param[in] derefFunctuion    the pointer to the function that will decrease the refcount of a widget.
 *                              Set this pointer or @c destroyFunction. NEVER set both.
 * @param [out] errorOut        error output
 * @return                      QBXHTMLWidgetData object describing a widget, or NULL in case of error.
 **/
extern QBXHTMLWidgetData
QBXHTMLWidgetDataCreate(void *widget,
                        int width,
                        int height,
                        XHTMLWidgetDestroyFunction destroyFunction,
                        XHTMLWidgetDerefFunction derefFunctuion,
                        SvErrorInfo *errorOut);

/**
 * QBXHTMLWidgetFactory interface.
 * This interface is responsible for manufacturing widgets, and container widgets.
 * WARNING! The object implementing this interface (called "widget factory visitor" or "factory visitor" or "widget factory")
 * should also implement @c QBNodeVisitor.
 * The way the library handles widget creation is that a factory visitor will visit the document tree in postorder.
 * It is suposed to create basic widgets for leaf (or near leaf) nodes like text, input, button, img etc.
 * Each of the widgets is suposed to be wrapped with a @c QBXHTMLWidgetData and inserted into a hashtable with a
 * document node beeing a key. This hashtable is later to be returned by the getResult method.
 *
 * Important thing to remember is that a tree is visited in postorder so when a factory visitor will visit a node it can assume that all its
 * children have allready been visited (for example all options for a select node were visited and are known at this point).
 *
 * Another important thing is that when wrapping a widget with @c QBXHTMLWidgetData the size of a widget need to be allready known
 * (width and height parameters of @c QBXHTMLWidgetDataCreate).
 *
 * @interface QBXHTMLWidgetFactory
 **/
typedef const struct QBXHTMLWidgetFactory_ {
    /**
     * Get the result of factory visitation.
     *
     * @param [in] self     the interface implementer reference
     * @return              the hash table of with document tree nodes as keys an @c QBXHTMLWidgetData as values.
     *                      The values in the hash table describe widgets created by factory visitor. There should be
     *                      an entry for every widget created by the factory.
     **/
    SvHashTable (*getResult)(SvObject self);

    /**
     * Manufacture new container.
     * This is a method called when a new container is needed. The creator can decide
     * whether the container is refcounted (fill the derefOut param), or destroyed
     * with a function (fill destroyOut).
     * After the call to this function the library takes ownership
     * of newly created container (or shared ownership in case of refcounting)
     * untill it is added to other containe or no longer needed (it will get destroyed or dereferenced then).
     *
     * @param [in] self         the interface implementer reference
     * @param [in] width        the width of new container
     * @param [in] height       the height of new container
     * @param [in] node         the node of XML document for which this container is created.
     *                          If needed the container can i.e. read background color from XML attribute of this node
     * @param [out] destroyOut  the pointer to the function that will be called when container needs to be destroyed.
     *                          Set this pointer or @c derefOut. NEVER set both.
     * @param [out] derefOut    the pointer to the function that will decrease the refcount of a container
     *                          (and destroy if refcount reaches 0). Set this pointer or @c destroyOut. NEVER set both.
     * @return                  Newly created container.
     **/
    void * (*manufactureContainer)(SvObject self,
                                   int width,
                                   int height,
                                   QBXHTMLVisitableNode node,
                                   XHTMLWidgetDestroyFunction * destroyOut,
                                   XHTMLWidgetDerefFunction * derefOut);

    /**
     * Add a child widget to a given container at a given position
     *
     * @param [in] self         the interface implementer reference
     * @param [in] parent       the parent container (always an object created with @c manufactureContainer)
     * @param [in] child        the child widget (object created with @c QBXHTMLWidgetData or container created with @c manufactureContainer)
     * @param [in] x            the x coordinate of upper left corner of child widget relative to container upper left corner
     * @param [in] y            the y coordinate of upper left corner of child widget relative to container upper left corner
     * @param [in] parentNode   the node of XML document representing prent container (this one should be more or less useless for this functuion)
     * @param [in] childNode    the node of XML document representing child widget (this one should be more or less useless for this functuion)
     **/
    void (*addChild)(SvObject self,
                     void *parent,
                     void *child,
                     int x,
                     int y,
                     QBXHTMLVisitableNode parentNode,
                     QBXHTMLVisitableNode childNode);
} *QBXHTMLWidgetFactory;

/**
 * Get the object representing QBXHTMLWidgetFactory type.
 *
 * @return      QBXHTMLWidgetFactory type meta object
 **/
extern SvInterface
QBXHTMLWidgetFactory_getInterface(void);

/**
 * @}
 **/


#endif //QB_XHTML_WIDGET_INTARFACE_H
