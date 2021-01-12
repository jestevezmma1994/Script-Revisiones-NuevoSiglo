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

#ifndef QBVIEWRIGHT_IPTV_COPYCONTROL_H_
#define QBVIEWRIGHT_IPTV_COPYCONTROL_H_

/**
 * @file QBViewRightIPTVCopyControl.h QBViewRightIPTVCopyControl API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewRightIPTVCopyControl QBViewRightIPTVCopyControl
 * @ingroup QBViewRightIPTV
 * @{
 **/

/**
 * @brief Defines EMI (CGMS/A) mode (see PROD-333 ViewRight STB for IPTV: API Reference, page 53)
 */
typedef enum QBViewRightIPTVCopyControlEMI_e {
    QBViewRightIPTVCopyControlEMI_CopyingPermited,              /**< Copying is not restricted */
    QBViewRightIPTVCopyControlEMI_NoFurtherCopying,             /**< One generation of copies has already been made; no further copying is allowed */
    QBViewRightIPTVCopyControlEMI_OneGenerationCopyPermited,    /**< One generation copy is permitted */
    QBViewRightIPTVCopyControlEMI_CopyingProhibited             /**< Copying is prohibited */
} QBViewRightIPTVCopyControlEMI;

/**
 * @brief Defines APS (Analog Protection System) mode (see PROD-333 ViewRight STB for IPTV: API Reference, page 53)
 */
typedef enum QBViewRightIPTVCopyControlAPS_e {
    QBViewRightIPTVCopyControlAPS_Disabled,                     /**< Copy protection encoding off */
    QBViewRightIPTVCopyControlAPS_AGConSplitBurstOff,           /**< AGC process on, split burst off */
    QBViewRightIPTVCopyControlAPS_AGConSplitBurst2,             /**< AGC process on, two-line split burst on */
    QBViewRightIPTVCopyControlAPS_AGConSplitBurst4              /**< AGC process on, four-line split burst on */
} QBViewRightIPTVCopyControlAPS;

/**
 * @brief Class declaration of QBViewRightIPTVCopyControl
 * For more information about Copy Control see PROD-333 ViewRight STB for IPTV: API Reference (starting at page 50)
 */
typedef struct QBViewRightIPTVCopyControl_s *QBViewRightIPTVCopyControl;

/**
 * @brief Constructor of QBViewRightIPTVCopyControl. It parses Copy Control Message received from VMX library
 *
 * @param[in] message Copy Control Message received from library
 * @param[in] len length of message parameter
 * @return instance of QBViewRightIPTVCopyControl in case of success or NULL in case of error
 */
QBViewRightIPTVCopyControl QBViewRightIPTVCopyControlCreate(const char *message, size_t len);

/**
 * @brief Gets EMI mode
 *
 * @param[in] self instance of QBViewRightIPTVCopyControl object
 * @return current EMI mode held by object
 */
QBViewRightIPTVCopyControlEMI QBViewRightIPTVCopyControlGetEMImode(QBViewRightIPTVCopyControl self);

/**
 * @brief Gets APS mode
 *
 * @param[in] self instance of QBViewRightIPTVCopyControl object
 * @return current APS mode held by object
 */
QBViewRightIPTVCopyControlAPS QBViewRightIPTVCopyControlGetAPSmode(QBViewRightIPTVCopyControl self);

/**
 * @brief Gets DC flag
 *
 * @param[in] self instance of QBViewRightIPTVCopyControl object
 * @return current Dwight-Cavendish process flag held by object
 */
bool QBViewRightIPTVCopyControlGetDC(QBViewRightIPTVCopyControl self);

/**
 * @brief Gets HDCP mode
 *
 * @param[in] self instance of QBViewRightIPTVCopyControl object
 * @return current HDCP flag held by object
 */
bool QBViewRightIPTVCopyControlGetHDCP(QBViewRightIPTVCopyControl self);

/**
 * @brief Gets DOT (Digital Only Trigger) mode; if DOT is enabled then all analog outputs shall be disabled
 *
 * @param[in] self instance of QBViewRightIPTVCopyControl object
 * @return current DOT flag mode held by object
 */
bool QBViewRightIPTVCopyControlGetDOT(QBViewRightIPTVCopyControl self);


/**
 * @}
 **/
#ifdef __cplusplus
}
#endif

#endif //QBVIEWRIGHT_IPTV_COPYCONTROL_H_
