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

#ifndef QB_SHARED_DECODER_H_
#define QB_SHARED_DECODER_H_

#include <stdbool.h>
#include <QBViewport_low.h>

/**
 * @defgroup QBSharedDecoder: shared decoder
 * @ingroup QBSharedDecoder
 * @{
 *
 * Shared decoder class enables sharing audio and video
 * decoders between CubiTV and other applications.
 **/

/**
 * QBSharedDecoder class.
 *
 * @class QBSharedDecoder
 *
 */
typedef struct QBSharedDecoder_s QBSharedDecoder;

/**
 * QBSharedDecoderBase settings.
 *
 * @class QBSharedDecoder
 *
 */
struct QBSharedDecoderBaseSettings_s {
    // content format
    struct svdataformat* format;
    // volume level
    int volume;
};

typedef struct QBSharedDecoderBaseSettings_s QBSharedDecoderBaseSettings;

/**
 * QBSharedDecoderBase class.
 *
 * @class QBSharedDecoder
 *
 */
struct QBSharedDecoderBase_s {
    // decoder base id
    int id;
    // error code of last error
    int error;
    // decoder readiness flag
    bool isReady;
    // audio outputs count
    int audioOutputCnt;
    // currently applied decoder base settings
    QBSharedDecoderBaseSettings currentSettings;
    // decoder base settings pending to be applied
    QBSharedDecoderBaseSettings pendingSettings;
    // viewport handle
    QBViewport viewport;
};

typedef struct QBSharedDecoderBase_s QBSharedDecoderBase;

/**
 * This function creates QBSharedDecoder class instance.
 *
 * @memberof QBSharedDecoder
 *
 * @param[in] viewport      QBViewport handle, shared decoder will attach to it
 *
 * @return                  QBSharedDecodr handle, @c NULL if error occured
 */
extern QBSharedDecoder* QBSharedDecoderCreate(QBViewport viewport);

/**
 * This function returns QBSharedDecoder instance readiness status.
 *
 * @memberof QBSharedDecoder
 *
 * @param[in] dec           shared decoder handle
 *
 * @return                  @c true if ready, @c false otherwise
 */
extern bool QBSharedDecoderIsReady(QBSharedDecoder *dec);

/**
 * This function tells if shared decoder is available on the platform.
 *
 * @memberof QBSharedDecoder
 *
 * @return                  @c true if available, @c false otherwise
 */
extern bool QBSharedDecoderIsAvailable(void);

/**
 * This function destroys shared decoder.
 *
 * @memberof QBSharedDecoder
 *
 * @param[in] dec           shared decoder handle
 *
 */
extern void QBSharedDecoderDestroy(QBSharedDecoder *dec);

/**
 * @}
 **/

#endif

