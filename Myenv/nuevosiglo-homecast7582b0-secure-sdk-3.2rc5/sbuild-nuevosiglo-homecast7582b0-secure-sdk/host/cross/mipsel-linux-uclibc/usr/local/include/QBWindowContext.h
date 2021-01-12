/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_WINDOW_CONTEXT_H_
#define QB_WINDOW_CONTEXT_H_

/**
 * @file QBWindowContext.h QBWindowContext class API
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <CUIT/Core/types.h>
#include <QBInput/QBInputEvent.h>
#include <QBApplicationControllerTypes.h>

/**
 * @defgroup QBWindowContext Window context class
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * An abstract base class for top-level window context implementations.
 **/

/**
 * Abstract window context class.
 * @class QBWindowContext
 * @extends SvObject
 **/
typedef struct QBWindowContext_t {
    struct SvObject_ super_;
    SvWindow window;
} *QBWindowContext;

/**
 * Virtual methods of the window context class.
 **/
typedef const struct QBWindowContextVTable_ {
    /// virtual methods of the base class
    struct SvObjectVTable_ super_;

    /**
     * Create window for window context.
     *
     * @param[in] self_     window context handle
     * @param[in] app       CUIT application handle
     **/
    void (*createWindow)(QBWindowContext self_, SvApplication app);

    /**
     * Ask window to reinitialize itself.
     *
     * Default implementation provided by base class does nothing.
     *
     * @param[in] self_     window context handle
     * @param[in] items     reasons for reinitialization
     **/
    void (*reinitializeWindow)(QBWindowContext self_, SvArray items);

    /**
     * Destroy window represented by window context.
     *
     * @param[in] self_     window context handle
     **/
    void (*destroyWindow)(QBWindowContext self_);

    /**
     * Handle input event.
     *
     * Default implementation provided by base class just calls
     * svAppDeliverInputEvent().
     *
     * @param[in] self_ window context handle
     * @param[in] e     input event
     * @return          @c true if event was handled, @c false otherwise
     **/
    bool (*handleInputEvent)(QBWindowContext self, const QBInputEvent *e);
} *QBWindowContextVTable;

/**
 * Get runtime type identification object representing window context class.
 * @return Window context type identification object
 **/
SvType QBWindowContext_getType(void);

/**
 * Create window for window context.
 *
 * @memberof QBWindowContext
 *
 * @param[in] self      window context handle
 * @param[in] app       CUIT application handle
 **/
extern void QBWindowContextCreateWindow(QBWindowContext self, SvApplication app);

/**
 * Ask window to reinitialize itself.
 *
 * @memberof QBWindowContext
 *
 * Default implementation does nothing.
 *
 * @param[in] self      window context handle
 * @param[in] items     reasons for reinitialization
 **/
extern void QBWindowContextReinitializeWindow(QBWindowContext self, SvArray items);

/**
 * Destroy window represented by window context.
 *
 * @memberof QBWindowContext
 *
 * @param[in] self      window context handle
 **/
extern void QBWindowContextDestroyWindow(QBWindowContext self);

/**
 * Handle input event.
 *
 * Default implementation just calls svAppDeliverInputEvent().
 *
 * @param[in] self_ window context handle
 * @param[in] e     input event
 * @return          @c true if event was handled, @c false otherwise
 **/
extern bool QBWindowContextHandleInputEvent(QBWindowContext self_, const QBInputEvent *e);

/**
 * @}
 **/

#endif
