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

#ifndef QB_LONG_KEYPRESS_FILTER_H_
#define QB_LONG_KEYPRESS_FILTER_H_

/**
 * @file QBLongKeyPressFilter.h
 * @brief Long key press input filter class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBLongKeyPressFilter Long key press input filter class
 * @ingroup QBInputEventFilters
 *
 * Long key press input filter changes the value of input code
 * in events of ::QBInputEventType_keyTyped type,
 * depending on how long the key has been pressed.
 *
 * In the following example an instance of such a filter is
 * installed, that will replace ::QBKEY_ENTER events with
 * ::QBKEY_PLAY events when ENTER key is held for at least 5 seconds:
 *
@code
  static const QBLongKeyPressMapping map[] = {
      {  QBKEY_ENTER,      QBKEY_PLAY,       5000 * 1000 },
      {  0,                0,                0           }
  };
  QBLongKeyPressFilter filter;
  filter = (QBLongKeyPressFilter) SvTypeAllocateInstance(QBLongKeyPressFilter_getType(), NULL);
  QBLongKeyPressFilterInit(filter, map, NULL);
  QBInputServiceAddGlobalFilter((SvObject) filter, NULL, NULL);
  SVRELEASE(filter);
@endcode
 *
 * @{
 **/

/**
 * Long key press input filter class.
 * @class QBLongKeyPressFilter
 * @extends SvObject
 **/
typedef struct QBLongKeyPressFilter_ *QBLongKeyPressFilter;

/**
 * Long key press mapping.
 **/
typedef struct {
   /// original (short press) key code
   unsigned int srcCode;
   /// converted (long press) key code
   unsigned int longCode;
   /// min press time (in microseconds) for short code to be converted to long code
   unsigned int minTime;
} QBLongKeyPressMapping;

/**
 * Extended long key press mapping.
 **/
typedef struct {
   /// original (short press) key code
   unsigned int srcCode;
   /// converted (short press) key code
   unsigned int shortCode;
   /// converted (long press) key code
   unsigned int longCode;
   /// min press time (in microseconds) for short code to be converted to long code
   unsigned int minTime;
   /**
    * @c true to emit three (::QBInputEventType_keyPressed, ::QBInputEventType_keyTyped,
    * ::QBInputEventType_keyReleased) events, @c false to only emit ::QBInputEventType_keyTyped
    **/
   bool generateFullEvent;
} QBLongKeyPressExtendedMapping;


/**
 * Get runtime type identification object representing
 * long key press input filter class.
 *
 * @return long key press input filter class
 **/
extern SvType QBLongKeyPressFilter_getType(void);

/**
 * Initialize long key press input filter object.
 *
 * @memberof QBLongKeyPressFilter
 *
 * @param[in] self      long key press input filter handle
 * @param[in] map       an array of key mappings, terminated by empty mapping
 *                      (containing only zeros)
 * @param[out] errorOut error info
 * @return              @a self, @c NULL in case of error
 **/
extern QBLongKeyPressFilter QBLongKeyPressFilterInit(QBLongKeyPressFilter self,
                                                     const QBLongKeyPressMapping *map,
                                                     SvErrorInfo *errorOut);

/**
 * Initialize long key press input filter object with extended mapping.
 *
 * @memberof QBLongKeyPressFilter
 *
 * @param[in] self      long key press input filter handle
 * @param[in] map       an array of extended key mappings, terminated by empty mapping
 *                      (containing only zeros)
 * @param[out] errorOut error info
 * @return              @a self, @c NULL in case of error
 **/
extern QBLongKeyPressFilter QBLongKeyPressFilterInitExtended(QBLongKeyPressFilter self,
                                                             const QBLongKeyPressExtendedMapping *map,
                                                             SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
