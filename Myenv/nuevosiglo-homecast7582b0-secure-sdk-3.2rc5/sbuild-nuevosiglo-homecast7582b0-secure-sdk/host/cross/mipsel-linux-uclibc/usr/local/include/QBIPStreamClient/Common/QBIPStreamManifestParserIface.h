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

#ifndef QB_IP_STREAM_MANIFEST_PARSER_IFACE_H
#define QB_IP_STREAM_MANIFEST_PARSER_IFACE_H

/**
 * @file  QBIPStreamManifestParserIface.h Streaming manifest parser interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/Common/QBIPStreamManifestParserListener.h>
#include <SvPlayerKit/SvBuf.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBIPStreamManifestParserIface Streaming manifest parser interface.
 * @{
 **/

/**
 * Stream manifest parser interface
 * @interface QBIPStreamManifestParserIface
 */
typedef struct QBIPStreamManifestParserIface_ *QBIPStreamManifestParserIface;

struct QBIPStreamManifestParserIface_ {
    /**
     * Set manifest parser listener.
     *
     * @param[in] self          interface object
     * @param[in] listener      listener to set
     * @param[out] errorOut     error info
     */
    void (*setListener)(SvObject self, SvObject listener, SvErrorInfo *errorOut);

    /**
     * Push data to manifest parser.
     *
     * @param[in] self          interface object
     * @param[in] sb            buffer
     * @param[in] pos           position in buffer
     * @param[out] errorOut     error info
     */
    void (*pushData)(SvObject self, SvBuf sb, int64_t pos, SvErrorInfo *errorOut);

    /**
     * Finished pushing data.
     *
     * @param[in] self          interface object
     * @param[out] errorOut     error info
     */
    void (*dataFinished)(SvObject self, SvErrorInfo *errorOut);

    /**
     * Reset manifest parser for parsing next manifest.
     *
     * @param[in] self          interface object
     * @param[out] errorOut     error info
     */
    void (*reset)(SvObject self, SvErrorInfo *errorOut);

    /**
     * Start manifest parser.
     *
     * @param[in] self          interface object
     */
    void (*start)(SvObject self);

    /**
     * Stop manifest parser.
     *
     * @param[in] self          interface object
     */
    void (*stop)(SvObject self);
};

/**
 * Get runtime type identification object representing
 * QBIPStreamManifestParserIface interface.
 *
 * @relates QBIPStreamManifestParserIface
 *
 * @return representing interface object
 **/
SvInterface
QBIPStreamManifestParserIface_getInterface(void);

/**
 * @}
 */

#endif /* QB_IP_STREAM_MANIFEST_PARSER_IFACE_H */
