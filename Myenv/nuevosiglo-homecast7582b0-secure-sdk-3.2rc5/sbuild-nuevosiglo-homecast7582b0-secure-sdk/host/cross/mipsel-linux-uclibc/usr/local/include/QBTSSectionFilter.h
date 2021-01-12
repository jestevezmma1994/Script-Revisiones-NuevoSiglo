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

#ifndef QB_TS_SECTION_FILTER_H
#define QB_TS_SECTION_FILTER_H

/**
 * @file QBTSSectionFilter.h QBTSSectionFilter API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

// TODO: ingroup
/**
 * @defgroup QBTSParser QBTSParser classes
 * @{
 *
 * QBTSSectionFilter represents a simple filter for section parser.
 * It can verify if a section meets certain criteria, by comparing section's contents with filter's data and mask.
 */

#include <SvFoundation/SvCoreTypes.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Maximal length of data that QBTSSectionFilter can check.
#define QBTSSectionFilterMaxSize  16

struct QBTSSectionFilter_s {
  /// super class
  struct SvObject_ super_;

  /// Maximum length of @a data and @a mask to be checked.
  int len;
  /// Data to be compared with section's data.
  unsigned char data[QBTSSectionFilterMaxSize];
  /// Mask telling which bits of @a data should be used for comparing.
  unsigned char mask[QBTSSectionFilterMaxSize];
};
/**
 * QBTSSectionParser class.
 * @class QBTSSectionParser
 */
typedef struct QBTSSectionFilter_s  QBTSSectionFilter;

/**
 *  Allocate a new section filter.
 *  Created filter is initially empty (lenght=0), so it needs to be filled-out manually, or with helper functions.
 *  @returns newly allocated section filter, it is a generic object, and so must be released after it's no longer needed
 */
QBTSSectionFilter* QBTSSectionFilterCreate(void);

/**
 *  Helper function that sets data/mask to filter by service id in EIT sections.
 *  @param[in] filter  section filter
 *  @param[in] serviceID  service ID to be filtering for (works only for EIT sections)
 */
void QBTSSectionFilterSetEITServiceID(QBTSSectionFilter* filter, uint16_t serviceID);

/**
 *  Helper function that sets data/mask to filter by table-id in any section.
 *  @param[in] filter  section filter
 *  @param[in] tableID  table-id to be filtering for
 */
void QBTSSectionFilterSetTableID(QBTSSectionFilter* filter, uint8_t tableID);

/**
 *  Helper function that sets data/mask to filter by table-id-extension in any section.
 *  @param[in] filter  section filter
 *  @param[in] tableIDExt table-id-extension to be filtering for
 */
void QBTSSectionFilterSetTableIDExt(QBTSSectionFilter* filter, uint16_t tableIDExt);

/**
 *  Test if @a section meets criteria of @a filter.
 *  @param[in] filter  section filter
 *  @param[in] section  beginning bytes of a section to be checked, this buffer must be at least the length of the filter
 *  @returns  @c true iff @a section meets criteria of @a filter
 */
bool QBTSSectionFilterTestSection(const QBTSSectionFilter* filter, const unsigned char* section);

#ifdef __cplusplus
}
#endif

/**
 * @}
 **/

#endif // #ifndef QB_TS_SECTION_FILTER_H
