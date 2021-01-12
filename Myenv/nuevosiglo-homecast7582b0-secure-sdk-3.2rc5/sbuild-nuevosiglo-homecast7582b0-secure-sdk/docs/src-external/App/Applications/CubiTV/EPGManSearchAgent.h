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

#ifndef EPG_MAN_SEARCH_AGENT__H_
#define EPG_MAN_SEARCH_AGENT__H_

/**
 * @file EPGManSearchAgent.h EPG manager search agent API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <SvFoundation/SvGenericObject.h>

/**
 * @defgroup EPGManSearchAgent EPG manager search agent
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * EPG Manager Search Agent mode.
 **/
typedef enum {
    EPGManSearchAgentMode_normal,       ///< normal mode
    EPGManSearchAgentMode_titleOnly,    ///< find only in titles
    EPGManSearchAgentMode_findSimilar,  ///< find similar to given
    EPGManSearchAgentMode_findByType    ///< find with given event type
} EPGManSearchAgentMode;

/**
 * Create EPG Manager Search Agent.
 *
 * @param[in] appGlobals CubiTV app globals
 * @return              created agent, @c NULL in case of error
 **/
extern SvObject
EPGManSearchAgentCreate(AppGlobals appGlobals);

/**
 * Set EPG Manager Search Agent mode.
 *
 * @param[in] self_     EPG Manager Search Agent handle
 * @param[in] mode      mode to be set
 **/
extern void
EPGManSearchAgentSetMode(SvObject self_, EPGManSearchAgentMode mode);

/**
 * Set EPG Manager Search Agent category.
 *
 * @param[in] self_     EPG Manager Search Agent handle
 * @param[in] category  category to be set
 **/
extern void
EPGManSearchAgentSetCategory(SvObject self_, int category);

/**
 * Set EPG Manager Search Agent view.
 *
 * @param[in] self_     EPG Manager Search Agent handle
 * @param[in] view      view to be set
 **/
extern void
EPGManSearchAgentSetView(SvObject self_, SvObject view);

/**
 * Set EPG Manager Search Back limit for catchup events.
 * E.g. backLimit set for 3600 will cause events ending
 * within last hour will be included in search results.
 *
 * @param[in] self_     EPG Manager Search Agent handle
 * @param[in] limit     limit in seconds
 **/
extern void
EPGManSearchAgentSetSearchBackLimitSeconds(SvObject self_, time_t limit);
/**
 * @}
 **/

#endif
