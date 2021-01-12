/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DVB_UPGRADE_SELECTOR_H_
#define QB_DVB_UPGRADE_SELECTOR_H_

/**
 * @file QBDVBUpgradeSelector.h
 * @brief DVB upgrade selector
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDVBUpgradeSelector DVB upgrade selector type
 * @ingroup QBUpgrade
 * @{
 **/

#include <stdint.h>
#include <SvFoundation/SvString.h>

/**
 * DVB upgrade selector.
 **/
typedef struct {
    /// 24 bit globally unique company ID assigned by IEEE
    uint8_t OUI[3];
    /// STB description
    struct {
        /// SoC model
        uint8_t cpuModel;
        /// STB model
        uint8_t model;
    } board;
    /// project ID
    uint8_t project;
} QBDVBUpgradeSelector;


/**
 * Creates formatted string containing selector
 * @param[in] selector      binary representation of selector
 * @return formatted string
 */
SvString QBDVBUpgradeSelectorCreateStringFromSelector(const QBDVBUpgradeSelector *selector);


/**
 * @}
 **/


#endif
