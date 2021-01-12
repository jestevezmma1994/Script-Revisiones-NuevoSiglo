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

#ifndef QB_WIDGET_INTERNAL_H_
#define QB_WIDGET_INTERNAL_H_

/**
 * @file QBWidgetInternal.h QBWidget internal API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/QBWidget.h>

#include <CUIT/Core/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @addtogroup QBWidget
 * @{
 **/

/**
 * QBWidget class internals.
 **/
struct QBWidget_ {
    struct SvObject_ super_;    ///< super class
};

/**
 * Virtual methods of widget class.
 **/
typedef const struct QBWidgetVTable_ {
    /// virtual methods of the base class
    struct SvObjectVTable_ super_;

    /**
     * Attach child widget.
     *
     * @param[in] self          parent widget handle
     * @param[in] child         child widget
     * @param[in] xOffset       x coordinate of top-left corner of widget (parent relative)
     * @param[in] yOffset       y coordinate of top-left corner of widget (parent relative)
     * @param[in] level         Z-order of the widget (higher number = closer to viewer)
     **/
    void (*attachChild)(QBWidget self, const struct QBWidget_ *child,
                        int xOffset, int yOffset, unsigned short int level);

    /**
     * Detach widget from parent.
     *
     * @param[in] self          widget handle
     **/
    void (*detach)(QBWidget self);

    /**
     * Get widget width.
     *
     * @param[in] self          widget handle
     * @return                  widget width
     */
    unsigned int (*getWidth)(QBWidget self);

    /**
     * Get widget height.
     *
     * @param[in] self          widget handle
     * @return                  widget height
     */
    unsigned int (*getHeight)(QBWidget self);

    /**
     * Set widget width.
     *
     * @param[in] self          widget handle
     * @param[in] width         widget width
     */
    void (*setWidth)(QBWidget self, unsigned int width);

    /**
     * Set widget height.
     *
     * @param[in] self          widget handle
     * @param[in] height        widget height
     */
    void (*setHeight)(QBWidget self, unsigned int height);

    /**
     * Get widget x offset.
     *
     * @param[in] self          widget handle
     * @return                  widget x offset
     */
    int (*getXOffset)(QBWidget self);

    /**
     * Get widget y offset.
     *
     * @param[in] self          widget handle
     * @return                  widget y offset
     */
    int (*getYOffset)(QBWidget self);

    /**
     * Set widget x offset.
     *
     * @param[in] self          widget handle
     * @param[in] xOffset       widget x offset
     */
    void (*setXOffset)(QBWidget self, int xOffset);

    /**
     * Set widget y offset.
     *
     * @param[in] self          widget handle
     * @param[in] yOffset       widget y offset
     */
    void (*setYOffset)(QBWidget self, int yOffset);

    /**
     * Detaches all children widgets.
     *
     * @param[in] self          widget handle
     */
    void (*detachChildren)(QBWidget self);
} *QBWidgetVTable;

/**
 * Get runtime type identification object representing QBWidget class.
 *
 * @relates QBWidget
 *
 * @return QBWidget runtime type identification object
 **/
extern SvType
QBWidget_getType(void);

/**
 * Initialize widget with given width and height
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @param[in] width         widget width
 * @param[in] height        widget height
 * @param[out] *errorOut    error info
 * @return                  self on success, @c NULL on error
 */
QBWidget
QBWidgetInit(QBWidget self,
             unsigned int width,
             unsigned int height,
             SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_WIDGET_INTERNAL_H_ */
