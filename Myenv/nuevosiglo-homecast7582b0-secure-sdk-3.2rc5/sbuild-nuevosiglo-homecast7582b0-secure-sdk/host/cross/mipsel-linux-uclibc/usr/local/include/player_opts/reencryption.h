/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef PLAYER_OPTS_RENCRYPTION_H
#define PLAYER_OPTS_RENCRYPTION_H

/**
 * @file reencryption.h reencryption opts declarations
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup ReencryptionOpts Reencryption opt commands.
 * @ingroup autoplayer
 * @{
 **/

/**
 *  Set up multichannel which can be used for reencryption.
 */
#define PLAYER_OPT_REENCRYPTION_SET_MULTICHANNEL  "reencryption:set_multichannel"

/**
 *  Remove multichannel.
 */
#define PLAYER_OPT_REENCRYPTION_REMOVE_MULTICHANNEL  "reencryption:progress:remove_multichannel"

/**
 *  Add reencryption on pid channel.
 */
#define PLAYER_OPT_REENCRYPTION_ADD_PID_CHANNEL  "reencryption:add_pid_channel"

/**
 *  Remove reencryption on pid channel.
 */
#define PLAYER_OPT_REENCRYPTION_REMOVE_PID_CHANNEL  "reencryption:progress:remove_pid_channel"

/**
 *  Set up input filter.
 *  Reencryption sink need it to set filters on ECM's and inject new Cubiware ECM's and new dataformat to the stream.
 */
#define PLAYER_OPT_REENCRYPTION_ADD_INPUT_FILTER  "reencryption:add_input_filter"

/**
 *  Remove input filter.
 */
#define PLAYER_OPT_REENCRYPTION_REMOVE_INPUT_FILTER  "reencryption:remove_input_filter"

/**
* @}
*/

#endif // #ifndef PLAYER_OPTS_RENCRYPTION_H
