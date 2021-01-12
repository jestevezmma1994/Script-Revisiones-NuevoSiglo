/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef MSTORE_VOD_PROVIDER_H_
#define MSTORE_VOD_PROVIDER_H_

/**
 * @file  MStoreVoDProvider.h
 * @brief MStoreVoD provider class
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <QBContentManager/QBContentTree.h>
#include <tunerReserver.h>
#include <QBDVBMuxesMap.h>
#include <QBDVBTableMonitor.h>
#include <stdbool.h>

/**
 * MStoreVoD provider class.
 *
 * This class provides data from MStore push vod library
 **/
typedef struct MStoreVoDProvider_ *MStoreVoDProvider;

/**
 * Parameters for MStoreVoD provider
 **/
struct MStoreVoDProviderParams {
    SvString          mountPoint; ///< MStore partition mount point
    QBContentTree     tree;
    bool              createSearch;
    bool              createLatestArrivals;
    bool              createOrderHistory;
    bool              createMyRentals;
};


/**
 * Create MStoreVoD provider.
 *
 * @param[in] params    handle to struct containing MStoreVodProvider parameters
 * @param[out] errorOut error info
 * @return              created provider, @c NULL in case of error
 **/
MStoreVoDProvider
MStoreVoDProviderCreate(struct MStoreVoDProviderParams *params, SvErrorInfo *errorOut);


/**
 * Set requested content language.
 *
 * @param[in] self      MStoreVoD provider handle
 * @param[in] language  language name
 * @param[out] errorOut error info
 **/
void
MStoreVoDProviderSetLanguage(MStoreVoDProvider self, SvString lang, SvErrorInfo *errorOut);

/**
 *  Set data source for 'order history' category
 *
 * @param[in]  self       MStoreVoD provider handle
 * @param[in]  orders     Array of @ref MStoreVoDOrder objects
 * @param[out] errorOut   error info
 **/
void
MStoreVoDProviderSetOrderHistory(MStoreVoDProvider self,
                                 SvArray orders,
                                 SvErrorInfo *errorOut);

/**
 *  Set data source for 'my rentals' category
 *
 * @param[in]  self       MStoreVoD provider handle
 * @param[in]  orders     Array of @ref MStoreVoDOrder objects
 * @param[out] errorOut   error info
 **/
void
MStoreVoDProviderSetMyRentals(MStoreVoDProvider self,
                              SvArray orders,
                              SvErrorInfo *errorOut);

#endif /* MSTORE_VOD_PROVIDER_H_ */

