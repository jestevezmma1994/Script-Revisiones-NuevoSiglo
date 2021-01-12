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

#ifndef QBIPSTREAMMANIFESTPARSERLISTENER_H_
#define QBIPSTREAMMANIFESTPARSERLISTENER_H_

/**
 * @file QBIPStreamManifestParserListener.h
 * @brief Stream manifest  parser listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBIPStreamManifestParserListener Stream manifest parser listener interface
 * @{
 **/

/**
 * QBIPStreamManifestParserListener interface.
 **/
typedef const struct QBIPStreamManifestParserListener_ {
    /**
     * Method called when parsing of stream manifest is finished successfully.
     *
     * @param[in] self     listener handle
     * @param[in] manifest stream manifest
     **/
    void (*parsingFinished)(SvObject self_,
                            SvObject manifest);

    /**
     * Method called when occured error while parsing stream manifest.
     *
     * @param[in] self     listener handle
     * @param[in] errorOut error info
     **/
    void (*parsingError)(SvObject self_,
                         SvErrorInfo errorOut);
} *QBIPStreamManifestParserListener;

/**
 * Get runtime identification object representing
 * stream manifest parser listener interface.
 *
 * @return stream manifest listener interface
 **/
extern SvInterface
QBIPStreamManifestParserListener_getInterface(void);

/**
 * @}
 **/

#endif /* QBIPSTREAMMANIFESTPARSERLISTENER_H_ */
