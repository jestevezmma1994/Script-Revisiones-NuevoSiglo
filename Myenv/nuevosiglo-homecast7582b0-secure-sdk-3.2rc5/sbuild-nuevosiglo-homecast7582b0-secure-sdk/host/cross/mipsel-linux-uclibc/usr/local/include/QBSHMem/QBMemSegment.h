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

#ifndef QB_MEM_SEGMENT_H_
#define QB_MEM_SEGMENT_H_

/**
 * @file QBMemSegment.h
 * @brief Memory segment interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURI.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBMemSegment Memory segment interface
 * @ingroup QBSHMem
 * @{
 *
 * QBMemSegment is an abstract interface describing memory segment.
 *
 * @image html QBMemSegment.png
 **/

/**
 * Get runtime type identification object representing
 * memory segment interface.
 *
 * @return memory segment interface
 **/
extern SvInterface
QBMemSegment_getInterface(void);


/**
 * QBMemSegment interface.
 **/
typedef const struct QBMemSegment_ {
    /**
     * Initialize memory segment object
     * by allocating new segment from the operating system.
     *
     * This method will initialize the segment object, allocating
     * required area of memory.
     *
     * @param[in] self      a handle to an object implementing QBMemSegment
     * @param[in] length    length of the segment, in bytes (it will be rounded
     *                      up to a multiple of page size)
     * @param[in] rights    UNIX access rights to the segment
     * @param[out] errorOut error info
     * @return              @a self_ or @c NULL in case of error
     **/
    SvObject (*allocate)(SvObject self_,
                         size_t length,
                         unsigned int rights,
                         SvErrorInfo *errorOut);

    /**
     * Initialize memory segment object by attaching
     * to the existing segment.
     *
     * @param[in] self      a handle to an object implementing QBMemSegment
     * @param[in] URI       shared memory segment URI
     * @param[out] errorOut error info
     * @return              @a self_ or @c NULL in case of error
     **/
    SvObject (*attach)(SvObject self_,
                       SvURI URI,
                       SvErrorInfo *errorOut);

    /**
     * Return segment's URI.
     *
     * @param[in] self_     a handle to an object implementing QBMemSegment
     * @return              URI of a segment, @c NULL on error
     **/
    SvURI (*getURI)(SvObject self_);

    /**
     * Return segment's length.
     *
     * @param[in] self_     a handle to an object implementing QBMemSegment
     * @return              segment's length in bytes, (size_t) -1 on error
     **/
    size_t (*getLength)(SvObject self_);

    /**
     * Return segment's base address.
     *
     * @param[in] self_     a handle to an object implementing QBMemSegment
     * @return              segment's base address or @c NULL in case of error
     **/
    void *(*getBase)(SvObject self_);

    /**
     * Set segment's UNIX access rights.
     *
     * @param[in] self_     a handle to an object implementing QBMemSegment
     * @param[in] rights    UNIX access rights
     * @param[out] errorOut error info
     **/
    void (*setRights)(SvObject self_,
                      unsigned int rights,
                      SvErrorInfo *errorOut);

    /**
     * Remove segment from its namespace.
     *
     * This method removes segment's name or ID from the system namespace.
     * There is no way to attach this segment afterwards, but it still
     * exists until all processes detach from it. Calling this method
     * after all possible users have attached a segment is a good idea,
     * because it will cause the operating system to destroy the segment
     * automatically when it is no longer used.
     *
     * @param[in] self      a handle to an object implementing QBMemSegment
     * @param[out] errorOut error info
     **/
    void (*unlink)(SvObject self_,
                   SvErrorInfo *errorOut);
} *QBMemSegment;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
