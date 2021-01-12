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

#ifndef MPEG_DESCRIPTORS_H
#define MPEG_DESCRIPTORS_H

// -----------------------------------
// MPEG section
// -----------------------------------

#define MPEG_DESC_TAG_VIDEO                     0x02
#define MPEG_DESC_TAG_AUDIO                     0x03
#define MPEG_DESC_TAG_REGISTRATION              0x05
#define MPEG_DESC_TAG_STREAM_ALIGNEMENT         0x06
#define MPEG_DESC_TAG_CA                        0x09
#define MPEG_DESC_TAG_LANGUAGE                  0x0a
#define MPEG_DESC_TAG_SMOOTHING_BUFFER          0x10
#define MPEG_DESC_TAG_SYSTEM_CLOCK              0x0b
#define MPEG_DESC_TAG_MULTIPLEX_UTILIZATION     0x0c
#define MPEG_DESC_TAG_MAX_BITRATE               0x0e
#define MPEG_DESC_TAG_PRIVATE_DATA_INDICATOR    0x0f
#define MPEG_DESC_TAG_STD                       0x11
#define MPEG_DESC_TAG_MPEG4_AUDIO               0x1c
#define MPEG_DESC_TAG_FLEX_MUX_TIMING           0x24
#define MPEG_DESC_TAG_METADATA_POINTER          0x25
#define MPEG_DESC_TAG_METADATA                  0x26
#define MPEG_DESC_TAG_METADATA_STD              0x27
#define MPEG_DESC_TAG_AVC_VIDEO                 0x28
#define MPEG_DESC_TAG_AVC_TIMING_HRD            0x2A


// -----------------------------------
// DVB section
// -----------------------------------

#define DVB_DESC_TAG_NETWORK_NAME                     0x40
#define DVB_DESC_TAG_SERVICE_LIST                     0x41
#define DVB_DESC_TAG_SATELLITE_DELIVERY_SYSTEM        0x43
#define DVB_DESC_TAG_CABLE_DELIVERY_SYSTEM            0x44
#define DVB_DESC_TAG_VBI_DATA                         0x45
#define DVB_DESC_TAG_BOUQUET_NAME                     0x47
#define DVB_DESC_TAG_SERVICE                          0x48
#define DVB_DESC_TAG_COUNTRY_AVAILABILITY             0x49
#define DVB_DESC_TAG_LINKAGE                          0x4A

// name and short decription of event
#define DVB_DESC_TAG_SHORT_EVENT                      0x4d
// long description of event
#define DVB_DESC_TAG_EXTENDED_EVENT                   0x4e

#define DVB_DESC_TAG_COMPONENT                        0x50
#define DVB_DESC_TAG_STREAM_IDENTIFIER                0x52
#define DVB_DESC_TAG_CA_IDENTIFIER                    0x53

// type of content, e.g. drama/news, etc.
#define DVB_DESC_TAG_CONTENT                          0x54
// parental rating for specific language(s)
#define DVB_DESC_TAG_PARENTAL_RATING                  0x55

#define DVB_DESC_TAG_TELETEXT                         0x56
#define DVB_DESC_TAG_LOCAL_TIME_OFFSET                0x58
#define DVB_DESC_TAG_SUBTITLE                         0x59
#define DVB_DESC_TAG_TERRESTRIAL_DELIVERY_SYSTEM      0x5a
#define DVB_DESC_TAG_PRIVATE_DATA_SPECIFIER           0x5f
#define DVB_DESC_TAG_FREQUENCY_LIST                   0x62
#define DVB_DESC_TAG_DATA_BROADCAST                   0x64
#define DVB_DESC_TAG_SCRAMBLING_DESCRIPTOR            0x65
#define DVB_DESC_TAG_DATA_BROADCAST_ID                0x66
#define DVB_DESC_TAG_AC3                              0x6a
#define DVB_DESC_TAG_DTS                              0x7b
#define DVB_DESC_TAG_EAC3                             0x7a
#define DVB_DESC_TAG_EXTENSION_DESCRIPTOR             0x7f
#define DVB_DESC_TAG_CLOSED_CAPTIONS                  0x86

// FIXME: dvb?
#define DVB_DESC_TAG_LOGICAL_CHANNEL_NUMBER          0x83

// FIXME: client specific
#define DVB_DESC_TAG_NORDIG_LOGICAL_CHANNEL_NUMBER    0x87
#define DVB_DESC_TAG_NORDIG_CONTENT_PROTECTION        0xa0

#define DVB_DESC_TAG_EXTENSION_IMGAE_ICON_DESCRIPTOR               0x00
#define DVB_DESC_TAG_EXTENSION_CPCM_DELIVERY_SIGNALLING_DESCRIPTOR 0x01
#define DVB_DESC_TAG_EXTENSION_CP_DESCRIPTOR                       0x02
#define DVB_DESC_TAG_EXTENSION_CP_IDENTIFIER_DESCRIPTOR            0x03
#define DVB_DESC_TAG_EXTENSION_T2_DELIVERY_SYSTEM_DESCRIPTOR       0x04
#define DVB_DESC_TAG_EXTENSION_SH_DELIVERY_SYSTEM_DESCRIPTOR       0x05
#define DVB_DESC_TAG_EXTENSION_SUPPLEMENTARY_AUDIO_DESCRIPTOR      0x06
#define DVB_DESC_TAG_EXTENSION_NETWORK_CHANGE_NOTIFY_DESCRIPTOR    0x07
#define DVB_DESC_TAG_EXTENSION_MESSAGE_DESCRIPTOR                  0x08
#define DVB_DESC_TAG_EXTENSION_TARGET_REGION_DESCRIPTOR            0x09
#define DVB_DESC_TAG_EXTENSION_TARGET_REGION_NAME_DESCRIPTOR       0x0a
#define DVB_DESC_TAG_EXTENSION_SERVICE_RELOCATED_DESCRIPTOR        0x0b

// -----------------------------------
// FIXME: section uknown
// -----------------------------------

// some kind of short description of track (audio only?)
#define UNK_DESC_TAG_TEXT_C5                          0xc5

#if 1
#define MPEG_DESC_TAG_HIERARCHICAL_TRANSMISSION       0xc0
#define MPEG_DESC_TAG_DIGITAL_COPY_CONTROL            0xc1
#define MPEG_DESC_TAG_NETWORK_IDENTIFICATION          0xc2
#define MPEG_DESC_TAG_PARTIAL_TRANSPORT_STREAM_TIME   0xc3
#define MPEG_DESC_TAG_AUDIO_COMPONENT                 0xc4
#define MPEG_DESC_TAG_HYPERLINK                       0xc5
#define MPEG_DESC_TAG_TARGET_AREA                     0xc6
#define MPEG_DESC_TAG_REFERENCE                       0xd1
#endif

#define MPEG_DESC_TAG_IP_DELIVERY_SYSTEM              0xbf

#define MPEG_DESC_TAG_AAC                             0x7c

#define MPEG_DESC_TAG_BROADCASTER_NAME                0xd8


#endif // #ifndef MPEG_DESCRIPTORS_H
