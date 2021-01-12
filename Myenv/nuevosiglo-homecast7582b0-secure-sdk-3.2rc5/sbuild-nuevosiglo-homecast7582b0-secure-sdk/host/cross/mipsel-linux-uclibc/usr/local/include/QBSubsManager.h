/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
****************************************************************************/

#ifndef QBSUBSMANAGER_H_
#define QBSUBSMANAGER_H_

/**
 * @file QBSubsManager.h
 * @brief Subtitle manager api
 **/

#include <stdint.h>
#include <stdbool.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <CUIT/Core/types.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvTextTypes.h>
#include <QBDataModel3/QBActiveArray.h>
#include <SvFoundation/SvDeque.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>


/**
 * @defgroup QBSubsManager Subtitle manager api
 * @ingroup QBSubsManager
 * @{
 **/
typedef enum {
    QBSubsTrackType_subtitles = 0,
    QBSubsTrackType_closedCaptions,
} QBSubsTrackType;

/**
 *  Manual subtitle parameters.
 **/
typedef struct QBSubsManualFontParams_ {
    bool useColor;
    SvColor color;
    bool useSize;
    unsigned int size;
    SvFontStyle style;
} *QBSubsManualFontParams;

struct QBSubsManagerSubtitleInterface_ {
    void (*decode)(SvObject self_);
    SvWidget (*show)(SvObject self_, SvWidget parent);
    void (*remove)(SvObject self_);
};

typedef struct QBSubsManagerSubtitleInterface_ *QBSubsManagerSubtitleInterface;

typedef struct QBSubsManagerSubtitle_ {
    struct SvObject_ super_;
    int64_t pts;
    int ptsBits;

    int64_t duration; //display duration in 90k

    bool preClear; //clear all subtitles before show
    bool immediate; //ignore pts and show subtitle immediate
} *QBSubsManagerSubtitle;

typedef const struct QBSubsManagerSubtitleVTable_ {
    struct SvObjectVTable_ super_;
    void (*decode)(QBSubsManagerSubtitle self_);
    SvWidget (*show)(QBSubsManagerSubtitle self_, SvWidget parent, SvDeque subtitlesDisplayed);
    void (*remove)(QBSubsManagerSubtitle self_);
} *QBSubsManagerSubtitleVTable;

/**
 * QBSubsManagerPluginInterface interface.
 **/
typedef struct QBSubsManagerPluginInterface_ {
    /**
     * Setup QBSubsManager plugin with manual params.
     *
     * @param[in] self_     plugin handle
     * @param[in] params    QBSubsManualFontParams handle containing manual params
     **/
    void (*setup)(SvObject self_, QBSubsManualFontParams params);
} *QBSubsManagerPluginInterface;

/**
 * Get runtime type identification object representing
 * QBSubsManagerPluginInterface interface.
 **/
SvInterface QBSubsManagerPluginInterface_getInterface(void);

SvType QBSubsManagerSubtitle_getType(void);

static inline void QBSubsManagerSubtitleDecode(QBSubsManagerSubtitle self)
{
    SvInvokeVirtual(QBSubsManagerSubtitle, self, decode);
}

static inline SvWidget QBSubsManagerSubtitleShow(QBSubsManagerSubtitle self, SvWidget parent, SvDeque subtitlesDisplayed)
{
    return SvInvokeVirtual(QBSubsManagerSubtitle, self, show, parent, subtitlesDisplayed);
}

static inline void QBSubsManagerSubtitleRemove(QBSubsManagerSubtitle self)
{
    SvInvokeVirtual(QBSubsManagerSubtitle, self, remove);
}

struct QBSubsTrack_t {
    struct SvObject_ super_;
    SvString langCode;
    unsigned int id;
};
typedef struct QBSubsTrack_t *QBSubsTrack;

SvType QBSubsTrack_getType(void);

typedef struct QBSubsManager_ *QBSubsManager;

QBSubsManager QBSubsManagerNew(SvApplication app);

/**
 * Register subtitles track with given id and notify observers.
 *
 * @note The id should be the same as id of a SvPlayerTrack corresponding to the track you are registering.
 *
 * @param[in] self          QBSubsManager handle
 * @param[in] track         track to be registered
 * @param[in] id            id to be assigned to the track
 * @return                  self or @c NULL if the track has already been registered
 */
QBSubsTrack QBSubsManagerRegisterTrack(QBSubsManager self, QBSubsTrack track, unsigned int id);

/**
 * Register subtitles track of provided type with given id.
 *
 * @note The id should be the same as id of a SvPlayerTrack corresponding to the track you are registering.
 *
 * @param[in] self           QBSubsManager handle
 * @param[in] track          track to be registered
 * @param[in] type           type of subtitle track (regular subtitles / closed captions)
 * @param[in] id             id to be assigned to the track
 * @return                   self or @c NULL if the track has already been registered
 */
QBSubsTrack QBSubsManagerRegisterTrackExt(QBSubsManager self, QBSubsTrack track, QBSubsTrackType type, unsigned int id);

