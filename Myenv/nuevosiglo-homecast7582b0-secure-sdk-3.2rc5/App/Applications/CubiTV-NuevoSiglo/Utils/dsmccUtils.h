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

#ifndef DSMCCUTILS_H
#define DSMCCUTILS_H

/**
 * @file dsmccUtils.h utilities for DSM-CC features.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <CUIT/Core/types.h>
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvHashTable.h>

/**
 * @defgroup CubiTVDSMCCUtils DSM-CC utilities
 * @ingroup CubiTV_utils
 * @{
 */

/**
 * Set EPG advertisements.
 *
 * @param[in] app           CUIT application handle
 * @param[in] window        window handle
 * @param[in] settingsCtx   context id where "TVPreview" and "BottomAd" widgets are defined
 * @param[in,out] adPreview pointer to preview advert widget handle (may be NULL)
 * @param[in,out] adBottom  pointer to bottom advert widget handle (may be NULL)
 * @param[in] images        map SvString to SvRBBitmap, where keys are "epg_1" and "epg_2"
 */
SvHidden void DSMCCUtilsEPGSetAds(SvApplication app,
                                  SvWindow window,
                                  int settingsCtx,
                                  SvWidget *adPreview,
                                  SvWidget *adBottom,
                                  SvHashTable images);

/**
 * @}
 */

#endif // DSMCCUTILS_H
