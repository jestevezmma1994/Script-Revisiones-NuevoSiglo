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

#ifndef QBMPDSUBSETINTERNAL_H_
#define QBMPDSUBSETINTERNAL_H_

/**
 * @file QBMPDSubsetInternal.h
 * @brief Subset element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSubset.h>

/**
 * @addtogroup QBMPDSubset
 * @{
 **/

/**
 * Create Subset element.
 *
 * @param[in]  contains      array of Adaptation Set ids
 * @param[in]  containsCount number of Adaptation Set ids in array @c contains
 * @param[out] errorOut      error info
 * @return                   new instance of Subset element, @c NULL in case of error
 **/
extern QBMPDSubset
QBMPDSubsetCreate(unsigned int *contains,
                  unsigned int containsCount,
                  SvErrorInfo *errorOut);

/**
 * Set identifier for Subset element.
 *
 * @param[in]  self     Subset element handle
 * @param[in]  id       identifier for Subset element
 * @param[out] errorOut error info
 **/
extern void
QBMPDSubsetSetId(QBMPDSubset self,
                 SvString id,
                 SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDSUBSETINTERNAL_H_ */
