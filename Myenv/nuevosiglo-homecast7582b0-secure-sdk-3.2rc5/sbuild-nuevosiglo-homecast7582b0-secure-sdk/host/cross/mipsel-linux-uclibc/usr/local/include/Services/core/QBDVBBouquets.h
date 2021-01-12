/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2017 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBDVBBOUQUETS_H_
#define QBDVBBOUQUETS_H_

/**
 * @file QBDVBBouquets.h
 * @brief Async service creating playlists based on bouquets from BAT (Boquet Association Table)
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <QBDVBTableMonitor.h>
#include <Services/core/playlistManager.h>

/**
 * @defgroup QBDVBBouquets service
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * Type of bouquets mask
 **/
typedef enum {
    QBDVBBouquetsMaskType_unknown = 0,  ///< unknown bouquet mask
    QBDVBBouquetsMaskType_allow,        ///< allow type bouquet mask
    QBDVBBouquetsMaskType_disallow      ///< disallow type bouquet mask
} QBDVBBouquetsMaskType;

/**
 * QBDVBBouquets class.
 *
 * @class QBDVBBouquets
 * @extends QBObservable
 * @implements QBDVBTableMonitorListener
 * @implements QBAsyncService
 * @implements QBInitializable
 *
 * This class implements DVB bouquets service. Service listens to DVBTableMonitor and receives names
 * of available services bouquets.
 * Received channels lists show up in main menu in favorites
 * node.
 **/
typedef struct QBDVBBouquets_ *QBDVBBouquets;

/**
 * Get runtime type identification object representing QBDVBBouquets class.
 *
 * @relates QBDVBBouquets
 *
 * @return type identification object
**/
SvType QBDVBBouquets_getType(void);

/**
 * @}
 **/

#endif /* QBDVBBOUQUETS_H_ */
