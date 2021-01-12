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

#ifndef QB_RPC_OBJECT_H_
#define QB_RPC_OBJECT_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>


/**
 * QBRPCObject class.
 * QBRPCObject represents basic rpc library object.
 **/
struct QBRPCObject_t {
    struct SvObject_ super_;

    // implemented interfaces container
    SvHashTable implementedInterfaces;
};
typedef struct QBRPCObject_t* QBRPCObject;

/**
 * Get instance of QBRPCObject type.
 *
 * @return                 QBRPCObject type instance
 *
 **/
SvType
QBRPCObject_getType(void);

/**
 * Create rpc object.
 *
 * @return                         @a self, @c NULL in case of error
 **/
QBRPCObject
QBRPCObjectCreate(void);

/**
 * Initialize rpc object.
 *
 * @param[in] self                 rpc object handle
 *
 * @return                         @c 0 if successful, @c -1 in case of error
 **/
int
QBRPCObjectInit(QBRPCObject self);

/**
 * Register an implemented interface.
 *
 * @param[in] self                 rpc object handle
 * @param[in] name                 interface name
 *
 **/
void
QBRPCObjectRegisterImplementedInterface(QBRPCObject self, SvString name);

/**
 * Get list of implemented interfaces.
 *
 * @param[in] self                 rpc object handle
 *
 * @return                         list of implemented interfaces
 *
 **/
SvHashTable
QBRPCObjectListImplementedInterfaces(QBRPCObject self);


#endif // QB_RPC_OBJECT_H_
