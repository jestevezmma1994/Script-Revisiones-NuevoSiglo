/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
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

#ifndef DRM_MANAGER_H
#define DRM_MANAGER_H

#include "types_fwd.h"

#define DRM_PLUGIN_MARLIN        1
#define DRM_PLUGIN_VERIMATRIX    2
#define DRM_PLUGIN_WINDOWSMEDIA  3

#define DRM_PLUGIN_FAKE          7

#ifdef __cplusplus
extern "C" {
#endif

SvDRMManager  SvDRMManagerCreate(void);
void SvDRMManagerDestroy(SvDRMManager man);

SvDRMPlugin SvDRMManagerFindPlugin(SvDRMManager manager, int drm_id);

/// global instance
SvDRMManager  SvDRMManagerGet(void);
void  SvDRMManagerCleanup(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef DRM_MANAGER_H
