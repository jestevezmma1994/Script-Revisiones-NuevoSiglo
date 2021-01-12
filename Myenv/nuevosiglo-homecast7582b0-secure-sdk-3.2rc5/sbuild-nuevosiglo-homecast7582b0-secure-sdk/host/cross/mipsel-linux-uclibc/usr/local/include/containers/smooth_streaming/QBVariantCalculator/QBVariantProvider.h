/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_VARIANT_PROVIDER_H_
#define QB_VARIANT_PROVIDER_H_

/**
 * @file QBVariantProvider.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>

#include <stdbool.h>

/**
 * @defgroup QBVariantProviderIface interface
 * @ingroup CubiTV
 * @{
 **/

/**
 * QBVariantProviderIface interface.
 *
 * @interface QBVariantProviderIface QBVariantProvider.h
 **/
struct QBVariantProviderIface_ {
    /**
     * Check if provided variants are video streams
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     *
     * @return true if object provide video variants
     **/
    bool (*isVideo)(SvObject self);

    /**
     * Get bitrate of selected variant
     * If there is no seleted variant it will be a hint for next variant
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     *
     * @return bitrate in bps
     **/
    int (*getBitrate)(SvObject self);

    /**
     * Get bitrate of given variant
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     * @param[in] variant variant which we want to ask for bitrate
     *
     * @return bitrate in bps
     **/
    int (*getBitrateOfVariant)(SvObject self, int variant);

    /**
     * Get selected variant
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     *
     * @return currently selected variant
     **/
    int (*getVariant)(SvObject self);

    /**
     * Get amount of variants
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     *
     * @return number of provided variants
     **/
    int (*getVariantCount)(SvObject self);

    /**
     * Get current player position in seconds
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     *
     * @return player position in seconds
     **/
    double (*getPlayerPosition)(SvObject self);

    /**
     * Get buffer duration in seconds
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     *
     * @return buffer duration in seconds
     **/
    double (*getBufferDuration)(SvObject self);

    /**
     * Get ratio of bitrate to estimated bitrate
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     * @param[in] variant variant which we want to ask for bitrate
     *
     * @return bitrate/estimated bitrate
     **/
    double (*getRatioOfQualityLevel)(SvObject self, int variant);

    /**
     * Get speed
     *
     * It is messure of current bitrate
     * speed*bitrate is close to estimted bitrate.
     * f.e speed equals 2 means that currently bitrate is twice lower than estimated bitrate
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     *
     * @return measure of current speed
     */
    double (*getCurrentSpeed)(SvObject self);

    /**
     * Get selected chunk duration in seconds
     *
     * @param[in] self instance to an object implementing @ref QBVariantProviderIface
     *
     * @return chunk duration in seconds
     */
    double (*getChunkDuration)(SvObject self);
};
typedef struct QBVariantProviderIface_ *QBVariantProviderIface;

/**
 * Get runtime type identification object representing the QBVariantProviderIface interface.
 *
 * @relates QBVariantProviderIface
 *
 * @return QBVariantProviderIface type identification object
 **/
SvInterface
QBVariantProviderIface_getInterface(void);

/// @copydoc QBVariantProviderIface_::isVideo()
bool
QBVariantProviderIsVideo(SvObject self);

/// @copydoc QBVariantProviderIface_::getBitrate()
int
QBVariantProviderGetBitrate(SvObject self);

/// @copydoc QBVariantProviderIface_::getBitrateOfVariant()
int
QBVariantProviderGetBitrateOfVariant(SvObject self, int variant);

/// @copydoc QBVariantProviderIface_::getVariant()
int
QBVariantProviderGetVariant(SvObject self);

/// @copydoc QBVariantProviderIface_::getVariantCount()
int
QBVariantProviderGetVariantCount(SvObject self);

/// @copydoc QBVariantProviderIface_::getPlayerPosition()
double
QBVariantProviderGetPlayerPosition(SvObject self);

/// @copydoc QBVariantProviderIface_::getBufferDuration()
double
QBVariantProviderGetBufferDuration(SvObject self);

/// @copydoc QBVariantProviderIface_::getRatioOfQualityLevel()
double
QBVariantProviderGetRatioOfQualityLevel(SvObject self, int variant);

/// @copydoc QBVariantProviderIface_::getCurrentSpeed()
double
QBVariantProviderGetCurrentSpeed(SvObject self);

/// @copydoc QBVariantProviderIface_::getChunkDuration()
double
QBVariantProviderGetChunkDuration(SvObject self);

/**
 * @}
 **/

#endif // QB_VARIANT_PROVIDER_H_
