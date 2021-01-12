/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_VIEWRIGHT_DVB_DRM_INFO_H_
#define SV_VIEWRIGHT_DVB_DRM_INFO_H_

/**
 * @file SvViewRightDvbDRMInfo.h SvViewRightDvbDRMInfo class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvType.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvViewRightDvbDRMInfo ViewRightWeb DRM information class.
 * @ingroup SvPlayerKit
 * @{
 **/

/**
 * Verimatrix DRM plugin ID.
 **/
#define DRM_VIEWRIGHT_DVB 0x11

/**
 * ViewRightWeb DRM information class.
 * @class SvViewRightDvbDRMInfo
 * @extends SvDRMInfo
 **/
typedef struct SvViewRightDvbDRMInfo_s *SvViewRightDvbDRMInfo;

SvType SvViewRightDvbDRMInfo_getType(void);

/**
 * @brief Constructor of SvViewRightDvbDRMInfo class.
 * @param[out] errorOut error handle
 * @return instance of SvViewRightDvbDRMInfo
 */
SvViewRightDvbDRMInfo
SvViewRightDvbDRMInfoCreate(SvData globalInfo, SvData storeInfo, SvErrorInfo *errorOut);

/**
 * @brief Construct SvViewRightDvbDRMInfo from global and store info files
 * @param[in] globalInfoPath path to global info file
 * @param[in] storeInfoPath path to store info file
 * @param[out] errorOut error handle
 * @return instance of SvViewRightDvbDRMInfo
 */
SvViewRightDvbDRMInfo
SvViewRightDvbDRMInfoCreateFromFiles(SvString globalInfoPath, SvString storeInfoPath, SvErrorInfo *errorOut);

/**
 * @brief Get global info data
 * @param self ViewRightDvbDRMInfo handle
 * @return global info data
 */
SvData
SvViewRightDvbDRMInfoGetGlobalInfo(SvViewRightDvbDRMInfo self);

/**
 * @brief Get store info data
 * @param self ViewRightDvbDRMInfo handle
 * @return store info data
 */
SvData
SvViewRightDvbDRMInfoGetStoreInfo(SvViewRightDvbDRMInfo self);

/**
 * @}
 * */

#ifdef __cplusplus
}
#endif

#endif
