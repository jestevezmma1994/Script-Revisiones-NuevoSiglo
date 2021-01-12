/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SV_VM_DRM_INFO_H_
#define SV_VM_DRM_INFO_H_

/**
 * @file SvVMDRMInfo.h SvVMDRMInfo class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvVMDRMInfo SvVMDRMInfo class
 * @ingroup SvPlayerKit
 * @{
 **/

/**
 * Verimatrix DRM plugin ID.
 **/
#define DRM_VERIMATRIX    3

/**
 * The SvVMDRMInfo class.
 **/
typedef struct SvVMDRMInfo_ *SvVMDRMInfo;


SvType
SvVMDRMInfo_getType(void);

/**
 * Create a Verimatrix DRM info object.
 *
 * The DRM info object contains an URL of the Verimatrix server.
 * The object should be set to SvContent object that is encrypted by Verimatrix server.
 *
 * @param[in] vm_server_url IP address of the Verimatix server responsible for this content
 * @param[in] vm_server_port port of the Verimatix server responsible for this content; if @c NULL, default 12697 will be used
 * @param[in] vm_company company name as used in STB's certificate
 * @param[out] errorOut error info
 * @return a reference to DRM info object
 **/
SvVMDRMInfo
SvVMDRMInfoCreate(SvString vm_company,
                  SvString vm_server_url,
                  int vm_port,
                  int VODmovieID,
                  SvErrorInfo *errorOut);

/**
 * Set information abount preferred VKS server address
 *
 * @param[in] vks_url -- IP address of the VKS server - if null, vm_server will be used
 * @param[in] vks_port -- port of the VKS server - if zero, default 12699 will be used
 **/
void
SvVMDRMInfoSetVKS(SvVMDRMInfo drm_info,
                  SvString vks_url,
                  int vks_port);

/**
 * Get the server url.
 *
 * This function returns the same SvUrl object that was provided to the constructor.
 * The SvData object is not retained. You must retain it by calling SvURLRetain()
 * if you want it to live longer than the DRM info object.
 *
 * @param[in] self a reference to a Verimatrix DRM Info object.
 * @return a pointer to URL with Verimatrix DRM server's IP address
 **/
SvString
SvVMDRMInfoGetServerURL(SvVMDRMInfo self);

/**
 * Get VKS address.
 */
SvString
SvVMDRMInfoGetVKSURL(SvVMDRMInfo self);

/**
 * Get the server port.
 *
 * This function returns the same port number that was provided to the constructor.
 *
 * @param[in] self a reference to a Verimatrix DRM Info object.
 * @return port of Verimatrix DRM server
 **/
int
SvVMDRMInfoGetServerPort(SvVMDRMInfo self);

/**
 * Get VKS port.
 */
int
SvVMDRMInfoGetVKSPort(SvVMDRMInfo self);

/**
 * Get the company.
 *
 * This function returns the same company name that was provided to the constructor.
 *
 * @param[in] self a reference to a Verimatrix DRM Info object.
 * @return port of Verimatrix DRM server
 **/
SvString
SvVMDRMInfoGetCompany(SvVMDRMInfo self);

/**
 * Get the VOD movie ID.
 *
 * This function returns the same movie ID that was provided to the constructor, which can be used for VOD keys' perfetching.
 *
 * @param[in] self a reference to a Verimatrix DRM Info object.
 * @return port of Verimatrix DRM server
 **/
int
SvVMDRMInfoGetVODMovieID(SvVMDRMInfo self);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
