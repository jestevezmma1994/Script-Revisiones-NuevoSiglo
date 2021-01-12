/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SV_CONTENT_CONSTRAINTS_H_
#define SV_CONTENT_CONSTRAINTS_H_

/**
 * @file SvContentConstraints.h Content constraints class API
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvContentConstraints Content constraints class
 * @ingroup SvPlayerKit
 * @{
 **/

/**
 * Represents how a constraint should be applied
 **/
typedef enum {
    SvConstraintStatusEnable,             /**< switch ON */
    SvConstraintStatusTryEnable,          /**< try to switch ON, do not care about result */
    SvConstraintStatusDisable             /**< switch OFF */
} SvConstraintStatus;

/**
 * Represents on what kind of outputs a constraint should be applied
 **/
typedef enum {
    SvResolutionTypeHD,         /**< only HD output should be possible */
    SvResolutionTypeSD,         /**< only SD output should be possible */
    SvResolutionTypeBoth        /**< both HD and SD output should be possible */
} SvResolutionType;

/**
 * Content constraints class.
 * @class SvContentConstraints
 * @extends SvObject
 **/
typedef struct SvContentConstraints_ *SvContentConstraints;

/**
 * Create new content constraint object.
 *
 * @memberof SvContentConstraints
 *
 * @param[out] errorOut error info
 * @return created content constraints object, @c NULL in case of error
 **/
extern SvContentConstraints
SvContentConstraintsCreate(SvErrorInfo *errorOut);

/**
 * Create new content constraint object by merging existing two.
 *
 * @memberof SvContentConstraints
 * @qb_allocator
 *
 * @param[in] first content constraints handle
 * @param[in] second content constraints handle
 * @return new constraints object that have the strongest constraints
 *         out of the two passed, @c NULL in case of error
 **/
extern SvContentConstraints
SvContentConstraintsMerge(SvContentConstraints first,
                          SvContentConstraints second);

/*
 * Handling Macrovision status and level.
 */

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetMacrovision(SvContentConstraints self,
                                   SvConstraintStatus status,
                                   int level);

/**
 * @memberof SvContentConstraints
 **/
int
SvContentConstraintsGetMacrovisionLevel(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
SvConstraintStatus
SvContentConstraintsGetMacrovisionStatus(SvContentConstraints self);

/* Handling APS status and level */

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetAPS(SvContentConstraints self,
                           SvConstraintStatus status,
                           int level);

/**
 * @memberof SvContentConstraints
 **/
int
SvContentConstraintsGetAPSLevel(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
SvConstraintStatus
SvContentConstraintsGetAPSStatus(SvContentConstraints self);

/* Handling CGMS-A status and level */

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetCGMSA(SvContentConstraints self,
                             SvConstraintStatus status,
                             int level);

/**
 * @memberof SvContentConstraints
 **/
int
SvContentConstraintsGetCGMSALevel(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
SvConstraintStatus
SvContentConstraintsGetGGMSAStatus(SvContentConstraints self);

/* Handling SCMS status and level */

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetSCMS(SvContentConstraints self,
                            SvConstraintStatus status,
                            int level);

/**
 * @memberof SvContentConstraints
 **/
int
SvContentConstraintsGetSCMSLevel(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
SvConstraintStatus
SvContentConstraintsGetSCMSStatus(SvContentConstraints self);

/* Handling HDCP status */

/**
 * @memberof SvContentConstraints
 **/
SvConstraintStatus
SvContentConstraintsGetHDCP(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetHDCP(SvContentConstraints self,
                            SvConstraintStatus status);

/* Handling digitalVideo status */

/**
 * @memberof SvContentConstraints
 **/
SvConstraintStatus
SvContentConstraintsGetDigitalVideo(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetDigitalVideo(SvContentConstraints self,
                                    SvConstraintStatus status);

/* Handling analogTV status */

/**
 * @memberof SvContentConstraints
 **/
SvConstraintStatus
SvContentConstraintsGetAnalogTV(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetAnalogTV(SvContentConstraints self,
                                SvConstraintStatus status);

/* Handling S/PDIF status */

/**
 * @memberof SvContentConstraints
 **/
SvConstraintStatus
SvContentConstraintsGetSpdif(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetSpdif(SvContentConstraints self,
                             SvConstraintStatus status);

/* Handling Resolution settings */

/**
 * @memberof SvContentConstraints
 **/
SvResolutionType
SvContentConstraintsGetResolution(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetResolution(SvContentConstraints self,
                                  SvResolutionType type);

/*
 * Handling No-skip
 */

/**
 * @memberof SvContentConstraints
 **/
bool
SvContentConstraintsGetNoSkip(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetNoSkip(SvContentConstraints self,
                              bool noSkip);

/*
 * Handling time range setting
 */

/**
 * @memberof SvContentConstraints
 **/
bool
SvContentConstraintsGetTimeRange(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetTimeRange(SvContentConstraints self,
                                 bool timeRange);

/**
 * @memberof SvContentConstraints
 **/
double
SvContentConstraintsGetTimeRangeStart(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetTimeRangeStart(SvContentConstraints self,
                                      double timeRangeStart);

/**
 * @memberof SvContentConstraints
 **/
double
SvContentConstraintsGetTimeRangeEnd(SvContentConstraints self);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsSetTimeRangeEnd(SvContentConstraints self,
                                    double timeRangeEnd);

/**
 * Compare two content constraints objects.
 *
 * @memberof SvContentConstraints
 * @deprecated Use SvObjectEquals() instead.
 *
 * @param[in] self content constraints handle
 * @param[in] other content constraints to compare with @a self
 * @return @c true if constraints are equal
 **/
bool
SvContentConstraintsEqual(SvContentConstraints self,
                          SvContentConstraints other);

/**
 * @memberof SvContentConstraints
 **/
void
SvContentConstraintsDisplay(SvContentConstraints self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
