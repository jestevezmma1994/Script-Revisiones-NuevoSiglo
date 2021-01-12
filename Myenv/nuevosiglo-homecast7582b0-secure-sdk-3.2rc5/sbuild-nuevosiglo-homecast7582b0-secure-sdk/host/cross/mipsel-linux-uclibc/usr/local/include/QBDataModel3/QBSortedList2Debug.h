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

#ifndef QB_SORTED_LIST2_DEBUG_H_
#define QB_SORTED_LIST2_DEBUG_H_

/**
 * @file QBSortedList2Debug.h
 * @brief Sorted list 2 class debug API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDataModel3/QBSortedList2.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup QBSortedList2
 * @{
 **/

/**
 * Self validate method for sorted list 2.
 *
 * @memberof QBSortedList2
 *
 * This function will validate if internal data structures in sorted list 2
 * are not broken.
 *
 * @param[in] self                      sorted list handle
 * @return                              @c false if something is wrong, @c true otherwise
 **/
bool
QBSortedList2IsValid(QBSortedList2 self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_SORTED_LIST2_DEBUG_H_ */
