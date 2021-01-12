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

#ifndef QB_SERVICES_VIEWRIGHT_WEB_H
#define QB_SERVICES_VIEWRIGHT_WEB_H

#include <main_decl.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * @file QBViewRightWebManager.h
 * @brief ViewRightWebService
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


/**
 * @defgroup QBViewRightWebService ViewRightWeb service
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * @typedef QBViewRightWebService
 * @brief It manages ViewRightWeb drm plugin used for hls playback.
 *   It is used for retrieving keys for encrypted hls content from VCAS server
 */
typedef struct QBViewRightWebService_s * QBViewRightWebService;

/**
 * Constructor of QBViewRightWebService
 * @param appGlobals appGlobals
 * @return instance of QBViewRightWebService
 */
QBViewRightWebService QBViewRightWebServiceCreate(AppGlobals appGlobals);

/**
 * @brief It registers listener at QBViewRightWebService
 * @param self handle of QBViewRightWebService
 * @param listener instance of object which implements interface of QBViewRightWebListener
 */
void QBViewRightWebServiceSetListener(QBViewRightWebService self, SvGenericObject listener);

/**
 * @brief Start QBViewRightWebService. That method is asynchronous.
 * It will launch ViewRightWeb drm plugin.
 * The following steps will be performed (all actions will be performed asynchronous):
 * - Check if STB has been provisioned. If not then drm plugin will try to connect to VCAS server and provision the device
 * - Check connection with VCAS server
 *
 * @param self handler of QBViewRightWebService
 */
void QBViewRightWebServiceStart(QBViewRightWebService self);

/**
 * @brief Stop QBViewRightWebService
 * @param self handler of QBViewRightWebService
 */
void QBViewRightWebServiceStop(QBViewRightWebService self);

/**
 * @}
 **/


#endif /// QB_SERVICES_VIEWRIGHT_WEB_H
