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

#ifndef QBLATENS_H_
#define QBLATENS_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <QBCAS.h>
#include "QBLatensOSDDisplayer.h"
#include "QBLatensPinStatusListener.h"

/**
 * @defgroup QBLatens Latens CAS API implementation.
 * @{
 **/

/**
 * @file QBLatens.h The main header file of the Latens CAS API implementation.
 **/

/**
 * @class QBLatens
 * @brief QBLatens is used for communicating between the application and Latens CAS library
 * QBLatens is a singleton.
 */
typedef struct QBLatens_s *QBLatens;

/**
 * Possible Latens running modes
 */
typedef enum {
    QBLatensMode_unknown,
    QBLatensMode_dvb,
    QBLatensMode_ip,
    QBLatensMode_ott
} QBLatensMode;

typedef enum QBLatensEntType {
    QBLatensEntType_pvr,      /**< PVR Record/Playback/Timeshift Entitlement */
    QBLatensEntType_max
} QBLatensEntType;


/**
 * @brief Add pin status listener.
 * This method adds a new listener to be notified about remote pin
 * chane request.
 *
 * @param[in] self reference to QBLatens instance
 * @param[in] listener object to be added as new listener
 */
void QBLatensAddPinStatusListener(QBLatens self, SvObject listener);

/**
 * @brief Sets Latens OSD displayer.
 * Only one OSD displayer might be set. Mulitple function call overwrites current OSD displayer
 *
 * @param self      handle to QBLatens instance
 * @param displayer object to be set as OSD displayer (must implement @ref QBLatensOSDDisplayer interface)
 */
void QBLatensSetOSDDisplayer(QBLatens self, SvObject displayer);


/**
 * @brief Get runtime type specification of QBLatens type
 *
 * @return QBLatens runtime type specification
 */
SvType QBLatens_getType(void);

/**
 * @brief Return global instace of QBLatens (singleton)
 *
 * @return QBLatens global instance
 */
QBLatens QBLatensGetInstance(void);

/**
 * @brief Remove Remote Message Screen
 * This method should be called by @ref QBLatensOSDDisplayer implementation to notify latens library to request removing the Remote Message Screen
 *
 * @param self         handle to QBLatens instance
 * @param screenHandle screenHandle to be removed
 */
void QBLatensRemoveRemoteMessageScreen(QBLatens self, int screenHandle);

/**
 * @interface QBLatensVersionListener
 * @brief QBLatensVersionListener is an interface to be implemented by instances which want to know the Latens Glue version
 */
typedef struct QBLatensVersionListener_i {
    /**
     * @brief Notify listeners that Latens Glue version has changed
     *
     * @param[in] self_            handle to an object implementing @ref QBLatensOSDDisplayer
     * @param[in] glueVersion      Latens Glue version
     **/
    void (*glueVersionChanged)(SvObject self_,
                               SvString glueVersion);
} *QBLatensVersionListener;

/**
 * @brief Get runtime type identification object representing Latens version Listener interface.
 *
 * @return QBLatensVersionListener interface object
 */
SvInterface QBLatensVersionListener_getInterface(void);

/**
 * @brief Add version listener.
 * This method adds a new listener to be notified about Latens Glue version changes
 * If glue version is available in the moment of adding the listener it will be notified synchronously
 *
 * @param[in] self     reference to QBLatens instance
 * @param[in] listener object to be added as a new listener
 */
void QBLatensAddVersionListener(QBLatens self, SvObject listener);

/**
 * @brief Cleanup files stroed by Latens and reboot STB.
 *
 * @param[in] self     reference to QBLatens instance
 */
void QBLatensCleanupStorage(QBLatens self);

/**
 * @brief Sets Latesn operation mode
 *
 * @param[in] self     reference to QBLatens instance
 * @param[in] mode     mode to set
 */
void QBLatensSetMode(QBLatens self, QBLatensMode mode);

/**
 * @brief Object which implements that interface can receive information about Latens entitlements.
 */
typedef const struct QBLatensEntitlementsListener_s {
    /**
     * @brief It is called each time some entitlement is enabled or disabled
     * @param[in] self_ listener instance
     * @param[in] type type of entitlement
     * @param[out] enabled indicates if entitlement is given
     */
    void (*entUpdate)(SvObject self_, QBLatensEntType type, bool enabled);
} *QBLatensEntitlementsListener;

/**
 * @brief Get runtime type identification object representing Latens entitlements Listener interface.
 *
 * @return QBLatensEntitlementsListener interface object
 */
SvExport SvInterface QBLatensEntitlementsListener_getInterface(void);

/**
 * @brief Add entitlements listener.
 * This method adds a new listener to be notified about entitlements changes.
 *
 * @param[in] self reference to QBLatens instance
 * @param[in] listener object to be added as new listener
 */
void QBLatensEntitlementsAddListener(QBLatens self, SvObject listener);

/**
 * @brief Removes entitlements listener.
 * This method removes listener from notifications about entitlements changes.
 *
 * @param[in] self reference to QBLatens instance
 * @param[in] listener object to be removed as new listener
 */
void QBLatensEntitlementsRemoveListener(QBLatens self, SvObject listener);

/**
 * @brief Checks if given entitlement is enabled.
 *
 * @param[in] self reference to QBLatens instance
 * @param[in] type entitlement id
 */
bool QBLatensEntitlementsIsEnabled(QBLatens self, QBLatensEntType type);

/**
 * @brief Sets latens CAS Server
 *
 * @param[in] self     reference to QBLatens instance
 * @param[in] address  IP address of CAS server
 * @param[in] port     port of CAS server
 */
void QBLatensSetCasServer(QBLatens self, SvString address, uint16_t port);

/**
 * @brief Sets latens Web Server. Locator and SSM will be downloaded from the server
 * when operating in unmanaged network (OTT mode).
 *
 * @param[in] self     reference to QBLatens instance
 * @param[in] address  IP address of the Web Server
 */
void QBLatensSetWebServer(QBLatens self, SvString address);

/**
 * @brief Add ERM session listener.
 * This method adds a new listener to be notified via QBDRMSystemListener interface
 * when Latens stops an ERM session.
 *
 * @param[in] self     reference to QBLatens instance
 * @param[in] listener object to be added as a new listener
 */
void QBLatensAddERMSessionListener(QBLatens self, SvObject listener);

/** @} */

#endif /* QBLATENS_H_ */
