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

#ifndef SV_CONTENT_H_
#define SV_CONTENT_H_

/**
 * @file SvContent.h Content class API
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvURI.h>
#include <SvFoundation/SvURL.h>
#include "SvContentMetaData.h"
#include "SvContentChapter.h"
#include "SvDRMInfo.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvContent Content class
 * @ingroup SvPlayerKit
 * @{
 **/

/**
 * Content class.
 * @class SvContent
 * @extends SvObject
 **/
typedef struct SvContent_ *SvContent;


/**
 * Get runtime type identification object representing content class.
 *
 * @return content class
 **/
extern SvType
SvContent_getType(void);

/**
 * Create a content object.
 *
 * This method constructs a content object for the given URI.
 * The URI is not verified nor is the remote location accessed during
 * this operation. The content's meta-data is initially empty.
 *
 * @memberof SvContent
 *
 * @param[in] URI an URI of the content
 * @param[out] errorOut error info
 * @return created content object or @c NULL in case of error
 **/
extern SvContent
SvContentCreateFromURI(SvURI URI,
                       SvErrorInfo *errorOut);

/**
 * Create a content object with content URI passed as a C string.
 *
 * This method is a convenience wrapper for SvContentCreateFromURI().
 *
 * @memberof SvContent
 *
 * @param[in] uriCStr an URI string of the content
 * @param[out] errorOut error info
 * @return created content object or @c NULL in case of error
 **/
extern SvContent
SvContentCreateFromCString(const char *uriCStr,
                           SvErrorInfo *errorOut);

/**
 * Set content URI.
 *
 * @param[in] self content handle
 * @param[in] URI an URI of the content
 * @return none
 **/
extern void
SvContentSetURI(SvContent self,
                SvURI URI);

/**
 * Get the URI of a content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return URI of a content
 **/
extern SvURI
SvContentGetURI(SvContent self);

/**
 * @cond
 **/
static inline SvURL
SvContentGetURL(SvContent self)
{
    // FIXME: remove after deprecation period
    return SvContentGetURI(self);
}
/**
 * @endcond
 **/

/**
 * Get the metadata of a content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return metadata object handle
 **/
extern SvContentMetaData
SvContentGetMetaData(SvContent self);

/**
 * Set the metadata of a content.
 *
 * This method replaces previous metadata object of the content
 * with a new one.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] metaData metadata object handle
 **/
extern void
SvContentSetMetaData(SvContent self,
                     SvContentMetaData metaData);

/**
 * Get the resources attribute of a content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return resources object handle
 **/
extern SvContentMetaData
SvContentGetResources(SvContent self);

/**
 * Get the hints attribute of a content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return hints object handle
 **/
extern SvContentMetaData
SvContentGetHints(SvContent self);

/**
 * Get an array of content chapters.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return an array of content chapters, @c NULL if unavailable
 **/
extern SvImmutableArray
SvContentGetChapters(SvContent self);

/**
 * Get the DRM information object of a content.
 *
 * This method retrieves the DRM information object associated
 * with a content. DRM information object is not present
 * for unencrypted contents.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return a DRM information object handle
 **/
extern SvDRMInfo
SvContentGetDrmInfo(SvContent self);

/**
 * Set the DRM information object.
 *
 * This method replaces previous DRM information object of the content
 * with a new one.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] drmInfo DRM info object handle
 **/
extern void
SvContentSetDrmInfo(SvContent self,
                    SvDRMInfo drmInfo);

/**
 * Set the status of the no-skip constraint of this content.
 *
 * Alters the status of the no-skip constraint of the content object. The new
 * status might not be honored by the player if playback has already begun.
 * The constraint is used by the player to reject seeking, trick play flush
 * playback of this content. The constraint is also used by the playback
 * controller to reject switching to a different play list item.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] enabled the new value of the constraint
 **/
extern void
SvContentSetNoSkipConstraint(SvContent self,
                             bool enabled);

/**
 * Set the status of the no-pause constraint of this content.
 *
 * Alters the status of the no-pause constraint of the content object. The new
 * status might not be honored by the player if playback has already begun.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] enabled the new value of the constraint
 **/
extern void
SvContentSetNoPauseConstraint(SvContent self,
                              bool enabled);

