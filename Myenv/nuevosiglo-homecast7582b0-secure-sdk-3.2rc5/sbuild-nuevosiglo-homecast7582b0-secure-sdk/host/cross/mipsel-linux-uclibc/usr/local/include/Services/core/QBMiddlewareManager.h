/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBMIDDLEWAREMANAGER_H_
#define QBMIDDLEWAREMANAGER_H_

#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBSmartCardMonitor.h>
#include <stdbool.h>

typedef struct QBMiddlewareManager_t *QBMiddlewareManager;

typedef enum {
    QBMiddlewareManagerType_Innov8on = 0,
    QBMiddlewareManagerType_Traxis,
    QBMiddlewareManagerType_Digitalsmiths,
    QBMiddlewareManagerType_TiVo,
    QBMiddlewareManagerType_MAX
} QBMiddlewareManagerType;

typedef struct {
    bool redirProxyEnabled;
    SvString prefixUrl;
    SvString cmdLineUrl;
    SvSSLParams sslParams;
} QBMiddlewareManagerInitParams;

extern QBMiddlewareManager QBMiddlewareManagerCreate(void);
extern QBMiddlewareManager QBMiddlewareManagerInit(QBMiddlewareManager self, QBMiddlewareManagerInitParams *initParams);
extern void QBMiddlewareManagerStart(QBMiddlewareManager self, QBSmartCardMonitor smartCardMonitor);
extern void QBMiddlewareManagerStop(QBMiddlewareManager self);
/**
 * Check if network requirements for Middleware Manager are met.
 * The network requirements are defined by QBNetworkLogic and passed to QBNetworkWatcher.
 *
 * @param[in] self  QBMiddlewareManager instance handle
 * @return  @c true if the network requirements are met, @c false otherwise
 **/
extern bool QBMiddlewareManagerHasNetwork(QBMiddlewareManager self);
extern SvString QBMiddlewareManagerGetId(QBMiddlewareManager self, QBMiddlewareManagerType middlewareType);
extern SvString QBMiddlewareManagerGetPrefixUrl(QBMiddlewareManager self, QBMiddlewareManagerType middlewareType);
extern SvString QBMiddlewareManagerGetThumbnailsPrefixUrl(QBMiddlewareManager self, QBMiddlewareManagerType middlewareType);
extern SvXMLRPCServerInfo QBMiddlewareManagerGetXMLRPCServerInfo(QBMiddlewareManager self, QBMiddlewareManagerType middlewareType);

extern void QBMiddlewareManagerAddListener(QBMiddlewareManager self, SvObject listener);
extern void QBMiddlewareManagerRemoveListener(QBMiddlewareManager self, SvObject listener);

extern void QBMiddlewareManagerSetURL(QBMiddlewareManager self, QBMiddlewareManagerType middlewareType, SvString middlewareUrl);

extern SvInterface QBMiddlewareManagerListener_getInterface(void);

typedef const struct QBMiddlewareManagerListener_t {
    void (*middlewareDataChanged)(SvObject self_, QBMiddlewareManagerType middlewareType);
} *QBMiddlewareManagerListener;


#endif
