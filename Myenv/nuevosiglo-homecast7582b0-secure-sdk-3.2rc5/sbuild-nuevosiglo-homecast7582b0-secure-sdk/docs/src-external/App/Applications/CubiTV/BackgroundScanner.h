/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef BACKGROUND_SCANNER_H_
#define BACKGROUND_SCANNER_H_

#include <stdbool.h>
#include <QBChannelScanningConf.h>
#include <main_decl.h>


typedef struct BackgroundScanner_ *BackgroundScanner;

/**
 * BackgroundScanner callback type to be called when scanning is finished for any reason.
 *
 * @param[in] owner     callback owner
 * @param[in] success   @c true if scanning is successfully finished or @c false otherwise
 */
typedef void (*BackgroundScannerFinishedCallback)(void *owner, bool success);

extern BackgroundScanner BackgroundScannerCreate(AppGlobals appGlobals);

/**
 * Set callback that will be called when background scanning is finished.
 *
 * @param[in] self          BackgroundScanner self object
 * @param[in] callback      callback to be set
 * @param[in] owner         data for the callback to be called with
 */
extern void BackgroundScannerSetCallback(BackgroundScanner self, BackgroundScannerFinishedCallback callback, void *owner);

extern bool BackgroundScannerIsScanning(BackgroundScanner self);
extern void BackgroundScannerStartScanning(BackgroundScanner self, unsigned int tunerNum, QBChannelScanningConf conf);
extern void BackgroundScannerStopScanning(BackgroundScanner self);


#endif
