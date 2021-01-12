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

#ifndef QBQRCODE_H_
#define QBQRCODE_H_

#include <CAGE/Core/SvBitmap.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>
#include <qrencode.h>

/**
 * @file  QBQRCode.h
 * @brief QBQRCode API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBQRCode encoding string into QR code bitmaps.
 * @ingroup QBWidgets
 * @{
 **/

/**
 * QuietZone is a region around QR code which shall be free of all other markings.
 * QuietZone width variants expressed in the number of QR code module sizes.
 **/
typedef enum {
    /// equal to 1 module size
    QBQRCodeQuietZone_x1 = 1,
    /// 2 times module size
    QBQRCodeQuietZone_x2 = 2,
    /// 4 times module size
    QBQRCodeQuietZone_x4 = 4,
    /// default value, 2 times module size
    QBQRCodeQuietZone_standard = QBQRCodeQuietZone_x4,
} QBQRCodeQuietZone;

/**
 * Parameters for QR code encoding.
 **/
struct QBQRCodeParams_ {
    /// Size (in pixels) of one square area comprising QR Code.
    unsigned int moduleSize;
    /// QBQRCodeQuietZone width expressed as the number of QR code module sizes.
    QBQRCodeQuietZone quietZoneSize;
    /// QR code version.
    int version;
    /// Level of error correction.
    QRecLevel level;
    /// Encoding mode.
    QRencodeMode hint;
    /// Case-sensitive(true) or not(false).
    bool isCaseSensitive;
};
typedef struct QBQRCodeParams_ *QBQRCodeParams;

/**
 * Sets /p params with default values for QR code generation.
 *
 * @param[out]  params       encoding parameters handle
 **/
void QBQRCodeSetDefaultParams(QBQRCodeParams params);

/**
 * Encodes /p string into a QR code and creates SvBitmap object.
 *
 * @param[in]  string       NULL-terminated string to encode
 * @param[in]  params       encoding parameters
 * @param[out] errorOut     error info
 * @return  SvBitmap object containing QR code
 **/
SvBitmap QBQRCodeCreate(SvString string, QBQRCodeParams params, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBQRCODE_H_ */