void QBSubsManagerUnregisterTrack(QBSubsManager self, QBSubsTrack track);
void QBSubsManagerAddSubtitleForTrack(QBSubsManager self, QBSubsTrack track, QBSubsManagerSubtitle subtitle);
void QBSubsManagerFlushSubtitleForTrack(QBSubsManager self, QBSubsTrack track);

QBActiveArray QBSubsManagerGetAllTracks(QBSubsManager self);
QBActiveArray QBSubsManagerGetClosedCaptionsTracks(QBSubsManager self);
void QBSubsManagerSetCurrentTrack(QBSubsManager self, QBSubsTrack track);
QBSubsTrack QBSubsManagerGetCurrentTrack(QBSubsManager self);

void QBSubsManagerSetPlayerTask(QBSubsManager self, SvPlayerTask playerTask);

/**
 * Get current player task.
 *
 * @param[in] self          QBSubsManager handle
 * @return                  current player task
 */
SvPlayerTask QBSubsManagerGetPlayerTask(QBSubsManager self);

void QBSubsManagerAttach(QBSubsManager self, SvWidget parent, int x, int y, int w, int h, int level);

/**
 * Attach subtitles window to given parent CUIT widget.
 *
 * @param[in] self          QBSubsManager handle
 * @param[in] parent        parent widget handle
 * @param[in] xOffset       x coordinate of top-left corner of window (parent relative)
 * @param[in] yOffset       y coordinate of top-left corner of window (parent relative)
 * @param[in] width         new width of subtitles window
 * @param[in] height        new height of subtitles window
 * @param[in] level         Z-order of the widget (higher number = closer to viewer)
 **/
void
QBSubsManagerAttachToQBCUITWidget(QBSubsManager self,
                                  QBCUITWidget parent,
                                  int xOffset, int yOffset,
                                  unsigned int width, unsigned int height,
                                  unsigned short int level);

void QBSubsManagerDetach(QBSubsManager self);

QBSubsTrack QBSubsManagerGetNullTrack(QBSubsManager self);
void QBSubsManagerSetMute(QBSubsManager self, bool mute);

void QBSubsManagerSetPreferenceFunction(QBSubsManager self, void (*fun)(void *target, QBSubsManager manager), void *target);

/**
 * Add subtitles service listener.
 * @param[in]  self        QBSubsManager handle
 * @param[in]  listener    listener to be added
 * @param[out] errorOut    error handle
 **/
void
QBSubsManagerAddListener(QBSubsManager self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Remove subtitles service listener.
 *
 * @param[in]  self        QBSubsManager handle
 * @param[in]  listener    listener to be removed
 * @param[out] errorOut    error handle
 **/
void
QBSubsManagerRemoveListener(QBSubsManager self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Subs manager listener.
 *
 * @interface QBSubsManagerListener QBSubsManager.h <QBSubsManager.h>
 **/
typedef const struct QBSubsManagerListener_ {
    /**
     * Called when subtitle track is added.
     *
     * @param[in] self          QBSubsManager listener handle
     * @param[in] subsTrack     subtitles track
     **/
    void (*trackAdded)(SvObject self, QBSubsTrack subsTrack);

    /**
     * Called when subtitle track is removed.
     *
     * @param[in] self          QBSubsManager listener handle
     * @param[in] subsTrack     subtitles track
     **/
    void (*trackRemoved)(SvObject self, QBSubsTrack subsTrack);

    /**
     * Called when subtitle track is set.
     *
     * @param[in] self          QBSubsManager listener handle
     * @param[in] subsTrack     subtitles track
     **/
    void (*trackSet)(SvObject self, QBSubsTrack subsTrack);
} *QBSubsManagerListener;

/**
 * Get runtime type identification object representing
 * QBSubsManagerListener interface.
 *
 * @relates QBSubsManagerListener
 *
 * @return QBSubsManagerListener runtime type identification object
 **/
SvInterface
QBSubsManagerListener_getInterface(void);

void QBSubsManagerSetCurrentTrackChangedCallback(QBSubsManager self, void (*callback)(void *callbackData, QBSubsTrack track), void *callbackData);

/* Add plugin to QBSubsManager.
 *
 * @param[in] self          QBSubsManager handle
 * @param[in] plugin        handle to an object implementing @ref QBSubsManagerPluginInterface
 * @param[out] errorOut     error info
 * @return                  @c 0 on success, @c -1 otherwise
 **/
int QBSubsManagerAddPlugin(QBSubsManager self, SvObject plugin, SvErrorInfo *errorOut);

/**
 * Remove plugin from QBSubsManager.
 *
 * @param[in] self          QBSubsManager handle
 * @param[in] plugin        handle to an object implementing @ref QBSubsManagerPluginInterface
 * @param[out] errorOut     error info
 * @return                  @c 0 on success, @c -1 otherwise
 **/
int QBSubsManagerRemovePlugin(QBSubsManager self, SvObject plugin, SvErrorInfo *errorOut);

/**
  * @}
  **/

#endif /* QBSUBSMANAGER_H_ */
