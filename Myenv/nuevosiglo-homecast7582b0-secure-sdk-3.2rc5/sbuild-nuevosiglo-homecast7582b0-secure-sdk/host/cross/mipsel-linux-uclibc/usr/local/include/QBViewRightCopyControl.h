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

#ifndef QB_VIEW_RIGHT_COPY_CONTROL_H
#define QB_VIEW_RIGHT_COPY_CONTROL_H

/**
 * @file QBViewRightCopyControl.h QBViewRight DVB copy control information
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <dataformat/content_protection.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewRightCopyControl QBViewRightCopyControl.
 * @ingroup QBViewRight
 * @{
 **/

/**
 *  Based on chapter 6.4.1 (embedded CCI) in Digital Transmission Content Protection Specification V1 Revision 1.71
 */
typedef enum QBViewRightCopyControlEmiCci_e {
    QBViewRightCopyControlEmiCci_copyFreely = 0, /**< copy freely */
    QBViewRightCopyControlEmiCci_noMoreCopy = 1, /**< copy no more */
    QBViewRightCopyControlEmiCci_copyOneGeneration = 2, /**< copy once*/
    QBViewRightCopyControlEmiCci_copyNever = 3, /**< copy never*/
    QBViewRightCopyControlEmiCci_noAllowed = 4, /**< on DTCP-IP output is allowed */
} QBViewRightCopyControlEmiCci;

/**
 * @brief QBViewRightCopyControl
 */
typedef struct QBViewRightCopyControl_s* QBViewRightCopyControl;

/**
 * @brief Create instance of @link QBViewRightCopyControl @endlink
 * @param[in] bAnalogProt defines the APS-value in Bits 0 and 1, bit 7 defines whether Dwight-Cavendish CP needs to activated
 * @param[in] bCgmsa cgmsa mode, for @c 0xff no analog output is allowed
 * @param[in] bHdcp @c 0 in case no HDCP is needed, for every other value HDCP is mandatory. The value @1 HDCP 1.X can be applied, @2 HDCP 2.2 is required
 * @param[in] bDownresing bit 0 indicates whether downresing from HD to SD is allowed, bit 1 indicates whether downresing from UHD to HD is allowed,
 * @param[in] bEmiCci copy control information, for @c 0xff no DTCP-IP output is allowed
 * @param[out] errorInfo error info
 * @return instance of @link QBViewRightCopyControl @endlink or @c null in case of error
 */
QBViewRightCopyControl QBViewRightCopyControlCreate(uint8_t bAnalogProt, uint8_t bCgmsa, uint8_t bHdcp,
                                                    uint8_t bDownresing, uint8_t bEmiCci, SvErrorInfo *errorInfo);

/**
 * @brief Get parsed sv_content_protection
 * @param[in] self instance of @link QBViewRightCopyControl @endlink
 * @param[out] contentProtectionInfo parsed sv_content_protection
 */
void QBViewRightCopyControlGetContentProtection(QBViewRightCopyControl self, struct sv_content_protection *contentProtectionInfo);

/**
 * @brief Get @link QBViewRightCopyControlEmiCci @endlink parsed value
 * @param self instance of @link QBViewRightCopyControl @endlink
 * @return @link QBViewRightCopyControlEmiCci @endlink
 */
QBViewRightCopyControlEmiCci QBViewRightCopyControlGetEmmCci(QBViewRightCopyControl self);

/**
 * @brief Get information whether HDCP in version at least 2.2 is required
 * @param self instance of @link QBViewRightCopyControl @endlink
 * @return @c true if HDCP in version at least 2.2 is required
 */
bool QBViewRightCopyControlIsHdcp_2_2_Required(QBViewRightCopyControl self);

/**
 * @brief Get information whether HD content can be downresed to SD
 * @param self instance of @link QBViewRightCopyControl @endlink
 * @return @c true if HD content can be downresing to SD
 */
bool QBViewRightCopyControlIsHdToSdDownresingAllowed(QBViewRightCopyControl self);

/**
 * @brief Get information whether UHD content can be downresed to HD
 * @param self instance of @link QBViewRightCopyControl @endlink
 * @return @c true if UHD content can be downresing to HD
 */
bool QBViewRightCopyControlIsUhdToHdDownresingAllowed(QBViewRightCopyControl self);

/**
 * @brief Get information whether Dwight-Cavend copy protection is required
 * @param self instance of @link QBViewRightCopyControl @endlink
 * @return @c true if Dwight-Cavend copy protection is required
 */
bool QBViewRightCopyControlIsDwightCavendishRequired(QBViewRightCopyControl self);

/**
 * @interface QBViewRightCopyControlListener
 * Object which implements that interface will be notified about output copy control settings
 */
typedef struct QBViewRightCopyControlListener_* QBViewRightCopyControlListener;

/**
 * @struct QBViewRightCopyControlListener_
 * @brief See @ref QBViewRightCopyControlListener
 */
struct QBViewRightCopyControlListener_ {
    /**
     * @brief Notifies about the need for checking PIN.
     * @param[in] self_ instance of listener
     * @param[in] sessionId cas session id
     * @param[in] copyControl instatnce of @link QBViewRightCopyControl @endlink
     */
    void (*copyControl)(SvObject self, int sessionId, QBViewRightCopyControl copyControl);
};

/**
 * @brief Get instance of @ref SvInterface related to @ref QBViewRightCopyControlListener.
 * @return Instance of @ref SvInterface.
 */
SvInterface QBViewRightCopyControlListener_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_VIEW_RIGHT_COPY_CONTROL_H
