/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_CLOSED_CAPTION_SUBTITLES_H_
#define SV_CLOSED_CAPTION_SUBTITLES_H_

/**
 * @file SvClosedCaptionSubtitles.h SvClosedCaptionSubtitles class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvClosedCaptionSubtitles Closed Captions class
 * @ingroup SvPlayerKit
 * @{
 **/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Standards of Closed Captions subtitles
 */
typedef enum {
    QBClosedCaptionType_CEA_608 = 0,
    QBClosedCaptionType_CEA_708
} QBClosedCaptionType;

/**
 * Closed Captions data packet
 */
struct QBClosedCaptionData_s {
    bool valid;
    QBClosedCaptionType type;
    unsigned char c1, c2;
    int field; // for CEA-608 indicates field (0 or 1)
};

typedef struct QBClosedCaptionData_s QBClosedCaptionData;


#ifdef __cplusplus
}
#endif

#endif
