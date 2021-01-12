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

/* App/Libraries/QBDSMCC/Headers/QBDSMCC/QBDSMCCDataProvider.h */

#ifndef QB_DSMCC_DATA_PROVIDER_H_
#define QB_DSMCC_DATA_PROVIDER_H_

/**
 * @file QBDSMCCDataProvider.h
 * @brief DSM-CC Data Provider API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDSMCCClient.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBDSMCCDataProvider DSM-CC Data Provider
 * @ingroup QBDSMCC
 * @{
 **/

/**
 * QBDSMCCDataProvider class.
 *
 * On start or dataReady notification from QBDSMCCClient:
 * - get /desc.json from client and find service description, which path is
 *   the same as given serviceID
 * - get /servicePath/desc.json
 * - validate structure of received JSON data
 * - find the following structures and replace paths with SvRBBitmaps:
 *      - "image": "path"
 *      - "images": [ "path1", "path2", ... ]
 *      - "images": { "id1": "path1. "id2": "path2", ... }
 * - notify listeners
 *
 * @class QBDSMCCDataProvider QBDSMCCDataProvider.h <QBDSMCC/QBDSMCCDataProvider.h>
 * @extends QBContentProvider
 * @implements QBDSMCClientListener
 * @implements QBDSMCCDataParserListener
 **/
typedef struct QBDSMCCDataProvider_s *QBDSMCCDataProvider;

/**
 * Create QBDSMCCDataProvider
 *
 * @param[in] dsmccClient   QBDSMCCClient handle
 * @param[in] serviceID     Id of service for which this provider is used. This
 *                          is equal to "path" field in /desc.json file
 * @return                  QBDSMCCDataProvider handle or NULL on error
 **/
QBDSMCCDataProvider QBDSMCCDataProviderCreate(QBDSMCCClient dsmccClient, SvString serviceID);

/**
 * Get object representing parsed JSON data. May be @c NULL if it's unavailable
 * or an error occured while processing data.
 *
 * @param[in] self          QBDSMCCDataProvider handle
 * @return                  object representing parsed JSON data or @c NULL
 **/
SvHashTable QBDSMCCDataProviderGetObject(QBDSMCCDataProvider self);

/**
 * QBDSMCCDataProvider listener interface
 **/
typedef const struct QBDSMCCDataProviderListener_ {
    /**
     * Method called when new object is received from the QBDSMCCClient for
     * service identified by serviceID.
     *
     * @param[in] self_     listener handle
     * @param[in] serviceID serviceID given in @ref QBDSMCCDataProviderCreate
     **/
    void (*objectReceived)(SvObject self_, SvString serviceID);
} *QBDSMCCDataProviderListener;

/**
 * Get runtime type identification object representing
 * DSM-CC data provider listener interface.
 *
 * @return  DSM-CC data provider listener interface identification object
 **/
SvInterface QBDSMCCDataProviderListener_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_DSMCC_DATA_PROVIDER_H_ */
