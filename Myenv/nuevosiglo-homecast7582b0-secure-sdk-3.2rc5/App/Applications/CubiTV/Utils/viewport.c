/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include "viewport.h"
#include <string.h>
#include <QBPlatformHAL/QBPlatformOutput.h>


static int lstvGetVideoOutputID(const char *output_type)
{
  if (strcasecmp(output_type, "SD") == 0)
    return QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_SD, true, false);
  else if (strcasecmp(output_type, "HD") == 0)
    return QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_HD, true, false);
  return -1;
}

void lstvSetVideoContentMode(QBViewport viewport, const char* output_type, const char* mode_name)
{
  QBContentDisplayMode mode = QBContentDisplayMode_full;
  if (mode_name) {
    if (strcasecmp(mode_name, "PANSCAN") == 0) {
      mode = QBContentDisplayMode_panScan;
    } else if (strcasecmp(mode_name, "LETTERBOX") == 0) {
      mode = QBContentDisplayMode_letterBox;
    } else if (strcasecmp(mode_name, "STRETCHED") == 0
               || /*backwards compatibility*/ strcasecmp(mode_name, "FULLSCREEN") == 0) {
      mode = QBContentDisplayMode_stretched;
    }
  }

  int output_id = lstvGetVideoOutputID(output_type);
  if (output_id < 0)
    return;

  QBViewportSetContentMode(viewport, output_id, mode);
}
