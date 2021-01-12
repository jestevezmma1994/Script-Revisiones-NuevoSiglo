/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
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

#ifndef QB_RTP_MEDIA_DESC_H
#define QB_RTP_MEDIA_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

#define QBRTPMediaDescMaxEncodings  10

struct QBRTPMediaDesc_s
{
  /// address and port that we listen on for incoming media data
  char* address;
  int port;

  /// valid entries in the array below
  int cnt;

  /// encodings expected on above address:port - see RFC 3551
  /// e.g. 0 == PCMU/8000Hz
  /// e.g. 8 == PCMA/8000Hz
  int encodings[QBRTPMediaDescMaxEncodings];
};
typedef struct QBRTPMediaDesc_s* QBRTPMediaDesc;


QBRTPMediaDesc  QBRTPMediaDescCreate(const char* address, int port);
void  QBRTPMediaDescDestroy(QBRTPMediaDesc media);
QBRTPMediaDesc QBRTPMediaDescCopy(QBRTPMediaDesc media);

void  QBRTPMediaDescAddEncoding(QBRTPMediaDesc media, int encoding);
int QBRTPMediaDescHasEncoding(QBRTPMediaDesc media, int encoding);

char* QBRTPMediaDescToSDP(QBRTPMediaDesc media);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_RTP_MEDIA_DESC_H
