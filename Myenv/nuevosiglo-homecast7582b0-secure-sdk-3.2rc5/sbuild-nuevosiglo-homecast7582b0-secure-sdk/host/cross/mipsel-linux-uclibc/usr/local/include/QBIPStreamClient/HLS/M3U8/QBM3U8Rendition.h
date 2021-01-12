/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBM3U8RENDITION_H_
#define QBM3U8RENDITION_H_

/**
 * @file QBM3U8Rendition.h
 * @brief HLS Rendition Group API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURI.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>

/**
 * @defgroup QBM3U8Rendition HLS Rendition class
 * @ingroup QBM3U8Parser
 * @{
 **/

/**
 * Rendition class.
 * @class QBM3U8Rendition.
 **/
typedef struct QBM3U8Rendition_ *QBM3U8Rendition;

/**
 * Get runtime type identification object representing
 * type of Rendition class.
 *
 * @return Rendition class
 **/
extern SvType
QBM3U8Rendition_getType(void);

/**
 * QBM3U8RenditionType is used to indicate the type of rendition group.
 **/
typedef enum {
    QBM3U8RenditionType_invalid,            ///< value not set
    QBM3U8RenditionType_audio,              ///< audio
    QBM3U8RenditionType_video,              ///< video
    QBM3U8RenditionType_subtitles,          ///< subtitles
    QBM3U8RenditionType_closedCaptions,     ///< closed captions
} QBM3U8RenditionType;

/**
 * QBM3U8RenditionGetType returns type of specified rendition.
 *
 * @param[in] self Rendition to get from
 * @return    type of Rendition
 **/
extern QBM3U8RenditionType
QBM3U8RenditionGetType(QBM3U8Rendition self);

/**
 * Get Group of specified rendition.
 *
 * @param[in] self Rendition to get from
 * @return         group ID of specified rendition.
 **/
extern SvString
QBM3U8RenditionGetGroupID(QBM3U8Rendition self);

/**
 * Get URI of specific Rendition.
 *
 * @param[in] self Rendition to get from
 * @return         URI of specific Rendition
 **/
extern SvURI
QBM3U8RenditionGetURI(QBM3U8Rendition self);

/**
 * Get Name of specific Rendition.
 *
 * @param[in] self Rendition to get from
 * @return         name of rendition
 **/
extern SvString
QBM3U8RenditionGetName(QBM3U8Rendition self);

/**
 * Get Language of specific Rendition.
 *
 * @param[in] self Rendition to get from
 * @return         language of rendition
 **/
extern SvString
QBM3U8RenditionGetLanguage(QBM3U8Rendition self);

/**
 * Get language associcated with specific Rendition.
 *
 * @param[in] self Rendition to get from
 * @return         language associcated with rendition
 **/
extern SvString
QBM3U8RenditionGetAssocLanguage(QBM3U8Rendition self);

/**
 * Get the "Default" parameter value of specific Rendition
 *
 * @param[in] self Rendition to get from
 * @return         value of the "Default" parameter
 **/
extern bool
QBM3U8RenditionGetDefault(QBM3U8Rendition self);

/**
 * Get the "Autoselect" parameter value of specific Rendition
 *
 * @param[in] self Rendition to get from
 * @return         value of the "Autoselect" parameter
 **/
extern bool
QBM3U8RenditionGetAutoselect(QBM3U8Rendition self);

/**
 * Get the "Forced" parameter value of specific Rendition
 *
 * @param[in] self Rendition to get from
 * @return         value of the "Forced" parameter
 **/
extern bool
QBM3U8RenditionGetForced(QBM3U8Rendition self);

/**
 * Get InstreamId parameter of a specific Rendition (only when TYPE of rendition
 * group is CLOSED-CAPTION).
 *
 * @param[in] self Rendition to get from
 * @return         value of InstreamId
 **/
extern SvString
QBM3U8RenditionGetInstreamId(QBM3U8Rendition self);

/**
 * Get Characteristics of s specific Rendition.
 *
 * @param[in] self Rendition to get from
 * @return         list of characteristics
 **/
extern SvString
QBM3U8RenditionGetCharacteristics(QBM3U8Rendition self);

/**
 * @}
 **/

#endif // QBM3U8RENDITION_H_
