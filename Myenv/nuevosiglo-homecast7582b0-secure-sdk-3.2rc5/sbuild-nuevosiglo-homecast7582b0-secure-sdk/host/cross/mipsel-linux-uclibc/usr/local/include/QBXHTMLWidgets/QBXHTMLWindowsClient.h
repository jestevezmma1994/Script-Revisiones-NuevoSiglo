/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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
#ifndef XHTMLWINDOWS_CLIENT_H_
#define XHTMLWINDOWS_CLIENT_H_

#include <SvHTTPClient/SvSSLParams.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>
#include <QBSmartCardMonitor.h>
#include <stdlib.h>

/**
 * @file QBXHTMLWindowsClient.h
 * @brief QBXHTMLWindows client class.
 * Client provides way of communication with XHTMLServer by performing asynchronous http requests.
 * It enables to retrieve list of XHTMLWindows form XHTMLserver.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * QBXHTMLWindowsClient states.
 **/
typedef enum {
    /** unknown state */
    QBXHTMLWindowsClientState_unknown = 0,
    /** request failed because of invalid data */
    QBXHTMLWindowsClientState_invalidData,
    /** request failed because of connection timeout */
    QBXHTMLWindowsClientState_connectionTimeout,
    /** request failed because of connection error */
    QBXHTMLWindowsClientState_connectionError,
    /** request has been cancelled */
    QBXHTMLWindowsClientState_transferCancelled,
    /** got response */
    QBXHTMLWindowsClientState_gotResponse
} QBXHTMLWindowsClientState;

/**
 * QBXHTMLWindowsClient class. Client provides way of communication with XHTMLServer
 * by performing asynchronous http requests.
 **/
typedef struct QBXHTMLWindowsClient_ *QBXHTMLWindowsClient;

/**
 * QBXHTMLWindowsClientRequest class. Each object of this class represents single call to server.
 * Through QBXHTMLWindowsClientRequest caller can check state or cancel request. Each instance of
 * QBXHTMLWindowsClientRequest is related to one caller. When state of request changes, caller will be
 * notified through callback function.
 **/
typedef struct QBXHTMLWindowsClientRequest_ *QBXHTMLWindowsClientRequest;

/**
 * Get runtime type identification object representing
 * QBXHTMLWindowsClientListener interface.
 *
 * @return QBXHTMLWindowsClientListener interface object
 **/
SvInterface
QBXHTMLWindowsClientListener_getInterface(void);

/**
 * QBXHTMLWindowsClient interface.
 **/
typedef struct QBXHTMLWindowsClientListener_ {
    /**
     * Handle new state of request.
     *
     * @param[in]   self_       handle to an object implementing @ref QBXHTMLWindowsClient
     * @param[out]  state       state of request
     **/
    void (*stateChanged)(SvObject self_, QBXHTMLWindowsClientState state);
} *QBXHTMLWindowsClientListener;

/**
 * Create instance of QBXHTMLWindowsClient.
 *
 * @param[in] sslParams         parameters to ssl connection
 * @param[in] smartCardMonitor  smatrCard monitor handle
 * @return QBXHTMLWindowsClient   QBXHTMLWindows client object
 **/
QBXHTMLWindowsClient
QBXHTMLWindowsClientCreate(SvSSLParams sslParams, QBSmartCardMonitor smartCardMonitor);

/**
 * Get windows list from the server.
 *
 * @param[in] self      QBXHTMLWindowsClient handle
 * @param[in] url       url of server with windows list
 * @param[in] listener  listener to be notified about change of the requesty
 *
 * @return QBXHTMLWindowsClientRequest    QBXHTMLWindowsClientRequest handle, NULL in case of error
 **/
QBXHTMLWindowsClientRequest
QBXHTMLWindowsClientGetWindowsList(QBXHTMLWindowsClient self, SvString url, SvObject listener);

/**
 * Get window from the server.
 *
 * @param[in] self      QBXHTMLWindowsClient handle
 * @param[in] url       url of window
 * @param[in] id        id of requested window
 * @param[in] listener  listener to be notified about change of the request
 **/
QBXHTMLWindowsClientRequest
QBXHTMLWindowsClientGetWindow(QBXHTMLWindowsClient self, SvString url, SvString id, SvObject listener);

/**
 * Send window inputs data to the server.
 *
 * @param[in] self          QBXHTMLWindowsClient handle
 * @param[in] url           url of server
 * @param[in] inputsData    inputs data from window
 * @param[in] listener      listener to be notified about change of the request
 * @return                  QBXHTMLWindowsClientRequest handle or @c NULL in case of error
 **/
QBXHTMLWindowsClientRequest
QBXHTMLWindowsClientSendWindowInputsData(QBXHTMLWindowsClient self, SvString url, SvHashTable inputsData, SvObject listener);

/**
 * Stop ongoing request.
 *
 * @param[in] request   QBXHTMLWindowsClientRequest handle
 **/
void
QBXHTMLWindowsClientRequestStop(QBXHTMLWindowsClientRequest request);

/**
 * Get response from request.
 *
 * @param[in] self  QBXHTMLWindowsClientRequest handle
 * @return          Object representing response from server, NULL if no available response
 **/
SvObject
QBXHTMLWindowsClientRequestGetResponse(QBXHTMLWindowsClientRequest self);

#endif
