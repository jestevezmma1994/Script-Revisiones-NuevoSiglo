/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QBPCUTILS_H_
#define QBPCUTILS_H_

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <QBPCRatings/QBPCList.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @file QBPCUtils.h
 * @brief QBPCUtils contains utils functions for QBPCList
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

/**
 * @defgroup PCRatings Parental control ratings.
 * @ingroup QBPCUtils
 *
 * QBPCUtils contains utils functions for QBPCList.
 * @{
 **/

/**
 * Function serializes ratings list into SvString in readable format.
 *
 * Function similar to one from QBPCList, but creates output string in more
 * readable format, so it can be used e.g. as description directly.
 * @see QBPCListCreateLockedRatingsString
 *
 * Output format with flag printStandards==true:
 * Standard: r1, r2, r2 Standard2: r1, r2, r2
 *
 * Output format with flag printStandards==false:
 * r1, r2, r2, r1, r2, r2
 *
 * @param[in] list QBPCList handle
 * @param[in] printStandards indicate if returned string shoud contain ratings standard names
 * @param[out] errorOut error handle
 * @return output string containing information about ratings,
 * or NULL if there were no ratings to lock.
 */
SvString QBPCUtilsCreateDescriptionStringFromPCList(QBPCList list, bool printStandards, SvErrorInfo* errorOut);

/**
 * Function creates array of ratings from passed string.
 *
 * String with ratings data should have format:
 * [STANDARD1={RATING1, RATING2..}][STANDARD2={RATING1, RATING2..}]
 * If unknown standard / ratingID is detected function uses to
 * print that information and continues to look for another ratings.
 * If function doesn'y create any ratings returns empty array.
 *
 * @param str SvString with ratings data
 * @return SvArray of QBPCRating objects
 */
SvArray QBPCUtilsCreateRatingsFromString(SvString str);

/**
 * Function creates rating string from passed array of ratings.
 *
 * @param[in] ratings       ratings
 * @param[out] errorOut     error info
 * @return                  string representation of passed ratings array in the following format
 *                          [STANDARD1={RATING1, RATING2..}][STANDARD2={RATING1, RATING2..}]
 */
SvString QBPCUtilsCreateRatingsStringFromArray(SvArray ratings,
                                               SvErrorInfo *errorOut);

/**
 * Function crates normalized rating name. One rating can be represented by multiple
 * strings (e.g. NC17 and NC-17). Normalization allow rating identification by name comparation.
 *
 * Function expects SvString containing standard and rating name in form 'standard_rating'.
 *
 * @param[in]   str string with standard and rating name in form 'sandard_rating'
 * @return      string with standard and normalized rating name in form 'standard_rating', NULL in case of error
 */
SvString QBPCUtilsCreateCanonicalRatingName(SvString str);
/**
 * @}
 **/
#endif /* QBPCUTILS_H_ */
