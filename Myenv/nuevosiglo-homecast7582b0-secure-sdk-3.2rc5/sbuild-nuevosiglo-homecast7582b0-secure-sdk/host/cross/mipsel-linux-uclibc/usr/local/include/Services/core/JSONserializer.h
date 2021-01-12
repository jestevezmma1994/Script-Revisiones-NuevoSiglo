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

#ifndef QBJSONSERIALIZER_H_
#define QBJSONSERIALIZER_H_

#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>

/**
 * @file JSONserializer.h
 * @brief Service used to write JSON content into a file.
 * The content is not write immediately into storage to prevent too often writes into flash disk.
 * Task to write are stored in queue and when time pass (not too long) then they are written into a disk.
 **/

/**
 * Class for QBJSONSerializer service.
 *
 * @class QBJSONSerializer JSONserializer.h <Services/core/JSONserializer.h>
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 * @implements QBStaticStorageListener
 *
 * @note This class is intended to be used in cooperation with QBServiceRegistry,
 *       use "QBJSONSerializer" key to get it from service registry.
 *
 * @see QBServiceRegistryGetService
 **/
typedef struct QBJSONSerializer_t* QBJSONSerializer;

/**
 * Create JSON serializer service.
 *
 * @return                 the instance of JSON serialize service.
 **/
QBJSONSerializer QBJSONSerializerCreate(void);

/**
 * Add job of writing data to a file into a queue. This function may fail from few reasons, the most important are: we can't serialize data into JSON or QBStaticStorage used internally do not exist.
 *
 * @param[in] self       QBJSONSerializer handler
 * @param[in] data       content to write
 * @param[in] filename   full path to file
 *
 * @return               @c true if write to file was scheduled, otherwise @c false
 **/
bool QBJSONSerializerAddJob(QBJSONSerializer self, SvObject data, SvString filename);

/**
 * Remove scheduled job from queue.
 *
 * @param self       QBJSONSerializer handler
 * @param filename   name of file which we are do not want serialize
 **/
void QBJSONSerializerRemoveJob(QBJSONSerializer self, SvString filename);

void QBJSONSerializerAddImmediateJobWithCallback(QBJSONSerializer self, SvObject data, SvString filename, SvObject listener);

typedef struct QBJSONSerializerListener_t {
    void (*serialized)(SvObject self);
} *QBJSONSerializerListener;

/**
 * Get instance of @ref SvInterface related to QBJSONSerializerListener.
 * @return  Instance of @ref SvInterface.
 **/
SvInterface QBJSONSerializerListener_getInterface(void);

#endif
