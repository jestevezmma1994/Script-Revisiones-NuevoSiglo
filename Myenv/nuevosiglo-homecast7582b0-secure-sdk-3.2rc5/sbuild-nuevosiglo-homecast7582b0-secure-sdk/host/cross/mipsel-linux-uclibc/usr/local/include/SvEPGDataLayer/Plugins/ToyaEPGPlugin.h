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

#ifndef SV_EPG_TOYA_EPG_PLUGIN_H_
#define SV_EPG_TOYA_EPG_PLUGIN_H_

/**
 * @file ToyaEPGPlugin.h
 * @brief Toya EPG data plugin
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <QBTunerTypes.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * @defgroup ToyaEPGPlugin_ Toya EPG data plugin
 * @ingroup SvEPGPlugins
 **/

/**
 * @defgroup ToyaEPGPlugin Toya EPG data plugin class
 * @ingroup ToyaEPGPlugin_
 * @{
 **/

/**
 * Toya EPG data plugin class.
 **/
typedef struct ToyaEPGPlugin_ *ToyaEPGPlugin;


/**
 * Get runtime type identification object
 * representing Toya EPG data plugin class.
 *
 * @return Toya EPG data plugin class
 **/
extern SvType
ToyaEPGPlugin_getType(void);

/**
 * Initialize Toya EPG data plugin instance.
 *
 * @param[in] self      Toya EPG data plugin handle
 * @param[in] tunersCnt number of tuners to use
 * @param[in] PID       Conax EIT PID
 * @param[in] maxParsersCount max number of concurrently existing (working)
 *                      Conax EIT parsers, @c -1 to disable this limit
 * @param[in] maxBufferSize max size of buffer for received raw Conax EIT data
 *                      waiting to be parsed
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern ToyaEPGPlugin
ToyaEPGPluginInit(ToyaEPGPlugin self,
                  unsigned int tunersCnt,
                  unsigned int PID,
                  int maxParsersCount,
                  size_t maxBufferSize,
                  SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
