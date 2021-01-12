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

#ifndef QB_RPC_INTERFACE_REGISTRY_H_
#define QB_RPC_INTERFACE_REGISTRY_H_

#include <QBRPC/QBRPCInterface.h>
#include <QBRPC/QBRPCInterfaceLock.h>
#include <QBRPC/QBRPCRemoteObjectFactory.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>


/**
 * QBRPCInterfaceRegistry class.
 * QBRPCInterfaceRegistry stores all available implemented interfaces.
 **/
struct QBRPCInterfaceRegistry_t {
    struct SvObject_ super_;
};
typedef struct QBRPCInterfaceRegistry_t* QBRPCInterfaceRegistry;

SvType
QBRPCInterfaceRegistry_getType(void);

/**
 * QBRPCInterfaceRegistry virtual methods table
 **/
typedef const struct QBRPCInterfaceRegistryVTable_ {
    /**
     * SvObject virtual methods table
     **/
    struct SvObjectVTable_ super_;

    /**
     * Register an interface.
     *
     * @param[in] self        interface registry handle
     * @param[in] name        interface name
     * @param[in] factory     interface's remote object factory handle
     *
     * @return                interface handle
     **/
    QBRPCInterface (*registerInterface)(QBRPCInterfaceRegistry self, SvString name, QBRPCRemoteObjectFactory factory);

    /**
     * Get an interface.
     *
     * @param[in] self        interface registry handle
     * @param[in] name        interface name
     *
     * @return                interface handle
     **/
    QBRPCInterface (*getInterface)(QBRPCInterfaceRegistry self, SvString name);

    /**
     * Get interface's remote object factory.
     *
     * @param[in] self        interface registry handle
     * @param[in] name        interface name
     *
     * @return                remote object factory handle
     **/
    QBRPCRemoteObjectFactory (*getFactory)(QBRPCInterfaceRegistry self, SvString name);

    /**
     * Register interface lock.
     *
     * @param[in] self                 interface registry handle
     * @param[in] id                   lock id
     * @param[in] lockTimeoutMs        period after which lock is timed out
     *
     * @return                         @a interface lock handle, @c NULL in case of error
     **/
    QBRPCInterfaceLock (*registerInterfaceLock)(QBRPCInterfaceRegistry self, SvObject id, long long lockTimeoutMs);

    /**
     * Remove interface lock.
     *
     * @param[in] self                 interface registry handle
     * @param[in] id                   lock id
     *
     **/
    void (*removeInterfaceLock)(QBRPCInterfaceRegistry self, SvObject id);

    /**
     * Get interface lock.
     *
     * @param[in] self                 interface registry handle
     * @param[in] id                   lock id
     *
     * @return                         interface lock handle
     **/
    QBRPCInterfaceLock (*getInterfaceLock)(QBRPCInterfaceRegistry self, SvObject id);
} *QBRPCInterfaceRegistryVTable;

/**
 * Get interface registry instance.
 *
 * @return            interface registry handle
 **/
QBRPCInterfaceRegistry
QBRPCInterfaceRegistryGetInstance(void);


#endif // QB_RPC_INTERFACE_REGISTRY_H_
