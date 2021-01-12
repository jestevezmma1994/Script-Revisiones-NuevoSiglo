/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_AD_SERVICE_H_
#define QB_AD_SERVICE_H_

#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <main_decl.h>

typedef struct QBAdService_t *QBAdService;

/**
 * Notify that ad id has changed.
 *
 * @param[in] self_      handle to a QBAdServiceListener
 **/
typedef void (*QBAdServiceCurrentAdIdChanged) (SvGenericObject self_);

struct QBAdServiceListener_t {
    QBAdServiceCurrentAdIdChanged currentAdChanged;
};

typedef struct QBAdServiceListener_t* QBAdServiceListener;

/**
 * Get runtime type identification object representing
 * QBAdServiceListener interface.
 *
 * @return QBAdServiceListener interface object
 **/
SvInterface QBAdServiceListener_getInterface(void);

/**
 * Add an ad service listener.
 *
 * @param[in] self      ad service instance handle
 * @param[in] listener  listener object handle
 * @param[out] errorOut error info
 **/
void QBAdServiceAddListener(QBAdService self, SvGenericObject listener, SvErrorInfo *errorOut);

/**
 * Remove an ad service listener.
 *
 * @param[in] self      ad service instance handle
 * @param[in] listener  listener object handle
 * @param[out] errorOut error info
 **/
void QBAdServiceRemoveListener(QBAdService self, SvGenericObject listener, SvErrorInfo *errorOut);

/**
 * Create new ad service instance.
 * Ad service allows displaying adverts targeted to a specified customer id.
 * If available for the customer, middleware provides stb with current ad data
 * such as resource url and ad view count.
 *
 * @param[in] appGlobals application globals
 *
 * @return               @a self or @c NULL in case of error
 **/
QBAdService QBAdServiceCreate(AppGlobals appGlobals);

/**
 * Start ad service instance.
 *
 * @param[in] self      ad service instance handle
 **/
void QBAdServiceStart(QBAdService self);

/**
 * Stop ad service instance.
 *
 * @param[in] self      ad service instance handle
 **/
void QBAdServiceStop(QBAdService self);

/**
 * Get ad widget from ad service instance.
 *
 * @param[in] self      ad service instance handle
 * @param[in] settings  name of settings file that contains ad widget definition,
 *                      if @c NULL default settings will be used
 *
 * @return               @a ad widget or @c NULL in case of error
 **/
SvWidget QBAdServiceGetAd(QBAdService self, const char* settings);

#endif // QB_AD_SERVICE_H_

