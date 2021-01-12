/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_DLNA_PROVIDER_H_
#define QB_DLNA_PROVIDER_H_

/**
 * @file QBDLNAProvider.h
 * @brief DLNA VoD provider class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdlib.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBContentManager/QBContentTree.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBDLNAProviderParams.h>


/**
 * @defgroup QBDLNA DLNA VoD provider
 * @ingroup DataLayer
 **/


/**
 * @defgroup QBDLNAProvider DLNA VoD provider class
 * @ingroup QBDLNA
 * @{
 **/

/**
 * DLNA VoD provider class.
 * @class QBDLNAProvider
 * @extends QBContentProvider
 **/
typedef struct QBDLNAProvider_ *QBDLNAProvider;

/**
 * Get runtime type identification object representing DLNA VoD provider class.
 * @return DLNA VoD provider class
 **/
extern SvType
QBDLNAProvider_getType(void);

/**
 * Create DLNA VoD provider.
 *
 * @param[in] tree      handle to a content tree to put VoD content in
 * @param[in] params    provider parameters
 * @param[in] service   name of the service all categories should belong to,
 *                      @c NULL to accept all categories
 * @param[out] errorOut error info
 * @return              created provider, @c NULL in case of error
 **/
extern QBDLNAProvider
QBDLNAProviderCreate(QBContentTree tree,
                     QBDLNAProviderParams params,
                     SvString service,
                     SvErrorInfo *errorOut);

/**
 * Schedule DLNA service discovery after specified delay.
 *
 * @param[in] self_     DLNA VoD provider handle
 * @param[in] discoveryDelay how long to wait (in milliseconds) before starting discovery
 **/
extern void
QBDLNAProviderPerformDiscoveryAfter(SvObject self_, unsigned int discoveryDelay);

/**
 * Cancel ongoing DLNA service discovery.
 *
 * @param[in] self_     DLNA VoD provider handle
 **/
extern void
QBDLNAProviderCancelDiscovery(SvObject self_);

/**
 * @}
 **/


#endif
