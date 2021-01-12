/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_EXT_SUBS_MANAGER_H
#define QB_EXT_SUBS_MANAGER_H

/**
 * @file QBExtSubsManager.h
 * @brief External subtitle manager api
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvPlayerManager/SvPlayerManager.h>
#include <QBSubsManager.h>
#include <dataformat/sv_data_format.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup ExtSubsManager QBExtSubsManager: External subtitle manager.
 *
 * Subtitles can be grouped into two categories by the way that they are stored,
 * one category contains subtitles stored in the container with the content and another
 * category contains subtitles stored in external files.
 * QBExtSubsManager provides support for subtitles from external files.
 *
 * Purpose of QBExtSubsManager is keeping track of current position in stream, pushing
 * subtitles from corresponding track to QBSubsManager in advance, providing callbacks
 * for decoding and displaying subtitles when requested by QBSubsManager.
 *
 * QBExtSubsFileFetcher is responsible for finding subtitle files that are associated
 * with the currently played content, it is checking if inside content file directory
 * exists subtitle files with the same filename as content but with one of the typical subtitle
 * files extensions (such as SRT and TXT).
 *
 * When some subtitle files are detected QBExtSubsFileParser is responsible for parsing subtitles
 * from that file to QBExtSubtitleRaw container and pushing them to corresponding QBExtSubsTrack.
 * Currently are supported following external subtitle formats: MicroDVD, SubRip, MPL2.
 * QBExtSubsTrack is returning correct subtitles with requested position in the stream.
 *
 **/

/**
 * @defgroup QBExtSubsManager QBExtSubsManager: External subtitle manager class.
 * @ingroup ExtSubsManager
 *
 * Purpose of QBExtSubsManager is keeping track of current position in stream, pushing
 * subtitles from corresponding track to QBSubsManager in advance, providing callbacks
 * for decoding and displaying subtitles when requested by QBSubsManager.
 *
 * @{
 **/

/**
 * External subtitle Manager class.
 * @class QBExtSubsManager
 * @extends SvObject
 **/
typedef struct QBExtSubsManager_s* QBExtSubsManager;

/**
 * Create QBExtSubsManager object.
 *
 * @memberof QBExtSubsManager
 *
 * This is a method that allocates and initializes
 * QBExtSubsManager object.
 *
 * @param[in] app             CUIT application context
 * @param[in] playerTask      SvPlayerTask used for getting current position in content
 * @param[in] format          struct svdataformat* used in QBExtSubsFileParser for getting content framerate
 * @param[in] subsManager     QBSubsManager handle responsible
 * @param[in] content         SvContent used for getting current content URI in QBExtSubsFileFetcher
 * @param[out] *errorOut      SvErrorInfo
 * @return                    QBExtSubsManager, NULL in case of error
 **/
extern QBExtSubsManager
QBExtSubsManagerCreate(SvApplication app, SvPlayerTask playerTask, struct svdataformat* format, QBSubsManager subsManager, SvContent content, SvErrorInfo *errorOut);

/**
 * Stop QBExtSubsManager object.
 *
 * @memberof QBExtSubsManager
 *
 * @param[in] self      external subtitles manager handle
 **/
void QBExtSubsManagerStop(QBExtSubsManager self);

/**
 * Set character encoding. Character encoding is given in iconv library format.
 *
 * @memberof QBExtSubsManager
 *
 * @param[in] self      external subtitles manager handle
 * @param[in] encoding  SvString with encoding type.
 **/
void QBExtSubsManagerSetEncoding(QBExtSubsManager self, SvString encoding);

/**
 * Get character encoding. Character encoding is given in iconv library format.
 *
 * @memberof QBExtSubsManager
 *
 * @param[in] self      external subtitles manager handle
 * @return              SvString with encoding type.
 **/
SvString QBExtSubsManagerGetEncoding(QBExtSubsManager self);

/**
 * @}
 **/

/**
 * @defgroup QBExtSubsTrack QBExtSubsTrack: External subtitle track class.
 * @ingroup ExtSubsManager
 *
 * QBExtSubsTrack is storing subtitles and is returning correct subtitles given current stream position
 *
 * @{
 **/

/**
 * External subtitle track class.
 * @class QBExtSubsTrack
 * @extends QBSubsTrack
 **/
struct QBExtSubsTrack_t;
typedef struct QBExtSubsTrack_t *QBExtSubsTrack;

/**
 * Get runtime type identification object representing QBExtSubsTrack class.
 *
 * @return External subtitle track class
 **/
SvType QBExtSubsTrack_getType(void);

/**
 * Get subtitle track type.
 *
 * @memberof QBExtSubsTrack
 *
 * @param[in] self      external subtitles track handle
 * @return              SvString with type
 **/
SvString QBExtSubsTrackGetSubtitleType(QBExtSubsTrack self);

/**
 * Get subtitle track filename.
 *
 * @memberof QBExtSubsTrack
 *
 * @param[in] self      external subtitles track handle
 * @return              SvString with filename
 **/
SvString QBExtSubsTrackGetSubtitleFilename(QBExtSubsTrack self);

/**
 * @}
 **/

#endif //QB_EXT_SUBS_MANAGER_H