/**
 * Set the status of the no-trick constraint of this content.
 *
 * Alters the status of the no-pause constraint of the content object. The new
 * status might not be honored by the player if playback has already begun.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] enabled the new value of the constraint
 **/
extern void
SvContentSetNoTrickConstraint(SvContent self,
                              bool enabled);

/**
 * Set the status of the play-once flag of this content.
 *
 * Alters the status of the play-once flag of the content object. If flag is set
 * content is removed from playlist after being played.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] enabled the new value of the constraint
 **/
extern void
SvContentSetPlayOnce(SvContent self,
                     bool enabled);

/**
 * Set the status of the back-jump-over flag of this content.
 *
 * Alters the status of the back-jump-over flag of the content object. If flag is set
 * content is ommited while jumping back.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] enabled the new value of the constraint
 **/
extern void
SvContentSetBackJumpOver(SvContent self,
                         bool enabled);

/**
 * Set the notification URI of this content.
 *
 * Replaces the notification URI of the content object. If set
 * request to URI is sent before content is played.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] notifyURI new notification URI
 **/
extern void
SvContentSetNotifyURI(SvContent self,
                      SvString notifyURI);

/**
 * Set the notification user agent of this content.
 *
 * Replaces the notification user agent of the content object. If NULL
 * default is used.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] userAgent new notification user agent
 **/
extern void
SvContentSetNotifyUserAgent(SvContent self,
                            SvString userAgent);

/**
 * Set the status of the time-range constraint of this content.
 *
 * This method alters the status of the time-range constraint of the content object.
 * The new status might not be honored by the player if playback has already begun.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] start the new value of the start position expressed in seconds
 * @param[in] stop the new value of the stop position expressed in seconds
 **/
extern void
SvContentSetTimeRangeConstraint(SvContent self,
                                double start,
                                double stop);

/**
 * Get the status of the no-skip constraint of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return current status of the constraint
 **/
extern bool
SvContentGetNoSkipConstraint(SvContent self);

/**
 * Get the status of the no-trick constraint of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return current status of the constraint
 **/
extern bool
SvContentGetNoTrickConstraint(SvContent self);

/**
 * Get the status of the no-pause constraint of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return current status of the constraint
 **/
extern bool
SvContentGetNoPauseConstraint(SvContent self);

/**
 * Get the status of the play-once flag of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return current status of the constraint
 **/
extern bool
SvContentGetPlayOnce(SvContent self);

/**
 * Get the status of the back-jump-over flag of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return current status of the flag
 **/
extern bool
SvContentGetBackJumpOver(SvContent self);

/**
 * Get the notification URI of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return current notification URI
 **/
extern SvString
SvContentGetNotifyURI(SvContent self);

/**
 * Get the notification URI of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return current user agent
 **/
extern SvString
SvContentGetNotifyUserAgent(SvContent self);

/**
 * Get the status of the time-range constraint of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[out] start a pointer used to store the current start position expressed in seconds
 * @param[out] stop a pointer used to store the current stop position expressed in seconds
 **/
extern void
SvContentGetTimeRangeConstraint(SvContent self,
                                double *start,
                                double *stop);

/**
 * Get the status of the auto-start constraint of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @return current status of auto-start constraint
 **/
extern bool
SvContentGetAutoStart(SvContent self);

/**
 * Set the status of the auto-start option of this content.
 *
 * @memberof SvContent
 *
 * @param[in] self content handle
 * @param[in] enabled the new value of the auto-start option
 **/
extern void
SvContentSetAutoStart(SvContent self,
                      bool enabled);

/**
 * @memberof SvContent
 **/
extern void
SvContentInvalidate(SvContent self,
                    bool invalid);

/**
 * @memberof SvContent
 **/
extern bool
SvContentIsValid(SvContent self);

/**
 * @memberof SvContent
 **/
extern void
SvContentAddChapter(SvContent self,
                    SvContentChapter chapter);

/**
 * @memberof SvContent
 **/
extern SvContentChapter
SvContentGetCurrentChapter(SvContent self,
                           double npt);

/**
 * @memberof SvContent
 **/
extern SvContentChapter
SvContentGetPreviousChapter(SvContent self,
                            double npt);

/**
 * @memberof SvContent
 **/
extern SvContentChapter
SvContentGetNextChapter(SvContent self,
                        double npt);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
