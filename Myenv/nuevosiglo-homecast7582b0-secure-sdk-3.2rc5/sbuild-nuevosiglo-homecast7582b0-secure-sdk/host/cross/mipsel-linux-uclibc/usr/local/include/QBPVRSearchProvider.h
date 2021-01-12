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

#ifndef QB_PVR_SEARCH_PROVIDER_H_
#define QB_PVR_SEARCH_PROVIDER_H_

/**
 * @file QBPVRProvider.h Virtual PVR provider class file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPVRTypes.h>

#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentSearch.h>
#include <SvFoundation/SvArray.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBPVRSearchProvider QBPVRSearchProvider: PVR search provider
 * @{
 **/

/**
 * QBPVR Search provider.
 * @class QBPVRSearchProvider
 * @extends QBContentProvider
 **/
typedef struct QBPVRSearchProvider_ *QBPVRSearchProvider;

/**
 * Create PVR search provider.
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] parent                    parent category
 * @param[in] dataSources               list of data sources in which provider should seek
 * @param[in] searchProviderTest        test associated data
 * @param[out] errorOut                 error info
 * @return                              QBContentSearch instance with search provider, @c NULL in case of error
 **/
QBContentSearch
QBPVRSearchProviderCreate(QBContentCategory parent,
                          SvArray dataSources,
                          QBPVRSearchProviderTest searchProviderTest,
                          SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_PVR_SEARCH_PROVIDER_H_ */
