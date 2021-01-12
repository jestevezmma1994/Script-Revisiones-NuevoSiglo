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

#ifndef QBMPDABSTRACTELEMENT_H_
#define QBMPDABSTRACTELEMENT_H_

/**
 * @file QBMPDAbstractElement.h
 * @brief Abstract element interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAbstractAttributes.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAbstractNode.h>

/**
 * @defgroup QBMPDAbstractElement Abstract element interface
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * QBMPDAbstractElement interface.
 **/
typedef const struct QBMPDAbstractElement_ {
    /**
     * Add abstract node to abstract element.
     *
     * @param[in]  self         handle to an object implementing @ref QBMPDAbstractElement
     * @param[in]  abstractNode abstract node @ref QBMPDAbstractNode
     * @param[out] errorOut     error info
     **/
    void (*addAbstractNode)(SvObject self_,
                            QBMPDAbstractNode abstractNode,
                            SvErrorInfo *errorOut);

    /**
     * Set abstract attributes for abstract element.
     *
     * @param[in] self               handle to an object implementing @ref QBMPDAbstractElement
     * @param[in] abstractAttributes abstract attributes @ref QBMPDAbstractAttributes
     * @param[in] errorOut           error info
     **/
    void (*setAbstractAttributes)(SvObject self_,
                                  QBMPDAbstractAttributes abstractAttributes,
                                  SvErrorInfo *errorOut);

    /**
     * Get array of abstract nodes of abstract element.
     *
     * @param[in] self_ handle to an object implementing @ref QBMPDAbstractElement
     * @return          array with abstract nodes @ref QBMPDAbstractNode, @c NULL in case of error
     **/
    SvArray (*getAbstractNodes)(SvObject self_);

    /**
     * Get attributes of abstract element.
     *
     * @param[in] self_ handle to an object implementing, @ref QBMPDAbstractElement
     * @return          abstract attributes @ref QBMPDAbstractAttributes, @c NULL in case of error
     **/
    QBMPDAbstractAttributes (*getAbstractAttributes)(SvObject self_);
} *QBMPDAbstractElement;

/**
 * Get runtime identification object representing
 * Media Presentation Description parser listener interface.
 *
 * @return Media Presentation Description listener interface
 **/
extern SvInterface
QBMPDAbstractElement_getInterface(void);

/**
 * @}
 **/

#endif /* QBMPDABSTRACTELEMENT_H_ */
