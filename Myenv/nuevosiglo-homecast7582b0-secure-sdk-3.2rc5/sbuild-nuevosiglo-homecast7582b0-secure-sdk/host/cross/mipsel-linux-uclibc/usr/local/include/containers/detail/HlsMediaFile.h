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

#ifndef HLSMEDIAFILE_H_
#define HLSMEDIAFILE_H_

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURL.h>


//TODO make MediaFileSimple a base class for MediaFile - changes in the cont_parser_hls (SvURL)
// Media file data
struct MediaFile_ {
    struct SvObject_ super_;
    int seqNumber;
    SvURL url;
    int startTime_ms;
    int duration_ms;
    bool discontinuity;
    int keyId;
};
typedef struct MediaFile_* MediaFile;

// Media file data - simplified used to store VOD media lists
struct MediaFileSimple_ {
    struct SvObject_ super_;
    char* urlPart;
    int startTime_ms;
    int duration_ms;
    int keyId;
};
typedef struct MediaFileSimple_* MediaFileSimple;

extern SvType
MediaFile_getType(void);

extern SvType
MediaFileSimple_getType(void);

#endif /* HLSMEDIAFILE_H_ */
