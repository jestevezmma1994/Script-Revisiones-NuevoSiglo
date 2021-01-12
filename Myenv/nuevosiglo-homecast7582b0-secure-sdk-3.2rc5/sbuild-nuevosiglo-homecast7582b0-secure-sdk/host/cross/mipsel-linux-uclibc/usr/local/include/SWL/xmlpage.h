/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SWL_XMLPAGE_H_
#define SWL_XMLPAGE_H_

/**
 * @file SMP/SWL/swl/xmlpage.h XmlPage widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvFont.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup SvXmlPage XmlPage widget
 * @ingroup SWL_widgets
 * @{
 **/


/// Type of XML page.
typedef enum {
    SV_XMLPAGE_BITMAP_BG,
    SV_XMLPAGE_WIDGET_BG
} SvXmlPageType;

/// XML page orientation.
typedef enum {
    SV_XMLPAGE_VERTICAL,
    SV_XMLPAGE_HORIZONTAL
} SvXmlPageDirection;

/**
 * Create XmlPage widget from memory buffer.
 **/
extern SvWidget
svXmlPageMemoryNew(SvApplication app, SvXmlPageType type,
                   SvFont font, int fsize, SvColor fcolor, SvColor bgcolor,
                   int w, int h, const char *src, SvXmlPageDirection dir, bool cropHeight);

// FIXME: documentation!
extern void
svXmlPageGetSize(SvWidget wgt, int *width, int *height);


/**
 * @}
 **/

#endif
