/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SV_CONTENT_CHAPTER_H_
#define SV_CONTENT_CHAPTER_H_

/**
 * @file SvContentChapter.h SvContentChapter class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

/**
 * @defgroup SvContentChapter The SvContentChapter class.
 * @ingroup SvPlayerKit_Metadata
 * @{
 **/

/**
 * The SvContentChapter reference type.
 *
 * This type should be used to store references to the instances of the
 * SvContentChapter class or it's sub-classes.
 **/
typedef struct SvContentChapter_ {
    struct SvObject_ super_;
    int number;
    float pos;
    SvString name;
} *SvContentChapter;


extern SvType
SvContentChapter_getType(void);

extern SvContentChapter
SvContentChapterCreate(int num,
                       float pos,
                       SvString name,
                       SvErrorInfo *errorOut);

/** @} */

#endif
