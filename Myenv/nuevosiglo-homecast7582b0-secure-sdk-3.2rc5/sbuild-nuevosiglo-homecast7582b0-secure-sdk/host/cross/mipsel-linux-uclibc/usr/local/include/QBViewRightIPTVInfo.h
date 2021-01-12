/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_VIEWRIGHT_IPTV_INFO_COMMON_H_
#define QB_VIEWRIGHT_IPTV_INFO_COMMON_H_

/**
 * @file QBViewRightIPTVInfo.h QBViewRightIPTVInfo API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBCASInfo.h>
#include <SvFoundation/SvString.h>
#include <QBViewRightIPTVInterface.h>

/**
 * @defgroup QBViewRightIPTVInfo QBViewRightIPTVInfo
 * @ingroup QBViewRightIPTV
 * @{
 **/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief QBViewRightIPTVInfo class, which contains ViewRightIPTV CAS specific informations
 */
struct QBViewRightIPTVInfo_s
{
    struct QBCASInfo_t base;    /**< Super class of QBViewRightIPTVInfo, which is QBCASInfo */
    QBViewRightIPTVState state; /**< Current state of QBViewRightIPTV */
    int errorCode;              /**< If state is indicates error, this field contains error code from ViewRight library */
};

/**
 * @brief QBViewRightIPTVInfo type definition
 */
typedef struct QBViewRightIPTVInfo_s* QBViewRightIPTVInfo;

/**
 * @brief Constructor of QBViewRightIPTVInfo. It creates instance clear instance
 * @param[out] errorOut errorinfo instance
 * @return instance of QBViewRightIPTVInfo
 */
QBViewRightIPTVInfo QBViewRightIPTVInfoCreate(SvErrorInfo *errorOut);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
