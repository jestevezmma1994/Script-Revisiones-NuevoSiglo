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

#ifndef FAKE_EPG_PLUGIN_H_
#define FAKE_EPG_PLUGIN_H_

/**
 * @file FakeEPGPlugin.h
 * @brief Fake EPG plugin
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>


/**
 * @defgroup FakeEPGPlugin Fake EPG plugin
 * @ingroup SvEPGPlugins
 **/


/**
 * @defgroup FakeEPGChannelListPlugin Fake EPG channel list plugin class
 * @ingroup FakeEPGPlugin
 * @{
 **/

/**
 * Fake EPG channel list plugin class.
 **/
typedef struct FakeEPGChannelListPlugin_ *FakeEPGChannelListPlugin;


/**
 * Get runtime type identification object
 * representing fake EPG channel list plugin class.
 **/
extern SvType
FakeEPGChannelListPlugin_getType(void);

/**
 * Initialize fake EPG channel list plugin instance.
 *
 * @param[in] self      fake EPG channel list plugin handle
 * @param[in] cnt       channel count
 * @param[in] prefix    channel name prefix
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern FakeEPGChannelListPlugin
FakeEPGChannelListPluginInit(FakeEPGChannelListPlugin self,
                             unsigned int cnt,
                             SvString prefix,
                             SvErrorInfo *errorOut);

/**
 * Set channel list refresh period.
 *
 * @param[in] self      fake EPG channel list plugin handle
 * @param[in] period    channel list refresh period in milliseconds,
 *                      @c 0 to disable periodic updates
 * @param[out] errorOut error info
 **/
extern void
FakeEPGChannelListPluginSetRefreshPeriod(FakeEPGChannelListPlugin self,
                                         unsigned int period,
                                         SvErrorInfo *errorOut);

/**
 * @}
 **/


/**
 * @defgroup FakeEPGDataPlugin Fake EPG data plugin class
 * @ingroup FakeEPGPlugin
 * @{
 **/

/**
 * Fake EPG data plugin class.
 **/
typedef struct FakeEPGDataPlugin_ *FakeEPGDataPlugin;


/**
 * Get runtime type identification object
 * representing fake EPG data plugin class.
 **/
extern SvType
FakeEPGDataPlugin_getType(void);

/**
 * Initialize fake EPG data plugin instance.
 *
 * @param[in] self      fake EPG data plugin handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern FakeEPGDataPlugin
FakeEPGDataPluginInit(FakeEPGDataPlugin self,
                      SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
