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
******************************************************************************/

/* App/Libraries/SvEPGDataLayer/SvEPGFileBackedView.h */

#ifndef SV_EPG_FILE_BACKED_VIEW_H_
#define SV_EPG_FILE_BACKED_VIEW_H_

/**
 * @file SvEPGFileBackedView.h
 * @brief EPG File Backed Channel View Class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGFileBackedView EPG file backed channel view class
 * @ingroup SvEPGChannelViews
 * @{
 **/

#include <SvEPGDataLayer/Views/SvEPGView.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * File backed EPG channel view class.
 *
 * File backed EPG channel view is a channel view, that uses the
 * list of channels from a text file for channel filtering.
 * It also updates the contents of the file to match changes
 * in the channels list done at runtime.
 **/
typedef struct SvEPGFileBackedView_ *SvEPGFileBackedView;


/**
 * Get runtime type identification object
 * representing SvEPGFileBackedView class.
 **/
extern SvType
SvEPGFileBackedView_getType(void);

/**
 * Initialize SvEPGFileBackedView instance.
 *
 * @param[in] self      file backed EPG channel view handle
 * @param[in] filter    channel filter (an object implementing
 *                      SvEPGChannelFilter inteface), can be NULL
 * @param[in] filePath  path to a file storing channels list
 * @param[in] ignoreMissingFile @c true to quietly ignore the fact
 *                      that file under @a filePath does not exist
 * @param[out] errorOut error info
 * @return              self or NULL in case of error
 **/
extern SvEPGFileBackedView
SvEPGFileBackedViewInit(SvEPGFileBackedView self,
                        SvGenericObject filter,
                        SvString filePath,
                        bool ignoreMissingFile,
                        SvEPGViewChannelNumbering numbering,
                        SvErrorInfo *errorOut);

/**
 * Get path to a file used for storing channels list.
 *
 * @param[in] self  file backed EPG channel view handle
 * @return              file path or NULL in case of error
 **/
extern SvString
SvEPGFileBackedViewGetFilePath(SvEPGFileBackedView self);

/**
 * Add a channel to the file backed channels list.
 *
 * @param[in] self      file backed EPG channel view handle
 * @param[in] channel   TV channel handle
 * @param[out] errorOut error info
 **/
extern void
SvEPGFileBackedViewAddChannel(SvEPGFileBackedView self,
                              SvTVChannel channel,
                              SvErrorInfo *errorOut);

/**
 * Remove channel from the file backed channels list.
 *
 * @param[in] self      EPG mapping channel view handle
 * @param[in] channelID EPG ID of a TV channel
 * @param[in] hide      true to hide channel, false to unhide
 * @param[out] errorOut error info
 **/
extern void
SvEPGFileBackedViewRemoveChannel(SvEPGFileBackedView self,
                                 SvTVChannel channel,
                                 SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif
