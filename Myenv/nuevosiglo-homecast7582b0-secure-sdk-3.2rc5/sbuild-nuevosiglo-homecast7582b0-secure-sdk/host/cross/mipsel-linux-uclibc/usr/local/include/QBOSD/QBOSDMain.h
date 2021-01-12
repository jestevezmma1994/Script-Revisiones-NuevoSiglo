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

#ifndef QBOSDMAIN_H_
#define QBOSDMAIN_H_

/**
 * @file QBOSDMain.h Main OSD component
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBOSD/QBOSD.h>

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup QBOSDMain Main OSD component
 * @ingroup QBOSD
 * @{
 *
 * An abstract base class for main OSD component.
 **/

/**
 * @relates QBOSDMain
 *
 * Attachment position for OSD components.
 **/
typedef enum {
    QBOSDMainAttachmentPosition_top,    ///< attach on top
    QBOSDMainAttachmentPosition_bottom, ///< attach on bottom

    QBOSDMainAttachmentPosition_cnt     ///< count of possible attachment positions
} QBOSDMainAttachmentPosition;

/**
 * An abstract base class for main OSD component.
 *
 * @class QBOSDMain QBOSDMain.h <QBOSD/QBOSDMain.h>
 * @extends QBOSD
 *
 * Object managing all the components of the OSD should derive from this class
 * and should provide QBOSDMain::attachOSD implementation.
 **/
typedef struct QBOSDMain_ *QBOSDMain;

/**
 * @relates QBOSDMain
 *
 * Main OSD class internals.
 **/
struct QBOSDMain_ {
    /**
     * super class
     **/
    struct QBOSD_ super_;

    /**
     * an array containing handlers for each type OSD type
     *
     * @note Don't access this attribute directly, use QBOSDGetHandler() instead.
     **/
    QBOSD osdTypesToHandlers[QBOSDHandlerType_cnt];
};

/**
 * @relates QBOSDMain
 *
 * QBOSDMain virtual methods table
 */
typedef const struct QBOSDMainVTable_ {
    /**
     * SvObject virtualmethods table
     */
    struct QBOSDVTable_ super_;

    /**
     * Attach OSD component to main OSD handler.
     *
     * @param[in]  self            main OSD object handle
     * @param[in]  osd             OSD component
     * @param[in]  position        attachment position
     * @param[in]  attachmentLevel attachment level
     * @param[out] errorOut        error info
     **/
    void (*attachOSD)(QBOSDMain self,
                      QBOSD osd,
                      QBOSDMainAttachmentPosition position,
                      int attachmentLevel,
                      SvErrorInfo *errorOut);
} *QBOSDMainVTable;

/**
 * @relates QBOSDMain
 *
 * Get runtime type identification object representing main OSD class.
 *
 * @return QBOSDMain type identification object
 **/
SvType
QBOSDMain_getType(void);

/**
 * Initialize main OSD.
 *
 * @protected @memberof QBOSDMain
 *
 * @note Internally, this method calls @ref QBOSDInit().
 *
 * @param[in]  self         main OSD object handle
 * @param[in]  w            main OSD widget
 * @param[in]  id           main OSD widget identifier
 * @param[out] errorOut     error info
 **/
void
QBOSDMainInit(QBOSDMain self,
              SvWidget w,
              SvString id,
              SvErrorInfo *errorOut);

/**
 * Get OSD handler associated with given type.
 *
 * @public @memberof QBOSDMain
 *
 * @param[in]  self        main OSD object handle
 * @param[in]  handlerType type of the handler
 * @param[out] errorOut    error info
 * @return                 OSD handler for given type or @c NULL in case of error
 **/
QBOSD
QBOSDMainGetHandler(QBOSDMain self,
                    QBOSDHandlerType handlerType,
                    SvErrorInfo *errorOut);

/**
 * Attach OSD component to main OSD handler.
 *
 * @public @memberof QBOSDMain
 *
 * Besides widget attachment this function should fill
 * QBOSDMain::osdTypesToHandlers table. Remember that only one handler for
 * each type is allowed.
 *
 * Example implementation:
 * @code
QBOSDMainAttachOSD(QBOSDMain self_,
                   QBOSD osd,
                   QBOSDMainAttachmentPosition position,
                   int attachmentLevel,
                   SvErrorInfo *errorOut)
{
    QBOSDMainImplementation self (QBOSDMainImplementation) self_;

    if (self_->osdTypesToHandlers[type]) {
        // Error: osd handler for given type already attached
    }

    SvWidget osdWidget = NULL;
    switch (position) {
        case QBOSDMainAttachmentPosition_top:
            osdWidget = self->osdTop;
            break;
        case QBOSDMainAttachmentPosition_bottom:
            osdWidget = self->osdBottom;
            break;
        default:
            // Error: unknown attachment position
    }

    QBOSDHandlerType type = QBOSDGetType(osd, NULL);
    SvString id = QBOSDGetId(osd, NULL);
    SvWidget w = QBOSDTakeWidget(osd, &error);

    svSettingsWidgetAttach(osdWidget, w, SvStringCString(id), attachmentLevel);

    self_->osdTypesToHandlers[type] = SVRETAIN(osd);
}
 * @endcode
 *
 * @param[in]  self            main OSD object handle
 * @param[in]  osd             OSD component
 * @param[in]  position        attachment position
 * @param[in]  attachmentLevel attachment level
 * @param[out] errorOut        error info
 **/
void
QBOSDMainAttachOSD(QBOSDMain self,
                   QBOSD osd,
                   QBOSDMainAttachmentPosition position,
                   int attachmentLevel,
                   SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBOSDMAIN_H_ */
