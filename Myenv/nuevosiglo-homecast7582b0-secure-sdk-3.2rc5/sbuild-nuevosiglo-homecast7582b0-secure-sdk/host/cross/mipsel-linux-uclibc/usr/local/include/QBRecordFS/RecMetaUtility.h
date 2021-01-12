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

#ifndef REC_META_UTILITY_H
#define REC_META_UTILITY_H

#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvCore/SvLocalTime.h>

#ifdef __cplusplus
extern "C" {
#endif


// TODO: storage implementation dependant.
typedef int32_t  pvr_file_id;
#define PVR_FILE_ID_INVALID ((pvr_file_id)-1)

void RecMetaSetDataFile(SvContentMetaData meta, uint32_t id);
/// \returns zero or SV_ERR_*
int RecMetaGetDataFile(SvContentMetaData meta, uint32_t* id_out);

SvLocalTime RecMetaGetLocalTime(SvContentMetaData meta, SvString utc_key, SvString offset_key);
void RecMetaSetLocalTime(SvContentMetaData meta, SvLocalTime t, SvString utc_key, SvString offset_key);

/// \returns zero or SV_ERR_*
int RecMetaCreateContent(SvContentMetaData meta, SvContent* out);

void RecMetaPrint(SvContentMetaData meta, const char* reason);

#ifdef __cplusplus
}
#endif

#endif // #ifndef REC_META_UTILITY_H
