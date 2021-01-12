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

#ifndef EVENTISMOVIEINFO_H_
#define EVENTISMOVIEINFO_H_

#include <main_decl.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <SvDataBucket2/SvDBRawObject.h>

/**
 * Object provides location of Content provider logos.
 *
 * URI is formed as follows:
 * prefix + content provider name + suffix
 */
struct QBEventISMovieDynamicLogoProvider_ {
    ///super object
    struct SvObject_ super_;
    ///uri prefix
    SvString prefix;
    ///uri suffix
    SvString suffix;
};
typedef struct QBEventISMovieDynamicLogoProvider_* QBEventISMovieDynamicLogoProvider;

/**
 * Get runtime type identification object representing SvValue class.
 *
 * @return dynamic logo provider class
 **/
extern SvType
QBEventISMovieDynamicLogoProvider_getType(void);

void
QBEventISMovieInfoSetObject(SvWidget w, SvGenericObject object);

/**
 * Set dynamic logo provider description.
 *
 * No dynamic logo is used when provider is not set.
 *
 * @param[in] w        widget handle
 * @param[in] provider provider object handle
 */
void
QBEventISMovieInfoSetDynamicLogoProvider(SvWidget w, QBEventISMovieDynamicLogoProvider provider);

SvWidget
QBEventISMovieInfoCreate(AppGlobals appGlobals);
SvWidget
QBEventISMovieInfoCreateMoviePage(AppGlobals appGlobals);
SvString
QBEventISMovieParseElement(SvDBRawObject metadata, SvString tag, SvString defaultValue);
bool
QBEventISMovieParseElementAsBoolean(SvDBRawObject metadata, SvString tag, bool defaultValue);
int
QBEventISMovieParseElementAsInteger(SvDBRawObject metadata, SvString tag, int defaultValue);
SvString
QBEventISMovieGetThumbnailURI(SvDBRawObject metaData);

/**
 * Creates string using array of strings strings stored in assert meta data.
 *
 * @param[in] metadata     asset meta data handle
 * @param[in] tag          name of the meta data key
 * @param[in] elementTag   name of the meta data sub-key
 * @param[in] defaultValue default value returned when meta data is not available
 * @param[in] maxElements  maximum number of elements to be used formatting the string
 * @param[in] multiLine    @c true to allow multiline text
 * @return                 string representing complex meta data
 */
SvString
QBEventISMovieParseElementList(SvDBRawObject metadata, SvString tag, SvString elementTag, SvString defaultValue, const size_t maxElements, bool multiLine);

/**
 * Creates content of the label with price for not bought assets.
 *
 * @param[in] metadata asset meta data handle
 * @return             human readable string describing asset price
 */
SvString
QBEventISMovieInfoCreateNotEntitledPriceString(SvDBRawObject metadata);

/**
 * Creates content of the label with price for bought assets.
 *
 * Usually contains rent expiration date.
 *
 * @param[in] metadata asset meta data handle
 * @return             human readable string describing asset price
 */
SvString
QBEventISMovieInfoCreateEntitledPriceString(SvDBRawObject metadata);

/**
 * Creates content of the label with minimal age.
 *
 * @param[in] metadata asset meta data handle
 * @return             human readable string describing minimal age
 */
SvString
QBEventISMovieInfoCreatePCRatingString(SvDBRawObject metadata);

#endif /* EVENTISMOVIEINFO_H_ */
