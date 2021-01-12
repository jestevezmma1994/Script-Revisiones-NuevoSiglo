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

#ifndef PLAYER_HINTS_RTSP_INPUT_H
#define PLAYER_HINTS_RTSP_INPUT_H

/// Hint ot the rtsp input plugin, that it will be talking to the server with the name specified.
/// The rtsp input will skip OPTIONS query.
/// (Some servers do not support OPTIONS, or do not introduce themselves in OPTIONS response.)
///
/// Usage:
/// SvString server_type = SVSTRING("WMServer");
/// SvContentMetaDataSetStringProperty(SvContentGetHints(content), PLAYER_HINT__RTSP_INPUT_SERVER_TYPE, server_type);
#define PLAYER_HINT__RTSP_INPUT_SERVER_TYPE           "rtsp_input:server_type"

/// Same as above, but only used when we have both main server (setup server) and control server (play/pause server)
#define PLAYER_HINT__RTSP_INPUT_CONTROL_SERVER_TYPE   "rtsp_input:control_server_type"


/// Hint ot the rtsp input plugin, that it should be asking for given transport type.
/// Some servers support more than one transport type, but do not indicate which ones.
///
/// Usage:
/// SvString transport_type = SVSTRING(PLAYER_HINT__RTSP_INPUT_TRANSPORT_TYPE__DVB);
/// SvContentMetaDataSetStringProperty(SvContentGetHints(content), PLAYER_HINT__RTSP_INPUT_TRANSPORT_TYPE, transport_type);
#define PLAYER_HINT__RTSP_INPUT_TRANSPORT_TYPE        "rtsp_input:transport_type"

/// values to be used with PLAYER_HINT__RTSP_INPUT_TRANSPORT_TYPE
#define PLAYER_HINT__RTSP_INPUT_TRANSPORT_TYPE__DVB   "dvb"
#define PLAYER_HINT__RTSP_INPUT_TRANSPORT_TYPE__IP    "ip"


#endif // #ifndef PLAYER_HINTS_RTSP_INPUT_H
