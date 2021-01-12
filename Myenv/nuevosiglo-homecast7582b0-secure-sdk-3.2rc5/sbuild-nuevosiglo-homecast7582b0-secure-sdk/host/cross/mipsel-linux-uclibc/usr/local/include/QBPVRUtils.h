/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PVR_UTILS_H_
#define QB_PVR_UTILS_H_

/**
 * @file QBPVRUtils.h QBPVRUtils set.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPVRTypes.h>
#include <QBContentManager/QBContentCategory.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBPVRUtils PVR utils.
 * @ingroup QBPVRProvider
 * @{
 **/

/**
 * Compare method for objects that may be contained
 * by PVR categories to put them into correct place.
 *
 * If PVR directory is not empty, it's rank is determined by its newest recording,
 * recordings are sorted by start time in descending order,
 * empty directories are on the end and they're sorted by name.
 *
 * @param[in] prv           not used
 * @param[in] objectA       a handle to the first object
 * @param[in] objectB       a handle to the second object
 * @return                  less than @c 0 if @a objectA < @a objectB,
 *                          @c 0 if @a objectA = @a objectB,
 *                          more than @c 0 if @a objectA > @a objectB
 */
int
QBPVRProviderCompareRecordingsAndDirectories(void *prv,
                                             SvObject objectA,
                                             SvObject objectB);

/**
 * Creates new PVR provider category initialized with QBSortedList
 * as its items source.
 *
 * @param[in] id                        id of the category, must be unique in the tree
 * @param[in] name                      name of the category, it must be user-displayable as
                                        title is set to this value.
 * @param[in] parent                    handle to parent category
 * @param[in] provider                  handle to category provider
 * @param[in] addSearch                 should search node be added
 * @param[in] searchProviderTest        search provider test associated data
 * @param[out] errorOut error info
 * @return new content category instance
 **/
QBContentCategory
QBPVRProviderCreateStaticCategory(SvString id,
                                  SvString name,
                                  QBContentCategory parent,
                                  SvObject provider,
                                  bool addSearch,
                                  QBPVRSearchProviderTest searchProviderTest,
                                  SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_PVR_UTILS_H_ */
