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

#ifndef QB_AUTOREPEAT_FILTER_H_
#define QB_AUTOREPEAT_FILTER_H_

/**
 * @file QBAutoRepeatFilter.h
 * @brief Auto repeat input filter class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBAutoRepeatFilter Auto repeat input filter class
 * @ingroup QBInputEventFilters
 * @{
 **/

/**
 * Auto repeat input filter class.
 * @class QBAutoRepeatFilter
 * @extends SvObject
 **/
typedef struct QBAutoRepeatFilter_ *QBAutoRepeatFilter;


/**
 * Get runtime type identification object representing
 * auto repeat input filter class.
 *
 * @return auto repeat input filter class
 **/
extern SvType QBAutoRepeatFilter_getType(void);

/**
 * Initialize auto repeat input filter object.
 *
 * This method initializes auto repeat filter. The filter will not
 * work until configured using QBAutoRepeatFilterSetup() method.
 *
 * @memberof QBAutoRepeatFilter
 *
 * @param[in] self      auto repeat input filter handle
 * @param[out] errorOut error info
 * @return              @a self, @c NULL in case of error
 **/
extern QBAutoRepeatFilter QBAutoRepeatFilterInit(QBAutoRepeatFilter self,
                                                 SvErrorInfo *errorOut);

/**
 * Set auto repeat filter parameters.
 *
 * @memberof QBAutoRepeatFilter
 *
 * @param[in] self      auto repeat input filter handle
 * @param[in] delay     delay in microseconds after which first
 *                      repeated event will be reported (must be >@c 0)
 * @param[in] period    time period in microseconds between repeated
 *                      events (pass @c 0 to disable auto repeat)
 * @param[out] errorOut error info
 **/
extern void QBAutoRepeatFilterSetup(QBAutoRepeatFilter self,
                                    unsigned int delay, unsigned int period,
                                    SvErrorInfo *errorOut);


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
