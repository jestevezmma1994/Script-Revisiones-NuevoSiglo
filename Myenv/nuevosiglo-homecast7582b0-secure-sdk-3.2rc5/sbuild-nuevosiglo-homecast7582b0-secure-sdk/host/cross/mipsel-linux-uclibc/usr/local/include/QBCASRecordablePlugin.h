/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCASRECORDABLEPLUGIN_H_
#define QBCASRECORDABLEPLUGIN_H_

#include <SvCore/SvCommonDefs.h>
#include <SvPlayerKit/SvBuf.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <stdbool.h>

/**
 * @brief Object which implements that interface can receive each data block.
 */
typedef const struct QBCASRecordablePlugin_ {
    /**
     * @brief It is called each time new data block appears
     * @param[in] self_ plugin instance
     * @param[in] sb data block
     * @param[out] recordable indicates if block is PVR capable
     * @param[out] timeshiftable indicates if block is timeshift capable
     */
    void (*processBlock) (SvObject self_, const SvBuf sb, bool *recordable, bool *timeshiftable);
} *QBCASRecordablePlugin;

/**
 * @brief Gets SvInterface instance of QBCASRecordablePlugin
 */
SvExport SvInterface QBCASRecordablePlugin_getInterface(void);

#endif /* QBCASRECORDABLEPLUGIN_H_ */
