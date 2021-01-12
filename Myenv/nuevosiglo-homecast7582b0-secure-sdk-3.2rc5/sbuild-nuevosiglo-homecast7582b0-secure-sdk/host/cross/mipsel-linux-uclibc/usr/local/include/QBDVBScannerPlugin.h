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

#ifndef QBDVBSCANNERPLUGIN_H_
#define QBDVBSCANNERPLUGIN_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvHashTable.h>
#include <SvPlayerKit/SvChbuf.h>
#include <QBDVBScannerTypes.h>
#include <QBDVBChannelDesc.h>
#include <mpeg_tables/sdt.h>
#include <mpeg_tables/nit.h>

typedef const struct QBDVBScannerPlugin_ {
    /**
     * Tells if plugin is ready.
     *
     * @param[in] self         plugin handle
     * @param[in] scanner      scannder handle
     * @return                 @c true if plugin is ready, @c false otherwise
     **/
    bool (*isReady)(SvObject self_, QBDVBScanner *scanner);

    void (*updateSDTChannel)(SvObject self, QBDVBChannelDesc chan, QBDVBChannelVariant* variant, mpeg_sdt_element* sdt_element, struct base_results_s* base, char** name, char** provider, int *running, bool *free_CA_mode, SvHashTable metadata);

    /**
     * Update channel with NIT information.
     *
     * @param[in] self              plugin handle
     * @param[in, out] chan         channel handle
     * @param[in, out] variant      channel variant
     * @param[in] nit_element       NIT table element
     * @param[in, out] base         scanner base results
     * @param[in, out] lcn          channel lcn
     * @param[in, out] visible      channel visibility
     * @param[in, out] nid          channel nid
     * @param[in, out] metadata     plugin metadata
     * @param[in] actualMuxId       actual mux id
     **/
    void (*updateNITChannel)(SvObject self, QBDVBChannelDesc chan, QBDVBChannelVariant *variant, mpeg_nit_element *nit_element, struct base_results_s *base, int *lcn, int *visible, int *nid, SvHashTable metadata, QBTunerMuxIdObj *actualMuxId);

    void (*afterUpdateSDTChannel)(SvObject self, QBDVBChannelDesc chan, QBDVBChannelVariant* variant, mpeg_sdt_element* sdt_element, SvHashTable metadata);

    /**
     * Notify about ending of channel update with NIT information.
     *
     * @param[in] self              plugin handle
     * @param[in, out] chan         channel handle
     * @param[in, out] variant      channel variant
     * @param[in] nit_element       NIT table element
     * @param[in, out] metadata     plugin metadata
     * @param[in] actualMuxId       actual mux id
     **/
    void (*afterUpdateNITChannel)(SvObject self, QBDVBChannelDesc chan, QBDVBChannelVariant *variant, mpeg_nit_element *nit_element, SvHashTable metadata, QBTunerMuxIdObj *acutalMuxId);

    /**
     * Notify about received NIT
     *
     * @param[in] self              plugin handle
     * @param[in] scanner           scanner handle
     * @param[in] chb               buffer with NIT raw data
     * @param[in, out] metadata     plugin metadata
     * @param[in] actualMuxId       actual mux id
     **/
    void (*receivedNIT)(SvObject self_, QBDVBScanner *scanner, SvChbuf chb, SvHashTable metadata, QBTunerMuxIdObj *acutalMuxId);

    void (*receivedPAT)(SvObject self_, QBDVBScanner *scanner, SvChbuf chb, SvHashTable metadata);
} *QBDVBScannerPlugin;
SvInterface QBDVBScannerPlugin_getInterface(void);

#endif
