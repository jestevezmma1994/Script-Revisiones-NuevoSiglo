/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TS_PACKET_H
#define QB_TS_PACKET_H

#include <SvCore/SvCommonDefs.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


static inline int
QBTSPacketHeaderGetCC(const unsigned char* packetHeader)
{
    return packetHeader[3] & 0x0F;
}

static inline bool
QBTSPacketHeaderGetStartInd(const unsigned char* packetHeader)
{
    return (packetHeader[1] & 0x40) != 0;
}

static inline unsigned char
QBTSPacketHeaderGetScrambledFlags(const unsigned char* packetHeader)
{
    return packetHeader[3] >> 6;
}

static inline void
QBTSPacketHeaderClearScrambledFlags(unsigned char* packetHeader)
{
    packetHeader[3] &= 0x3f;
}

static inline int
QBTSPacketHeaderGetPid(const unsigned char* packetHeader)
{
    return ((packetHeader[1] << 8) | packetHeader[2]) & 0x1fff;
}


static inline unsigned char
QBTSPacketHeaderGetAdaptFieldControl(const unsigned char* packetHeader)
{
    return (packetHeader[3] >> 4) & 0x3;
}

static inline bool
QBTSPacketHeaderHasAdaptField(const unsigned char* packetHeader)
{
    return (packetHeader[3] & 0x20) != 0;
}

static inline unsigned char
QBTSPacketHeaderGetAdaptFieldLen(const unsigned char* packetHeader)
{
    return packetHeader[4];
}

static inline unsigned char
QBTSPacketHeaderGetAdaptFieldFlags(const unsigned char* packetHeader)
{
    return packetHeader[5] & 0x1f;
}

static inline const unsigned char*
QBTSPacketHeaderGetAdaptFieldOptionals(const unsigned char* packetHeader)
{
    return packetHeader + 6;
}

static inline bool
QBTSPacketHeaderHasPCR(const unsigned char* packetHeader)
{
    return (QBTSPacketHeaderGetAdaptFieldFlags(packetHeader) & 0x10) != 0
        && QBTSPacketHeaderGetAdaptFieldLen(packetHeader) >= 5;
}

static inline uint64_t
QBTSPacketHeaderGetPCR(const unsigned char* packetHeader)
{
    const unsigned char* optFields = QBTSPacketHeaderGetAdaptFieldOptionals(packetHeader);
    uint64_t res = (((uint32_t)optFields[0]) << 24)
                 | (((uint32_t)optFields[1]) << 16)
                 | (((uint32_t)optFields[2]) <<  8)
                 | (((uint32_t)optFields[3]) <<  0);
    res = (res << 1) | (optFields[4] >> 7);
    return res;
}


static inline bool
QBTSPacketHeaderHasPayload(const unsigned char* packetHeader)
{
    return (packetHeader[3] & 0x10) != 0;
}

static inline int
QBTSPacketHeaderGetPayloadOffset(const unsigned char* packetHeader)
{
    if (unlikely(!QBTSPacketHeaderHasPayload(packetHeader)))
        return -1; // no payload - this packet does not concern us

    if (likely(!QBTSPacketHeaderHasAdaptField(packetHeader)))
        return 4; // no adaptation fields

    // adaptation fields needed to be skipped
    unsigned int adapt_len = packetHeader[4];
    if (unlikely(adapt_len >= 183))
      return -1;

    return 4 + 1 + adapt_len;
}


static inline int
QBTSPacketPayloadGetOffsetPtr(const unsigned char* packetPayload)
{
    return (unsigned int)packetPayload[0];
}



static inline int
QBTSPacketPayloadGetSectionLen(const unsigned char* packetPayload)
{
    return ((packetPayload[1] & 0x0F) << 8) | packetPayload[2];
}

static inline int
QBTSPacketPayloadGetPesLen(const unsigned char* packetPayload)
{
    return (packetPayload[4] << 8) | packetPayload[5];
}


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_TS_PACKET_H
