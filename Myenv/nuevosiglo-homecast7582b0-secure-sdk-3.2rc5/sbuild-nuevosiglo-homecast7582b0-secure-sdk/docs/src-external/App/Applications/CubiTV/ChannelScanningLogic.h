/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBCHANNELSCANNINGLOGIC_H_
#define QBCHANNELSCANNINGLOGIC_H_

/**
 * @file ChannelScanningLogic.h Channel scanning logic API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBInput/QBInputEvent.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>
#include <QBChannelScanningConf.h>
#include <QBApplicationController.h>
#include <QBDVBScanner.h>
#include <QBDVBSatelliteDescriptor.h>
#include <QBTunerTypes.h>
#include <main_decl.h>
#include <stdbool.h>

/**
 * @class QBChannelScanningLogic ChannelScanningLogic.h <Logic/ChannelScanningLogic.h>
 * @extends SvObject
 *
 * Channel scanning logic.
 **/
typedef struct QBChannelScanningLogic_ *QBChannelScanningLogic;

/**
 * Channel scanning logic virtual methods table.
 **/
typedef const struct QBChannelScanningLogicVTable_ {
    /**
     * virtual methods table of the base class
     **/
    struct SvObjectVTable_ super_;

    /**
     * Initialize channel scanning logic instance.
     *
     * @param[in]  self       QBChannelScanningLogic handle
     * @param[in]  appGlobals CubiTV application state
     * @param[out] errorOut   error info
     * @return                @a self or @c NULL in case of error
     **/
    QBChannelScanningLogic (*init)(QBChannelScanningLogic self,
                                   AppGlobals appGlobals,
                                   SvErrorInfo *errorOut);

    /**
     * Create window context that will be shown during forced channel scanning.
     *
     * @param[in] self      channel scanning logic handle
     * @param[in] conf      scanning configuration
     * @return              window context
     */
    QBWindowContext (*createForcedContext)(QBChannelScanningLogic self,
                                           QBChannelScanningConf conf);

    /**
     * Process found channels.
     *
     * @param[in] self      channel scanning logic handle
     * @param[in] conf      scanning configuration
     * @param[in] scanner   scanner
     */
    void (*processFoundChannels)(QBChannelScanningLogic self,
                                 QBChannelScanningConf conf,
                                 QBDVBScanner *scanner);

    /**
     * Inform about scanning end.
     *
     * @param[in] self              channel scanning logic handle
     * @param[in] conf              scanning configuration
     * @param[in] succesfulScan     @c true if scan was successful, @c false otherwise
     */
    void (*scanningFinished)(QBChannelScanningLogic self,
                             QBChannelScanningConf conf,
                             bool succesfulScan);

    /**
     * Input event handler.
     *
     * @param[in] self      channel scanning logic handle
     * @param[in] e         input event
     */
    bool (*inputEventHandler)(QBChannelScanningLogic self,
                              const QBInputEvent *e);

    /**
     * Show channel scanning window.
     *
     * @param[in] self      channel scanning logic handle
     * @param[in] config    scanning configuration
     */
    void (*showChannelScanningWithConf)(QBChannelScanningLogic self,
                                        QBChannelScanningConf config);

    /**
     * Checks if radio is enabled.
     *
     * @param[in] self      channel scanning logic handle
     * @return              @c true if radio is enabled, @c false otherwise
     */
    bool (*isRadioEnabled)(QBChannelScanningLogic self);

    /**
     * Create and set plugins for given DVBScanner instance.
     *
     * @param[in] self      QBChannelScanningLogic handle
     * @param[in] scanner   QBDVBScanner handle for which plugins will be connected
     */
    void (*setDVBScannerPlugins)(QBChannelScanningLogic self,
                                 QBDVBScanner *scanner);
} *QBChannelScanningLogicVTable;

/**
 * Get runtime type identification object representing QBChannelScanningLogic type.
 *
 * @relates QBChannelScanningLogic
 *
 * @return type identification object
**/
SvType
QBChannelScanningLogic_getType(void);

/**
 * Create channel scanning logic.
 *
 * @param[in] appGlobals    application globals
 * @param[out] errorOut     error info
 * @return                  newly created logic or @c NULL in case of error
 */
QBChannelScanningLogic
QBChannelScanningLogicCreate(AppGlobals appGlobals, SvErrorInfo *errorOut);

