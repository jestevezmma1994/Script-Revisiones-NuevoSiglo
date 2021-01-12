/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_PVR_PACKET_HEADER_H
#define SV_PVR_PACKET_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

struct sv_pvr_packet_header
{
  unsigned int len;// : 24;
  //unsigned int stream_id : 8;
};


#define sv_static_assert( name, expr ) \
__attribute__((unused))  static char sv_static_assert__##name[ (expr) ? 1 : -1 ]

sv_static_assert(sv_pvr_packet_header__sizeof, sizeof(struct sv_pvr_packet_header) == 4);

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_PVR_PACKET_HEADER_H
