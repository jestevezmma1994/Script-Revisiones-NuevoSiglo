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

#ifndef QB_FS_READER_FILTER_H_
#define QB_FS_READER_FILTER_H_

/**
 * @file QBFSFilter.h
 * @brief File system filter interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBFSTypes.h>
#include <SvFoundation/SvComparator.h>

/**
 * @defgroup QBFSFilter File system filter interface
 * @ingroup QBFSReader
 * @{
 **/


/**
 * Get runtime type identification object representing
 * file system filter interface.
 **/
extern SvInterface
QBFSFilter_getInterface(void);


/**
 * File system filter interface.
 **/
typedef const struct QBFSFilter_ {
    /// super interface
    struct SvComparator_ super_;

    /**
     * Check if we are interested in a given entry.
     *
     * @param[in] self_     filter handle
     * @param[in] entry     file system entry to test
     * @return              @c false to ignore this entry
     **/
    bool (*test)(SvObject self_,
                 QBFSEntry entry);
} *QBFSFilter;

/**
 * @}
 **/


#endif
