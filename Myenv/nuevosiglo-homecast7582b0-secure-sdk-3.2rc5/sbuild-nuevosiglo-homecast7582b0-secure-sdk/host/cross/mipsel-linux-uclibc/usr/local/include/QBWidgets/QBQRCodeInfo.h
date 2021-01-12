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

#ifndef QBQRCODEINFO_H_
#define QBQRCODEINFO_H_

#include <CUIT/Core/types.h>
#include <QBApplicationControllerTypes.h>
#include <QBSmartCardMonitor.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * @file  QBQRCodeInfo.h
 * @brief QBQRCodeInfo API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBQRCodeInfo displaying dialog with QR code encoded configuration.
 * @ingroup QBWidgets
 * @{
 **/

/**
 * This function runs a dialog window with configuration
 * information encoded in QR code.
 *
 * @param[in]  res              application handle
 * @param[in]  controller       application controller handle
 * @param[in]  smartCardMonitor QBSmartCardMonitor handle
 * @param[in]  writeToFile      write configuration to file (true) or not (false)
 * @param[out] errorOut         error info
 **/
void QBQRCodeInfoRunDialog(SvApplication res, QBApplicationController controller, QBSmartCardMonitor smartCardMonitor, bool writeToFile, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBQRCODEINFO_H_ */