/**
 * Get default scanning method.
 *
 * @param[in] self      channel scanning logic handle
 * @return              default scanning method
 */
SvString
QBChannelScanningLogicGetDefaultScanningMethod(QBChannelScanningLogic self);

/**
 * Is scan successful.
 *
 * @param[in] self      channel scanning logic handle
 * @param[in] conf      scanning configuration
 * @param[in] scanner   scanner
 * @return              @c true if scan is successful, @c false otherwise
 */
bool
QBChannelScanningLogicIsScanSuccessful(QBChannelScanningLogic self, QBChannelScanningConf conf, QBDVBScanner *scanner);

/**
 * Create window context that will be shown during forced channel scanning.
 *
 * @param[in] self      channel scanning logic handle
 * @param[in] conf      scanning configuration
 * @return              window context
 */
static inline QBWindowContext
QBChannelScanningLogicCreateForcedContext(QBChannelScanningLogic self, QBChannelScanningConf conf)
{
    return SvInvokeVirtual(QBChannelScanningLogic, self, createForcedContext, conf);
}

/**
 * Process found channels.
 *
 * @param[in] self      channel scanning logic handle
 * @param[in] conf      scanning configuration
 * @param[in] scanner   scanner
 */
static inline void
QBChannelScanningLogicProcessFoundChannels(QBChannelScanningLogic self, QBChannelScanningConf conf, QBDVBScanner *scanner)
{
    return SvInvokeVirtual(QBChannelScanningLogic, self, processFoundChannels, conf, scanner);
}

/**
 * Inform about scanning end.
 *
 * @param[in] self              channel scanning logic handle
 * @param[in] conf              scanning configuration
 * @param[in] succesfulScan     @c true if scan was successful, @c false otherwise
 */
static inline void
QBChannelScanningLogicScanningFinished(QBChannelScanningLogic self, QBChannelScanningConf conf, bool succesfulScan)
{
    return SvInvokeVirtual(QBChannelScanningLogic, self, scanningFinished, conf, succesfulScan);
}

/**
 * Input event handler.
 *
 * @param[in] self      channel scanning logic handle
 * @param[in] e         input event
 * @return              @c true if event was handled, @c false otherwise
 */
static inline bool
QBChannelScanningLogicInputEventHandler(QBChannelScanningLogic self, const QBInputEvent *e)
{
    return SvInvokeVirtual(QBChannelScanningLogic, self, inputEventHandler, e);
}

/**
 * Get channel scanning results.
 *
 * @param[in] self      channel scanning logic handle
 * @return              scanning results
 */
SvHashTable
QBChannelScanningLogicGetResults(QBChannelScanningLogic self);

/**
 * Show channel scanning window.
 *
 * @param[in] self      channel scanning logic handle
 */
void
QBChannelScanningLogicShowChannelScanning(QBChannelScanningLogic self);

/**
 * Show channel scanning window.
 *
 * @param[in] self      channel scanning logic handle
 * @param[in] config    scanning configuration
 */
static inline void
QBChannelScanningLogicShowChannelScanningWithConf(QBChannelScanningLogic self, QBChannelScanningConf config)
{
    return SvInvokeVirtual(QBChannelScanningLogic, self, showChannelScanningWithConf, config);
}

/**
 * Get middleware timeout message tag.
 *
 * @param[in] self      channel scanning logic handle
 * @return              message tag
 */
SvString
QBChannelScanningLogicGetMWTimeoutMessageTag(QBChannelScanningLogic self);

/**
 * Checks if radio is enabled.
 *
 * @param[in] self      channel scanning logic handle
 * @return              @c true if radio is enabled, @c false otherwise
 */
static inline bool
QBChannelScanningLogicIsRadioEnabled(QBChannelScanningLogic self)
{
    return SvInvokeVirtual(QBChannelScanningLogic, self, isRadioEnabled);
}

/**
 * Create and set plugins for given DVBScanner instance.
 *
 * @param[in] self      QBChannelScanningLogic handle
 * @param[in] scanner   QBDVBScanner handle for which plugins will be connected
 */
static inline void
QBChannelScanningLogicSetDVBScannerPlugins(QBChannelScanningLogic self, QBDVBScanner *scanner)
{
    return SvInvokeVirtual(QBChannelScanningLogic, self, setDVBScannerPlugins, scanner);
}

#endif
