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

#ifndef QB_UDP_READER_H
#define QB_UDP_READER_H

#include <fibers/c/fibers.h>
#include <ionet.h>
#include <SvPlayerKit/SvBufMan.h>

#ifdef __cplusplus
extern "C" {
#endif

struct svdataformat;
struct QBUdpReader;
typedef struct QBUdpReader* QBUdpReader;

struct QBUdpReaderHandlers
{
  void (*error)   (void* owner, QBUdpReader reader, int error);
  int  (*ready)   (void* owner, QBUdpReader reader, struct svdataformat* format);
  int  (*flushed) (void* owner, QBUdpReader reader);
  void (*eos)     (void* owner, QBUdpReader reader);
};

struct QBUdpReaderParams
{
  SvScheduler scheduler;
  const struct QBUdpReaderHandlers* handlers;
  void* owner;

  /// we don't own it, it is not destroyed.
  struct epoint_t* epoint;
  unsigned int maxpacketsize;
  /// non-null - if format->rtp_format is unknown, reader is to autodetect the format, and then report it with its "ready" handler.
  struct svdataformat* format;
};

struct QBUdpStartParams {
  short has_start_seq; // if non-zero, "start_seq" is valid.
  unsigned short start_seq;
};

struct QBUdpEosParams {
  short has_last_seq; // if non-zero, "last_seq" is valid.
  unsigned short last_seq;
  int timeout_ms;
};

/// could only fail on oom.
QBUdpReader QBUdpReaderCreate(const struct QBUdpReaderParams* params);
void QBUdpReaderDestroy(QBUdpReader udpr);
void QBUdpReaderSetBufMan(QBUdpReader udpr, SvBufMan bufMan);

/// \param params  can be null, meaning: use current settings.
void QBUdpReaderStart(QBUdpReader udpr, const struct QBUdpStartParams* params);
void QBUdpReaderPause(QBUdpReader udpr);

/// \param params  cannot be null.
void QBUdpReaderDetectEos(QBUdpReader udpr, const struct QBUdpEosParams* params);
void QBUdpReaderRevokeEos(QBUdpReader udpr);
/// \param timeoutMs  must be set to a non-negative number.
void QBUdpReaderFlush(QBUdpReader udpr, int timeoutMs);


/// \param ip  ip address of local interface.
/// \param port  if it's positive, it will be used, else, a random port will be chosen and returned here.
struct epoint_t* QBUdpCreateEpoint(const char* ip, unsigned short* port);
/// \param addr  must be valid.
/// \param port  must be valid.
struct epoint_t* QBUdpCreateEpointUnicast(netaddr* addr);
/// \param group  must be valid.
/// \param port  must be valid.
struct epoint_t* QBUdpCreateEpointMulticast(netaddr* group, int ttl);

/// For router punch-through.
/// \param remote_addr  address (with port set) of the remote socket.
void QBUdpReaderPunchThrough(QBUdpReader udpr, const netaddr* remote_addr);

void QBUdpReaderChangeEpoint_(QBUdpReader udpr, struct epoint_t* epoint);

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_UDP_READER_H
