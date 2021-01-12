/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RANGE_REQUEST_H_
#define QB_RANGE_REQUEST_H_

/**
 * @file QBRangeRequest.h
 * @brief Range request interface API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBRangeRequest Range request interface
 * @ingroup QBDataProvider
 * @{
 *
 * An interface for data requests that were created for a range.
 **/

/**
 * Range request interface.
 **/
typedef const struct QBRangeRequest_ {
    /**
     * Get range index for which this request was created.
     *
     * @param[in] self  request handle
     * @return          range index
     **/
    size_t (*getRangeIndex)(SvObject self);
} *QBRangeRequest;

/**
 * Get runtime type identification object representing QBRangeRequest interface.
 *
 * @return QBMainMenu interface object
 **/
extern SvInterface
QBRangeRequest_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif /* QB_RANGE_REQUEST_H_ */
