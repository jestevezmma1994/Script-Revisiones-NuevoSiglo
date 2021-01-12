/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DVB_MUXES_MAP_H_
#define QB_DVB_MUXES_MAP_H_

/**
 * @file QBDVBMuxesMap.h
 * @brief DVB upgrade muxes map class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDVBMuxesMap DVB upgrade muxes map class
 * @ingroup QBTunerLibrary
 * @{
 **/

#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDVBScanner.h>
#include <QBDVBMuxDesc.h>

/**
 * DVB upgrade muxes map class.
 * @class QBDVBMuxesMap
 * @extends SvObject
 **/
typedef struct QBDVBMuxesMap_ *QBDVBMuxesMap;


/**
 * Get runtime type identification object
 * representing QBDVBMuxesMap class.
 *
 * @return QBDVBMuxesMap type identification object
 **/
extern SvType
QBDVBMuxesMap_getType(void);

/**
 * Create new DVB upgrade muxes map and load contents from
 * previously serialized representation stored in file.
 *
 * @memberof QBDVBMuxesMap
 *
 * @param[in] srcPath   source file path, @c NULL to create empty map
 * @param[out] errorOut error info
 * @return              new muxes map or @c NULL in case of error
 **/
extern QBDVBMuxesMap
QBDVBMuxesMapCreate(const char *srcPath,
                    SvErrorInfo *errorOut);

/**
 * Serialize muxes map and write it to file.
 *
 * @memberof QBDVBMuxesMap
 *
 * @param[in] self      DVB upgrade muxes map
 * @param[in] destPath  destination file path
 * @param[out] errorOut error info
 **/
extern void
QBDVBMuxesMapSave(QBDVBMuxesMap self,
                  const char *destPath,
                  SvErrorInfo *errorOut);

/**
 * Update DVB upgrade muxes map from DVB scanner results.
 *
 * @memberof QBDVBMuxesMap
 *
 * @param[in] self      DVB upgrade muxes map
 * @param[in] scanner   DVB scanner handle
 * @param[out] errorOut error info
 * @return              new muxes map or @c NULL in case of error
 **/
extern void
QBDVBMuxesMapUpdate(QBDVBMuxesMap self,
                    QBDVBScanner *scanner,
                    SvErrorInfo *errorOut);

/**
 * Find single mux in a map.
 *
 * @memberof QBDVBMuxesMap
 *
 * @param[in] self      DVB upgrade muxes map
 * @param[in] TSID      TSID of the mux to find
 * @param[in] ONID      ONID of the mux to find, pass @c -1 to
 *                      use only @c TSID
 * @param[out] errorOut error info
 * @return              mux description, @c NULL if not found
 **/
extern QBDVBMuxDesc
QBDVBMuxesMapFindMux(QBDVBMuxesMap self,
                     unsigned int TSID, int ONID,
                     SvErrorInfo *errorOut);

/**
 * Create an array with all muxes from a map
 *
 * @memberof QBDVBMuxesMap
 *
 * @param[in] self      DVB upgrade muxes map
 * @param[out] errorOut error info
 * @return              array with all muxes from a map or @c in case of error
 *
 **/
extern SvArray
QBDVBMuxesMapCreateMuxesList(QBDVBMuxesMap self,
                             SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
