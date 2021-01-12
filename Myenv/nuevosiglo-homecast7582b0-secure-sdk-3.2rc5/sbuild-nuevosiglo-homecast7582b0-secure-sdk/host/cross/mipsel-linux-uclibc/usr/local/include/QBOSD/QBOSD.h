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

#ifndef QBOSD_H_
#define QBOSD_H_

/**
 * @file QBOSD.h On Screen Display base class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <QBAppKit/QBObservable.h>

/**
 * @defgroup QBOSD On screen display modules
 * @{
 *
 * A base class for OSD components.
 **/

/**
 * @relates QBOSD
 *
 * On screen display handler types.
 *
 * Remember to keep this enum in sync with available abstract OSD components.
 **/
typedef enum {
    QBOSDHandlerType_main,         ///< main OSD handler
    QBOSDHandlerType_recording,    ///< recording OSD handler

    QBOSDHandlerType_cnt           ///< number of possible OSD handler types
} QBOSDHandlerType;

/**
 * A base class for for OSD components.
 *
 * @class QBOSD QBOSD.h <QBOSD/QBOSD.h>
 * @extends QBObservable
 *
 * This class is a simple wrapper for OSD widgets. It provides basic API
 * for managing OSD components in an object-oriented way. Use it as a base
 * class for real OSD components.
 *
 * QBOSD derives from QBObservable, so components based on it can be observed
 * by objects implementing QBObserver interfrace. Use QBOSDGetState() inside
 * QBObserver::observedObjectUpdated handler to get state of OSD widget. Note,
 * that default implementation of QBOSDGetState() returns nothing.
 *
 * @warning This class should derive from SvWidget, but because SvWidget
 *     deosn't derive from SvObject, underlying widget is added as an attribute
 *     of QBOSD class. For this reason, special care should be taken when
 *     accessing underlying widget.
 * @warning As long as the widget is not taken, QBOSD owns the instance of
 *     the widget and destroys it when it is necessary. When the widget will be
 *     taken (see @ref QBOSDTakeWidget()), taker is responsible for widget
 *     destruction.
 * @warning Provided widget **MUST** have SvWidget::clean and SvWidget::prv
 *     attributes set to @c NULL. QBOSD sets SvWidget::prv to self, so any
 *     class deriving from QBOSD can register and use custom handlers for
 *     input, effect, user and timer events. QBOSD
 *     registers it's own SvWidget::clean handler to be notified when internal
 *     widget is destroyed. After internal widget destruction QBOSD instance
 *     can not be used any more and must be released as soon as possible.
 **/
typedef struct QBOSD_ *QBOSD;

/**
 * @relates QBOSD
 *
 * On screen display class internals.
 **/
struct QBOSD_ {
    /**
     * super class
     **/
    struct QBObservable_ super_;

    /**
     * type of OSD handler
     *
     * @note Don't access this attribute directly, use QBOSDGetType() instead.
     **/
    QBOSDHandlerType type;

    /**
     * OSD widget identifier
     *
     * @note Don't access this attribute directly, use QBOSDGetId() instead.
     **/
    SvString id;

    /**
     * OSD widget
     *
     * @note Don't access this attribute directly, use QBOSDTakeWidget() instad.
     **/
    SvWidget widget;

    /**
     * if widget was taken then destruction is the taker responsibility
     *
     * @warning Don't access this attribute, it's used internally to indicate
     *          if widget ownership was taken. **Modyfing this attribute may
     *          cause memory leak or crash**.
     **/
    bool widgetTaken;

    /**
     * flag used to indicate whether internal widget is destroyed
     *
     * @warning Don't access this attribute, it's used internally to indicate
     *          if widget ownership was taken. **Modyfing this attribute may
     *          cause memory leak or crash**.
     * */
    bool widgetValid;
};

/**
 * @relates QBOSD
 *
 * OSD component state.
 **/
typedef struct {
    bool show;      ///< OSD component should be shown
    bool forceShow; ///< OSD component should be shown immidiately
    bool lock;      ///< lock OSD component on screen
    bool allowHide; ///< OSD hidding is allowed
} QBOSDState;

/**
 * @relates QBOSD
 *
 * QBOSD virtual methods table.
 */
typedef const struct QBOSDVTable_ {
    /**
     * SvObject virtualmethods table
     */
    struct SvObjectVTable_ super_;

    /**
     * Get state of the OSD widget
     *
     * @param[in]  self     OSD object handle
     * @param[out] errorOut error info
     * @return              OSD state
     **/
    QBOSDState (*getState)(QBOSD self,
                           SvErrorInfo *errorOut);
} *QBOSDVTable;

/**
 * @relates QBOSD
 *
 * Get runtime type identification object representing the OSD class.
 *
 * @return QBOSD type identification object
 **/
SvType QBOSD_getType(void);

/**
 * Initialize OSD component.
 *
 * @protected @memberof QBOSD
 *
 * Widget destruction depends on current widget owner (see @ref QBOSDTakeWidget).
 *     - if widget is not taken, it will be destroyed by QBOSD.
 *     - if widget is taken, QBOSD will not destroy it.
 * It is not possible to return widget ownership back to QBOSD.
 *
 * Widget identifier can be used to attach widget using settings manager
 * (see @ref svSettingsWidgetAttach()), so identifier value must be equal to
 * settings path used upon widget creation.
 *
 * @warning Provided widget **MUST** have SvWidget::clean
 *          and SvWidget::prv attributes set tu @c NULL. QBOSD will register
 *          custom SvWidget::clean handler and set SvWidget::prv to self
 *          (for more info see @ref QBOSD class description).
 *
 * @param[in]  self     OSD object handle
 * @param[in]  type     type of OSD object
 * @param[in]  widget   underlying widget
 * @param[in]  id       widget identifier
 * @param[out] errorOut error info
 **/
void
QBOSDInit(QBOSD self,
          QBOSDHandlerType type,
          SvWidget widget,
          SvString id,
          SvErrorInfo *errorOut);

/**
 * Take OSD widget.
 *
 * @protected @memberof QBOSD
 *
 * Caller takes ownership of the underlying widget, so it's responsible
 * to destroy it when it is no longer needed. It is not possible to return
 * widget ownership back to QBOSD.
 *
 * @param[in]  self     OSD object handle
 * @param[out] errorOut error info
 * @return              OSD widget or @c NULL in case of error
 **/
SvWidget
QBOSDTakeWidget(QBOSD self, SvErrorInfo *errorOut);

/**
 * Get OSD widget identifier.
 *
 * @protected @memberof QBOSD
 *
 * Widget identifier can be used in settings manager.
 *
 * @param[in]  self     OSD object handle
 * @return              OSD widget identifier or @c NULL in case of error
 **/
SvString
QBOSDGetId(QBOSD self);

/**
 * Get type of OSD component.
 *
 * @protected @memberof QBOSD
 *
 * @param[in]  self     OSD object handle
 * @return              type of OSD component or QBOSDHandlerType_unknown
 *                      in case of error
 **/
QBOSDHandlerType
QBOSDGetType(QBOSD self);

/**
 * Get state of the OSD widget.
 *
 * @public @memberof QBOSD
 *
 * Use this function inside QBObserver::observedObjectUpdated handler to get
 * state of OSD component.
 *
 * @note Default implementation of this function returns nothing.
 *
 * @param[in]  self     OSD object handle
 * @param[out] errorOut error info
 * @return              OSD state
 **/
QBOSDState
QBOSDGetState(QBOSD self,
              SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBOSD_H_ */
