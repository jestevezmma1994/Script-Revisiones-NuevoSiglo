/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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
 ** includes, but is not limited to
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef CAGE_PROPERTIES_H_
#define CAGE_PROPERTIES_H_

/**
 * @file CAGEProperties.h CAGE properties API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup CAGEProperites CAGE properties
 * @ingroup CAGE
 * @{
 *
 * CAGEProperties module provides set of functions that make it possible to set global parameters of CAGE
 * and get its current properties at any time.
 **/

/**
 * Set bitmap global size limit.
 *
 * @param[in] maxBitmapWidth     bitmap width global limit
 * @param[in] maxBitmapHeight    bitmap height global limit
 **/
extern void
CAGEPropertiesSetBitmapSizeLimit(unsigned int maxBitmapWidth, unsigned int maxBitmapHeight);

/**
 * Get bitmap global size limit.
 *
 * @param[out] maxBitmapWidth    bitmap width global limit
 * @param[out] maxBitmapHeight   bitmap height global limit
 **/
extern void CAGEPropertiesGetBitmapSizeLimit(unsigned int *maxBitmapWidth, unsigned int *maxBitmapHeight);


/**
 * Set default text paragraph implementation.
 *
 * @param[in] paragraphType      subclass of @ref SvAbstractTextParagraph
 *                               to be used as default text paragraph implementation
 * @param[out] errorOut          error info
 **/
extern void
CAGEPropertiesSetDefaultTextParagraphImplementation(SvType paragraphType,
                                                    SvErrorInfo *errorOut);

/**
 * Get default text paragraph implementation.
 *
 * @return                       default text paragraph implementation
 **/
extern SvType
CAGEPropertiesGetDefaultTextParagraphImplementation(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif

