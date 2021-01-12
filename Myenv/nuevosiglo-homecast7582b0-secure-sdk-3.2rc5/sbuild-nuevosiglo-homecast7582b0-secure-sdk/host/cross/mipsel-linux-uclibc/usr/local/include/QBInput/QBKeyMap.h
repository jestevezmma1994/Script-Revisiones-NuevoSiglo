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

#ifndef QB_KEYMAP_H_
#define QB_KEYMAP_H_

/**
 * @file QBKeyMap.h
 * @brief Keyboard map class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <unistd.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * @defgroup QBKeyMap Keyboard map class
 * @ingroup QBInput
 * @{
 **/

/**
 * Single entry of a keyboard map.
 **/
struct QBKeyMapEntry_ {
    /// raw key code reported by the input device
    uint32_t srcCode;
    /// a set of QBKEY_* key codes after mapping
    struct {
        /// output key code when no modifiers are active
        uint32_t unmodified;
        /// output key code when Shift is pressed or CapsLock is active
        uint32_t withShift;
        /// output key code when Meta (Alt) is pressed
        uint32_t withMeta;
        /// output key code when both Shift and Meta (Alt) are pressed
        uint32_t withShiftMeta;
    } destCode;
};

/**
 * Keyboard map class.
 * @class QBKeyMap
 * @extends SvObject
 **/
typedef struct QBKeyMap_ *QBKeyMap;


/**
 * Get runtime type identification object representing
 * keyboard map class.
 *
 * @return keyboard map class
 **/
extern SvType QBKeyMap_getType(void);

/**
 * Get number of entries in a keyboard map.
 *
 * @memberof QBKeyMap
 *
 * @param[in] self      keyboard map handle
 * @return              number of entries, @c -1 in case of error
 **/
extern ssize_t QBKeyMapGetEntriesCount(QBKeyMap self);

/**
 * Get keyboard map entries.
 *
 * @memberof QBKeyMap
 *
 * @param[in] self      keyboard map handle
 * @return              keyboard map entries, @c NULL in case of error
 **/
extern const struct QBKeyMapEntry_ *QBKeyMapGetEntries(QBKeyMap self);

/**
 * @}
 **/


#endif
