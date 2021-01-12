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

#ifndef QB_INPUT_UTILS_H_
#define QB_INPUT_UTILS_H_

/**
 * @file QBInputUtils.h
 * @brief Input device library utility functions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvHashTable.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBInputUtils Input device library utility functions
 * @ingroup QBInputCore
 * @{
 **/

/**
 * Setup default auto repeat filter.
 *
 * This method will configure default auto repeat filter instance
 * installed in common filters chain of the input service. If such
 * filter is not present, it will be created.
 * When @a period == 0, auto repeat filter will be removed.
 *
 * @param[in] delay     delay in microseconds after which first
 *                      repeated event will be reported (must be >0)
 * @param[in] period    time period in microseconds between repeated
 *                      events (pass 0 to disable auto repeat)
 * @param[out] errorOut error info
 **/
extern void QBInputSetupAutoRepeat(unsigned int delay, unsigned int period,
                                   SvErrorInfo *errorOut);

/**
 * Load contents of the file that maps input event codes to symbolic names.
 *
 * @param[in,out] names2codes handle to a hash table that will be filled with
 *                      name (SvString) -> code (SvValue<int>) map;
 *                      pass @c NULL if you don't care
 * @return              newly allocated hash table filled with
 *                      code (SvValue<int>) to name (SvString) map
 *                      (caller is responsible for releasing it)
 *                      or @c NULL in case of error
 **/
extern SvHashTable QBInputLoadCodesMap(SvHashTable names2codes);


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
