/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_INTEK_CONAX_SMARTCARD_FINGERPRINT_H
#define QB_INTEK_CONAX_SMARTCARD_FINGERPRINT_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBICSmartcardFingerprint_s {
    struct SvObject_ super_;

    int32_t start_time;     /**< -1 is "now", else number of milli-seconds from last midnight */
    int duration;           /**< fingerprint duration in milli-seconds */
    bool withPriority;      /**< findicates if fingerprint has priority level */

    uint16_t x;             /**< position horizontal coordinate */
    uint16_t y;             /**< position vertical coordinate */
    uint8_t font_size;      /**< font size */
    SvString text;          /**< text which should be shown on the screen, white label without background */

    bool validReceivedTime; /**< true if @receivedTime field is set properly */
    int64_t receivingTime;   /**< received time in microseconds, could be used only if @validReceivedTime is set */
};
typedef struct QBICSmartcardFingerprint_s QBICSmartcardFingerprint;

extern SvType  QBICSmartcardFingerprint_getType(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_INTEK_CONAX_SMARTCARD_FINGERPRINT_H
