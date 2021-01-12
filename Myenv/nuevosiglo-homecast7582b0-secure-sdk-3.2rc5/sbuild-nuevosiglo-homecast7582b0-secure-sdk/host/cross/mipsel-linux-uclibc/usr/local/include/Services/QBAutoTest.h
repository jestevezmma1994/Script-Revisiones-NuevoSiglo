/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBAUTOTEST_H_
#define QBAUTOTEST_H_

/**
 * @file QBAutoTest.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief Auto test support.
 * If you call function QBAutoTestCreate and daemon for auto test will
 * be present on the STB (see App/Libraries/QBAutoTest),
 * then you might start/stop auto test.
 *
 * To start/stop auto test you need type from remote control this sequence
 * of keys:
 *
 *  MUTE
 *  RED
 *  MUTE
 *  GREEN
 *  MUTE
 *  YELLOW
 */

/**
 * @defgroup QBAutoTest auto test support service
 * @ingroup CubiTV_services
 * @{
 **/


/**
 * @brief Create QBAutoTest if exists daemon for auto test.
 **/
void QBAutoTestCreate(void);


/**
 * @brief Destroy QBAutoTest.
 **/
void QBAutoTestDestroy(void);

/**
 * @}
 **/

#endif
