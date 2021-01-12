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

#ifndef QBIPSTREAMCLIENT_QBIPSTREAMFILTER188_H_
#define QBIPSTREAMCLIENT_QBIPSTREAMFILTER188_H_

/**
 * @file QBIPStreamFilter188.h
 * @brief Data segmentation filter
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBIPStreamFilter188 Segmentation filter
 * @ingroup QBHLSManager
 * @{
 * Data segmentation filter
 **/

/**
 * QBIPStreamFilter188
 * @class QBIPStreamFilter188
 * @extends SvObject
 */
typedef struct QBIPStreamFilter188_ *QBIPStreamFilter188;

/**
 * Create new filter.
 *
 * Filter is transparent for data, but it forwards data in buffers with size as multiply of 188.
 *
 * @param[out] errorOut errorInfo
 * @return     filter handle or @c NULL on error
 */
QBIPStreamFilter188
QBIPStreamFilter188Create(SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBIPSTREAMCLIENT_QBIPSTREAMFILTER188_H_ */
