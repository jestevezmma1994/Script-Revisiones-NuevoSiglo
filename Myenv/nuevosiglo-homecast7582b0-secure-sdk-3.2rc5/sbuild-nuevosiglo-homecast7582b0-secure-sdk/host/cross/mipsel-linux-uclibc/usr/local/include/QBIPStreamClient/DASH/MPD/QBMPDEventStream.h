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

#ifndef QBMPDEVENTSTREAM_H_
#define QBMPDEVENTSTREAM_H_

/**
 * @file QBMPDEventStream.h
 * @brief Event Stream element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDXlink.h>

/**
 * @defgroup QBMPDEventStream Event Stream element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Event Stream element class.
 * @class QBMPDEventStream
 * @extends SvObject
 **/
typedef struct QBMPDEventStream_ *QBMPDEventStream;

/**
 * Get runtime type identification object representing
 * type of Event Stream element class.
 *
 * @return Event Stream element class
 **/
extern SvType
QBMPDEventStream_getType(void);

/**
 * Get message scheme of Event Stream element.
 *
 * @param[in] self Event Stream element handle
 * @return         message scheme, @c NULL in case of error
 **/
extern SvURI
QBMPDEventStreamGetShemeIdURI(QBMPDEventStream self);

/**
 * Get xlink element of Event Stream element.
 *
 * @param[in] self Event Stream element handle
 * @return         Xlink element, @c NULL in case of error
 **/
extern QBMPDXlink
QBMPDEventStreamGetxlink(QBMPDEventStream self);

/**
 * Get value of Event Stream element.
 *
 * @param[in] self Event Stream element
 * @return         value of Event Stream element, @c NULL in case of error
 **/
extern SvString
QBMPDEventStreamGetValue(QBMPDEventStream self);

/**
 * Get timescale of Event Stream element.
 *
 * @param[in] self Event Stream element
 * @return         timescale
 **/
extern uint64_t
QBMPDEventStreamGetTimescale(QBMPDEventStream self);

/**
 * Get array of Event elements of Event Stream element.
 *
 * @param[in]  self     Event Stream element
 * @return              array of Event elements, @c NULL in case of error
 **/
extern SvArray
QBMPDEventStreamGetEvents(QBMPDEventStream self);

/**
 * @}
 **/

#endif /* QBMPDEVENTSTREAM_H_*/
