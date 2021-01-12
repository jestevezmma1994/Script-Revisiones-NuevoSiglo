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

#ifndef QBPROVIDERSCCONTROLLERSERVICEINNOV8ON_H_
#define QBPROVIDERSCCONTROLLERSERVICEINNOV8ON_H_

/**
 * @file QBProvidersControllerServiceInnov8on.h Providers Controller Service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBProvidersControllerServiceInnov8on Innov8on Providers Controller Service
 * @ingroup CubiTV_services
 * @{
 **/

#include <SvFoundation/SvObject.h>
#include <main_decl.h>

/**
 * Create instance of Innov8on services controller.
 *
 * @param appGlobals  AppGlobals handle
 * @return            Innov8on service controller handle, or NULL on error.
 */
SvObject
QBProvidersControllerServiceInnov8onCreate(AppGlobals appGlobals);

// BEGIN AMERELES Update VoD Services
/**
 * Llama al procedimiento que es utilizado para actualizar los servicios VoD cada 12 horas (por defecto).
 *
 * @param self SvObject (QBProvidersControllerServiceInnov8on)
 * @}
 **/
void QBProvidersControllerServiceInnov8onUpdateVODServices(SvObject self);

/**
 * Retorna la cantidad de veces que fue llamado el actualizar servicios VoD.
 *
 * @param self SvObject (QBProvidersControllerServiceInnov8on)
 * @return            cantidad de veces que fue llamado el actualizar servicios VoD.
 **/
int QBProvidersControllerServiceInnov8onGetUpdateVODServicesCount(SvObject self);

/**
 * Modifica la cantidad de veces que fue llamado el actualizar servicios VoD.
 *
 * @param self SvObject (QBProvidersControllerServiceInnov8on)
 **/
void QBProvidersControllerServiceInnov8onSetUpdateVODServicesCount(SvObject self, int val);
// END AMERELES Update VoD Services

// AMERELES #2206 Canal lineal que lleve a VOD
/**
 * Retorna el servicio con ID serviceID
 *
 * @param self SvObject
 * @}
 **/
SvObject QBProvidersControllerServiceInnov8onGetInnov8onServiceByID(SvObject self_, SvString tag, SvString serviceID);

/**
 * @}
 **/

#endif
