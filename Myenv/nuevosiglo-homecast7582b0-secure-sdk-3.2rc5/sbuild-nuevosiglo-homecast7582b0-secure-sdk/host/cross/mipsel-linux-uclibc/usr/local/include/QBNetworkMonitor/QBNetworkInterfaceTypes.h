/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_NETWORK_INTERFACE_TYPES_H_
#define QB_NETWORK_INTERFACE_TYPES_H_

/**
 * @file QBNetworkInterfaceTypes.h Network interfaces of different types declarations.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @ingroup QBNetworkMonitor
 * @{
 **/

/**
 * Network interface is an abstract class that contains common data
 * that is required by different kinds of network interfaces
 *
 * @class QBNetworkInterface
 **/
typedef struct QBNetworkInterface_ *QBNetworkInterface;

/**
 * Ethernet interface is a class that represents an ethernet
 * network interface.
 *
 * @class QBEthernetInterface
 **/
typedef struct QBEthernetInterface_ *QBEthernetInterface;

/**
 * WiFi interface is a class that represents a WiFi
 * network interface.
 *
 * @class QBWiFiInterface
 **/
typedef struct QBWiFiInterface_ *QBWiFiInterface;

/**
 * MoCa interface is a class that represents an MoCa
 * network interface.
 *
 * @class QBMoCaInterface
 **/
typedef struct QBMoCaInterface_ *QBMoCaInterface;

/**
 * Cable modem interface is a class that represents a cable modem
 * network interface.
 *
 * @class QBDocsisInterface
 **/
typedef struct QBDocsisInterface_ *QBDocsisInterface;

/**
 * 3G modem interface is a class that represents a 3G modem
 * network interface.
 *
 * @class QBDocsisInterface
 **/
typedef struct QB3GInterface_ *QB3GInterface;

/**
 * @}
**/

#endif //QB_NETWORK_INTERFACE_TYPES_H_
