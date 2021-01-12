/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_XMLRPC_REQUEST_STATS_H_
#define SV_XMLRPC_REQUEST_STATS_H_

/**
 * @file SvXMLRPCRequestStats.h XML-RPC statistics API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>

typedef struct SvXMLRPCRequestsCounter_ *SvXMLRPCRequestsCounter;


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCRequestStats XML-RPC statistics
 * @ingroup SvXMLRPCClient
 * @{
 *
 * This simple module collects statistics of called XML-RPC requests.
 **/

/**
 * Get collection of all requests counters.
 *
 * @return collection of requests counters or @c NULL if didn't send any data or we use secure version.
 */
extern SvHashTable
SvXMLRPCRequestStatsGetRequestsCounters(void);

/**
 * Clean all requests counters.
 **/
extern void
SvXMLRPCRequestStatsCleanRequestsCounters(void);


/**
 * Get value of counter.
 *
 * @param[in] counter    requests counter
 * @return value stored in counter
 **/
extern long int
SvXMLRPCRequestsCounterGetValue(SvXMLRPCRequestsCounter counter);

/**
 * Get type name of counter.
 *
 * @param[in] counter    requests counter
 * @return name of the type or @c NULL in case of error
 **/
SvString
SvXMLRPCRequestsCounterGetType(SvXMLRPCRequestsCounter counter);
/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
