/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_OSK_MAP_H_
#define QB_OSK_MAP_H_

/**
 * @file QBOSKMap.h
 * @brief On Screen Keyboard map class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvArray.h>
#include <QBOSK/QBOSKTypes.h>
#include <QBOSK/QBOSKKey.h>
#include <QBOSK/QBOSKNumericKeyboard.h>

/**
 * @defgroup QBOSKMap On Screen Keyboard map class
 * @ingroup QBOSKCore
 * @{
 **/

/**
 * Get runtime type identification object representing OSK map class.
 *
 * @relates QBOSKMap
 *
 * @return QBOSKMap type identification object
 **/
extern SvType
QBOSKMap_getType(void);

/**
 * Create new OSK map.
 *
 * @memberof QBOSKMap
 *
 * @param[in] defaultLayoutID identifier of the default layout
 * @param[in] layouts   an array of unique keyboard layout identifiers
 *                      (each identifier is an SvString)
 * @param[out] errorOut error info
 * @return              created map, @c NULL in case of error
 **/
extern QBOSKMap
QBOSKMapCreate(SvString defaultLayoutID,
               SvImmutableArray layouts,
               SvErrorInfo *errorOut);

/**
 * Get default layout's identifier.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @return              @c NULL in case of no default layout
 */
extern SvString
QBOSKMapGetDefaultLayoutID(QBOSKMap self);

/**
 * Get array of keyboard layout identifiers.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @return              handle to an array of keyboard layout identifiers,
 *                      @c NULL in case of error
 **/
extern SvImmutableArray
QBOSKMapGetLayouts(QBOSKMap self);

/**
 * Find index of a keyboard layout.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @param[in] layoutID  OSK layout ID
 * @return              index of the layout with given ID, @c -1 if not found
 **/
extern int
QBOSKMapFindLayout(QBOSKMap self,
                   SvString layoutID);

/**
 * Add new OSK key or replace existing one.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @param[in] key       OSK key handle
 * @param[out] errorOut error info
 **/
extern void
QBOSKMapAddKey(QBOSKMap self,
               QBOSKKey key,
               SvErrorInfo *errorOut);

/**
 * Find OSK key with given identifier.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @param[in] keyID     unique key identifier
 * @return              handle to an OSK key, @c NULL in case of error
 **/
extern QBOSKKey
QBOSKMapFindKey(QBOSKMap self,
                SvString keyID);

/**
 * Find OSK key with given value.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @param[in] layout    layout identifier
 * @param[in] code      key code
 * @return              handle to an OSK key, @c NULL in case of error
 **/
extern QBOSKKey
QBOSKMapFindKeyByValue(QBOSKMap self,
                       unsigned int layout,
                       SvString code);

/**
 * Find OSK key with given type.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @param[in] layout    layout identifier
 * @param[in] type      key type
 * @return              handle to an OSK key, @c NULL in case of error
 **/
extern QBOSKKey
QBOSKMapFindKeyByType(QBOSKMap self,
                      unsigned int layout,
                      QBOSKKeyType type);

/**
 * Find OSK key in all layouts with given value.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @param[in] code      key code
 * @param[out] layout   layout identifier, where key found
 * @return              handle to an OSK key, @c NULL in case of error
 **/
QBOSKKey
QBOSKMapFindKeyByValueInAllLayouts(QBOSKMap self,
                                   SvString code,
                                   int *layout);

/**
 * Get iterator over all OSK keys.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @return              OSK keys iterator
 **/
extern SvIterator
QBOSKMapGetKeysIterator(QBOSKMap self);

/**
 * Get an array of OSK keys visible in given layout.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @param[in] layoutID  layout identifier
 * @param[out] errorOut error info
 * @return              handle to an array of OSK keys
 **/
extern SvArray
QBOSKMapGetKeysForLayout(QBOSKMap self,
                         SvString layoutID,
                         SvErrorInfo *errorOut);

/**
 * Set OSK key for initial focus.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @param[in] key       handle to an OSK key
 * @param[out] errorOut error info
 **/
extern void
QBOSKMapSetInitialKey(QBOSKMap self,
        QBOSKKey key,
        SvErrorInfo *errorOut);

/**
 * Get OSK key for initial focus.
 *
 * @memberof QBOSKMap
 *
 * @param[in] self      OSK map handle
 * @return              handle to an OSK key, @c NULL in case of error
 **/
extern QBOSKKey
QBOSKMapGetInitialKey(QBOSKMap self);

/**
 * Get language of given layout, if specified.
 *
 * @memberof QBOSKMap
 * @since 1.0.5
 *
 * @param[in] self      OSK map handle
 * @param[in] layoutID  layout identifier
 * @param[out] errorOut error info
 * @return              language of the layout with given @a layoutID,
 *                      @c NULL in case of error or when language for this
 *                      layout is not defined
 **/
extern SvString
QBOSKMapGetLayoutLanguage(QBOSKMap self,
                          SvString layoutID,
                          SvErrorInfo *errorOut);

/**
 * @param[in] self            OSK map handle
 * @param[in] numericKeyboard @ref QBOSKNumericKeyboard handle
 * @param[in] layoutID        layout identifier
 **/
extern void
QBOSKMapSetNumericKeyboardMap(QBOSKMap self,
                              QBOSKNumericKeyboard numericKeyboard,
                              SvString layoutID);

/**
 * @}
 **/

#endif
