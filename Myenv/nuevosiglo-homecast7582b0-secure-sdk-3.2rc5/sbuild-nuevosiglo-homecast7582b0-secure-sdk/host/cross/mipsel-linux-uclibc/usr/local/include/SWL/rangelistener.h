/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SWL_RANGE_LISTENER_H_
#define SWL_RANGE_LISTENER_H_

/**
 * @file SMP/SWL/swl/rangelistener.h
 * @brief Range listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvInterface.h>

/**
 * @defgroup SvRangeListener Range listener interface
 * @ingroup SWL
 * @{
 **/


/**
 * Range listener interface.
 **/
typedef const struct SvRangeListener_t {
    /**
     * Notify about the change of the range.
     *
     * This method notifies about the change of the range.
     * The invariant "lower <= upper" is supposed to be hold.
     *
     * @param[in] self_     range listener handle
     * @param[in] lower     lower bound of the range
     * @param[in] upper     upper bound of the range
     */
    void (*rangeChanged)(SvObject self_,
                         size_t lower, size_t upper);

    /**
     * Notify about the change of the active range.
     *
     * This method notifies about the change of the active range.
     * The invariant "lower <= upper" is supposed to be hold.
     * Moreover the active range is supposed to be bounded by the
     * range &mdash; see SvRangeListener::rangeChanged().
     *
     * @param[in] self_     range listener handle
     * @param[in] lower     lower bound of the active range
     * @param[in] upper     upper bound of the active range
     **/
    void (*activeRangeChanged)(SvObject self_,
                               size_t lower, size_t upper);
} *SvRangeListener;


/**
 * Get runtime type identification object representing
 * range listener interface.
 *
 * @return range listener interface
 **/
extern SvInterface
SvRangeListener_getInterface(void);


/**
 * @}
 **/

#endif
