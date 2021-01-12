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

#ifndef SV_XMLRPC_CLIENT_LISTENER_H_
#define SV_XMLRPC_CLIENT_LISTENER_H_

/**
 * @file SvXMLRPCClientListener.h
 * @brief XML-RPC client listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCClientListener XML-RPC client listener interface
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * SvXMLRPCClientListener interface.
 **/
typedef const struct SvXMLRPCClientListener_ {
    /**
     * Method called when state of XML-RPC request has changed.
     *
     * @param[in] self      listener handle
     * @param[in] request   XML-RPC request handle
     * @param[in] state     new request state
     **/
    void (*stateChanged)(SvObject self,
                         SvXMLRPCRequest request,
                         SvXMLRPCRequestState state);

    /**
     * Method called when HTTP request is ready to be started.
     *
     * @param[in] self      listener handle
     * @param[in] request   XML-RPC request handle
     **/
    void (*setup)(SvObject self,
                  SvXMLRPCRequest request);
} *SvXMLRPCClientListener;
#define SvXMLRPCClientListener_t SvXMLRPCClientListener_


/**
 * Get runtime type identification object representing
 * interface of XML-RPC client listener.
 *
 * @return XML-RPC client listener interface
 **/
extern SvInterface
SvXMLRPCClientListener_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
