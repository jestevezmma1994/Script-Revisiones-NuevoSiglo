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

#ifndef QBM3U8RENDITIONGROUP_H_
#define QBM3U8RENDITIONGROUP_H_

/**
 * @file QBM3U8RenditionGroup.h
 * @brief HLS Rendition Group API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/HLS/M3U8/QBM3U8Rendition.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBM3U8RenditionGroup HLS Rendition Group class
 * @ingroup QBM3U8Parser
 * @{
 **/

/**
 * Rendition Group class.
 * @class QBM3U8RenditionGroup.
 **/
typedef struct QBM3U8RenditionGroup_ *QBM3U8RenditionGroup;

/**
 * Get runtime type identification object representing
 * type of Rendition Group class.
 *
 * @return Rendition Group class
 *
 **/
extern SvType
QBM3U8RenditionGroup_getType(void);

/**
 * QBM3U8RenditionGroupGetType returns type of specified rendition group.
 *
 * @param[in] self Rendition Group to get from
 * @return         type of Rendition Group
 **/
extern QBM3U8RenditionType
QBM3U8RenditionGroupGetType(QBM3U8RenditionGroup self);

/**
 * QBM3U8RenditionGroupGetGroupID returns GroupID of specified rendition group.
 *
 * @param[in] self Rendition Group to get from
 * @return         group ID of specified rendition group.
 **/
extern SvString
QBM3U8RenditionGroupGetGroupID(QBM3U8RenditionGroup self);

/**
 * @}
 **/

#endif // QBM3U8RENDITIONGROUP_H_
