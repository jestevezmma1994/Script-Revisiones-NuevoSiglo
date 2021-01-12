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

#ifndef QBMPDSEGMENTTEMPLATE_H_
#define QBMPDSEGMENTTEMPLATE_H_

/**
 * @file QBMPDSegmentTemplate.h
 * @brief Segment Template element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

/**
 * @defgroup QBMPDSegmentTemplate Segment Template element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Segment Template element class.
 * @class QBMPDSegmentTemplate
 * @extends QBMPDMultipleSegmentBase
 **/
typedef struct QBMPDSegmentTemplate_ *QBMPDSegmentTemplate;

/**
 * Get runtime type identification object representing
 * type of Segment Template element class.
 *
 * @return Segment Template element class
 **/
extern SvType
QBMPDSegmentTemplate_getType(void);

/**
 * Get media of Segment Template element.
 *
 * @param[in] self Segment Template element handle
 * @return         template to create Media Segment List, @c NULL in case of error
 **/
extern SvString
QBMPDSegmentTemplateGetMedia(QBMPDSegmentTemplate self);

/**
 * Get index of Segment Template element.
 *
 * @param[in] self Segment Template element handle
 * @return         template to create Index Segment List, @c NULL in case of error
 **/
extern SvString
QBMPDSegmentTemplateGetIndex(QBMPDSegmentTemplate self);

/**
 * Get initialization of Segment Template element.
 *
 * @param[in] self Segment Template element handle
 * @return         template to create Initialization Segment, @c NULL in case of error
 **/
extern SvString
QBMPDSegmentTemplateGetInitialization(QBMPDSegmentTemplate self);

/**
 * Get bitstream switching of Segment Template element.
 *
 * @param[in] self Segment Template element handle
 * @return         template to create Bitstream Switching Segment, @c NULL in case of error
 **/
extern SvString
QBMPDSegmentTemplateGetBitstreamSwitching(QBMPDSegmentTemplate self);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTTEMPLATE_H_ */
