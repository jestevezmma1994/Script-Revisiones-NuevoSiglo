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

#ifndef SV_EPG_DATE_LABEL_H_
#define SV_EPG_DATE_LABEL_H_

/**
 * @file SvEPGDateLabel.h EPG Date Label Widget API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGDateLabel EPG date label widget
 * @ingroup SvEPG
 * @{
 **/

#include <time.h>
#include <SvCore/SvErrorInfo.h>
#include <CUIT/Core/types.h>

/**
 * Create new EPG date label widget.
 *
 * This method creates time axis widget.
 * Appearance of this widget is controlled by the Settings Manager.
 * It will use following parameters:
 *
 *  - width, height : integer, required,
 *  - font : font, required,
 *  - fontSize : integer, required,
 *  - textColor : color, required,
 *  - textAlign : string, optional (default: left),
 *  - delay : double, optional (default: 0.0),
 *  - format : string as in strftime(), optional (default: "%x").
 *
 * @param[in] app          CUIT application context
 * @param[in] widgetName   widget name
 * @param[out] errorOut    error info
 * @return                 SvEPGDateLabel widget, NULL in case of error
 **/
extern SvWidget
SvEPGDateLabelNew(SvApplication app,
                  const char *widgetName,
                  SvErrorInfo *errorOut);

/**
 * Set date to be shown.
 *
 * @param[in] label        EPG date label widget
 * @param[in] date         date to be shown
 * @param[out] errorOut    error info
 **/
extern void
SvEPGDateLabelSetCurrentDate(SvWidget label,
                             time_t date);

/**
 * Reinitializes the date label.
 *
 * @param[in] label       EPG date label widget
 **/
extern void
SvEPGDateLabelReinitialize(SvWidget label);

extern void
SvEPGDateLabelSetFormatId(SvWidget w, const char *formatId);

/**
 * @}
 **/

#endif
