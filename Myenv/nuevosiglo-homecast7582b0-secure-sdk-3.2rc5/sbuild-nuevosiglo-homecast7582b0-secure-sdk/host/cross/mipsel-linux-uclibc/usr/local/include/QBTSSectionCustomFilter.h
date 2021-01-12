/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TS_SECTION_CUSTOM_FILTER_H
#define QB_TS_SECTION_CUSTOM_FILTER_H

/**
 * @file QBTSSectionCustomFilter.h QBTSSectionCustomFilter API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

// TODO: ingroup
/**
 * @defgroup QBTSParser QBTSParser classes
 * @{
 *
 * QBTSSectionCustomFilter represents a customizable filter for section parser.
 * It can verify if a section meets certain criteria, by calling a user-defined function
 * with several beginning bytes of that section.
 */

#include <SvFoundation/SvType.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBTSSectionCustomFilter_s;
/**
 * QBTSSectionParser class.
 * @class QBTSSectionParser
 */
typedef struct  QBTSSectionCustomFilter_s  QBTSSectionCustomFilter;

/**
 * A new section should be checked and accepted/rejected.
 * @param[in] section  buffer with the beginnign bytes of section data, at least @a QBTSSectionCustomFilter::len
 * @param[in] payloadLen length of payload
 * @returns @c true iff section should be accepted
 */
typedef bool (QBTSSectionCustomFilterFun) (void* target, const QBTSSectionCustomFilter* filter, const unsigned char* section, int payloadLen);

struct QBTSSectionCustomFilter_s {
  /// Super class
  struct SvObject_ super_;

  /// Minimal section data size to be passed to @a callback.
  /// Must be manually set by filter's owner.
  int len;

  /// Callback parameter.
  void* target;
  /// Callback function.
  QBTSSectionCustomFilterFun* callback;
};


/**
 *  Allocate a new custom section filter.
 *  Created filter is initially empty (lenght=0), so it MUST be filled-out manually.
 *  @returns newly allocated custom section filter, it is a generic object, and so must be released after it's no longer needed
 */
QBTSSectionCustomFilter* QBTSSectionCustomFilterCreate(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 **/

#endif // #ifndef QB_TS_SECTION_CUSTOM_FILTER_H

