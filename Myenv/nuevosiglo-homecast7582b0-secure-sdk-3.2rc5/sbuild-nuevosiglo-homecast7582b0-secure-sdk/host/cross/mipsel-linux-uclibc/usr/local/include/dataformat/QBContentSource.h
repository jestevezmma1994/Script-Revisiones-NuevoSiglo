/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015-2016 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QB_CONTENT_SOURCE_H
#define QB_CONTENT_SOURCE_H

#include <QBTunerTypes.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file QBContentSource.h Content source class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 *
 *
 * @defgroup QBContentSource  QBContentSource class
 * @{
 * QBContentSource class
 *
 * Keeps information about source of received content
 *
 * @class QBContentSource
 */
typedef struct QBContentSource_ *QBContentSource;


/**
 * Type of the data source protocol.
 */
typedef enum QBContentSourceProtocolType_ {
    /** protocol type not initialized */
    QBContentSourceProtocolType_uninitialized = 0,
    /** using rtsp protocol */
    QBContentSourceProtocolType_rtsp,
    /** using hls protocol */
    QBContentSourceProtocolType_hls,
    /** using smooth streaming protocol */
    QBContentSourceProtocolType_smooth,
    /** other protocol */
    QBContentSourceProtocolType_other
} QBContentSourceProtocolType;


/**
 * Set protocol type
 *
 * @param[in] self content source instance handle
 * @param[in] type source protocol type
 */
void QBContentSourceSetProtocolType(QBContentSource self, QBContentSourceProtocolType type);

/**
 * Get protocol type
 *
 * @param[in] self content source instance handle
 * @return source protocol type
 */
QBContentSourceProtocolType QBContentSourceGetProtocolType(QBContentSource self);

/**
 * @class QBContentSourceUnknown
 *
 * Keeps information about source which main type is unknown
 *
 * extends QBContentSource
 */
typedef struct QBContentSourceUnknown_ *QBContentSourceUnknown;

/**
 * Get runtime specification of QBContentSourcePVR type
 *
 * @return QBContentSourcePVR runtime type specification
 */
SvType QBContentSourceUnknown_getType(void);

/**
 * Create new empty data source object of unknown type
 *
 * @return content source handle or NULL on failure
 */
QBContentSourceUnknown QBContentSourceUnknownCreate(void);

/**
 * @class QBContentSourceIP
 *
 * Keeps information about IP source
 *
 * extends QBContentSource
 */
typedef struct QBContentSourceIP_ *QBContentSourceIP;

/**
 * IP specific type
 */
typedef enum QBContentSourceIPSubType_ {
    /** IP type not initialized */
    QBContentSourceIPSubType_uninitialized = 0,
    /** using UDP unicast */
    QBContentSourceIPSubType_udp,
    /** using UDP multicast */
    QBContentSourceIPSubType_multicast,
    /** other e.g TCP */
    QBContentSourceIPSubType_other,
} QBContentSourceIPSubType;

/**
 * Get runtime specification of QBContentSourceIP type
 *
 * @return QBContentSourceTuner runtime type specification
 */
SvType QBContentSourceIP_getType(void);

/**
 * Create new empty data source object of ip type
 *
 * @return content source handle or NULL on failure
 */
QBContentSourceIP QBContentSourceIPCreate(void);

/**
 * Set IP sub type
 *
 * @param[in] self content source instance handle
 * @param[in] type IP specific type
 */
void QBContentSourceIPSetSubType(QBContentSourceIP self, QBContentSourceIPSubType type);

/**
 * Get IP sub type
 *
 * @param[in] self content source instance handle
 * @return IP specific type
 */
QBContentSourceIPSubType QBContentSourceIPGetSubType(QBContentSourceIP self);


/**
 * @class QBContentSourceTuner
 *
 * Keeps information about tuner source
 *
 * extends QBContentSource
 */
typedef struct QBContentSourceTuner_ *QBContentSourceTuner;

/**
 * Get runtime specification of QBContentSourceTuner type
 *
 * @return QBContentSourceTuner runtime type specification
 */
SvType QBContentSourceTuner_getType(void);

/**
 * Create new empty data source object of tuner type
 *
 * @return content source handle or NULL on failure
 */
QBContentSourceTuner QBContentSourceTunerCreate(void);

/**
 * Set Mux id
 *
 * @param[in] self content source instance handle
 * @param[in] muxId information where tuner is tunned
 */
void QBContentSourceTunerSetMuxId(QBContentSourceTuner self, QBTunerMuxIdObj *muxId);

/**
 * Get Mux id
 *
 *
 * @param[in] self content source instance handle
 * @return information where tuner is tunned
 */
QBTunerMuxIdObj *QBContentSourceTunerGetMuxId(QBContentSourceTuner self);

/**
 * Set tuner number
 *
 * @param[in] self content source instance handle
 * @param[in] tunerNum information which tuner is used
 */
void QBContentSourceTunerSetNum(QBContentSourceTuner self, int tunerNum);

/**
 * Get tuner number
 *
 * @param[in] self content source instance handle
 * @return information which tuner is used or -1 in case of uninitialized
 */
int QBContentSourceTunerGetNum(QBContentSourceTuner self);


/**
 * @class QBContentSourcePVR
 *
 * Keeps information about PVR source
 *
 * extends QBContentSource
 */
typedef struct QBContentSourcePVR_ *QBContentSourcePVR;

/**
 * PVR specific type
 */
typedef enum QBContentSourcePVRSubType_ {
    /** PVR type not initialized */
    QBContentSourcePVRSubType_uninitialized = 0,
    /** PVR */
    QBContentSourcePVRSubType_pvr,
    /** timeshift */
    QBContentSourcePVRSubType_timeshift,
} QBContentSourcePVRSubType;

/**
 * Get runtime specification of QBContentSourcePVR type
 *
 * @return QBContentSourcePVR runtime type specification
 */
SvType QBContentSourcePVR_getType(void);

/**
 * Create new empty data source object of tuner type
 *
 * @return content source handle or NULL on failure
 */
QBContentSourcePVR QBContentSourcePVRCreate(void);

/**
 * Set PVR sub type
 *
 * @param[in] self content source instance handle
 * @param[in] type PVR specific type
 */
void QBContentSourcePVRSetSubType(QBContentSourcePVR self, QBContentSourcePVRSubType type);

/**
 * Get PVR sub type
 *
 * @param[in] self content source instance handle
 * @return PVR specific type
 */
QBContentSourcePVRSubType QBContentSourcePVRGetSubType(QBContentSourcePVR self);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_CONTENT_SOURCE_H
