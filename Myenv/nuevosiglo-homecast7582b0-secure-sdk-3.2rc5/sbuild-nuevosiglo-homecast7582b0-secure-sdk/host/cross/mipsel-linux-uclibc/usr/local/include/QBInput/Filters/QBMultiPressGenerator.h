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

#ifndef MULTI_PRESS_GENERATOR_H_
#define MULTI_PRESS_GENERATOR_H_

/**
 * @file QBMultiPressGenerator.h
 * @brief Multi input press generator class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBMultiPressGenerator Multi press generator class
 * @ingroup QBInputEventFilters
 *
 * Multi press input generator can translate input key events
 * with type ::QBInputEventType_keyTyped into a set of other
 * input key events. This can have zero, one or many elements.
 *
 * In the following example an instance of such a filter is
 * installed, that will replace each ::QBKEY_ENTER event with
 * ::QBKEY_PLAY event followed by ::QBKEY_INFO event:
 *
@code
  static const uint32_t outTab = {
      QBKEY_PLAY, QBKEY_INFO, 0
  };
  static const QBMultiPressMapping map[] = {
      { QBKEY_ENTER,  true,   outTab },
      { 0,            false,  0      }
  };
  QBMultiPressGenerator generator;
  generator = (QBMultiPressGenerator) SvTypeAllocateInstance(QBMultiPressGenerator_getType(), NULL);
  QBMultiPressGeneratorInit(generator, map, NULL);
  QBInputServiceAddGlobalFilter((SvObject) generator, NULL, NULL);
  SVRELEASE(generator);
@endcode
 *
 * @{
 **/

/**
 * Multi key press mapping.
 **/
typedef struct {
    /// original input code
    uint32_t srcCode;
    /// is this rule enabled
    bool enabled;
    /// sequence on output, terminated with @c 0
    const uint32_t *outputSequence;
} QBMultiPressMapping;

/**
 * Multi key press generator class.
 * @class QBMultiPressGenerator
 * @extends SvObject
 **/
typedef struct QBMultiPressGenerator_ *QBMultiPressGenerator;


/**
 * Get runtime type identification object representing
 * multi key press generator filter class.
 *
 * @return mutli key press generator class
 **/
extern SvType
QBMultiPressGenerator_getType(void);

/**
 * Initialize multi press generator with a set of translation rules.
 *
 * @memberof QBMultiPressGenerator
 *
 * @param[in] self          multi press generator handle
 * @param[in] map           array of mappings, terminated with zero'ed element
 * @param[out] errorOut     error info
 * @return                  @a self, @c NULL in case of errror
 **/
extern QBMultiPressGenerator
QBMultiPressGeneratorInit(QBMultiPressGenerator self,
                          QBMultiPressMapping map[],
                          SvErrorInfo *errorOut);

/**
 * Turn off/on mapping in filter.
 *
 * @memberof QBMultiPressGenerator
 *
 * @param[in] self          multi press generator handle
 * @param[in] idx           index of rule to modify
 * @param[in] enabled       desired state of mapping
 * @return                  @c 0, @c -1 in case of error
 **/
extern int
QBMultiPressGeneratorEnableMapping(QBMultiPressGenerator self,
                                   size_t idx, bool enabled);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
