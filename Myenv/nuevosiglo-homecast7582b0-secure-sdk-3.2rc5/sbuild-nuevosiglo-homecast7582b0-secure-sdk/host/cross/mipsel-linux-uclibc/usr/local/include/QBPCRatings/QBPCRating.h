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

#ifndef QB_PC_RATINGS_H_
#define QB_PC_RATINGS_H_

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @file QBPCRating.h
 * @brief QBPCRating represents single parental control rating.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

/**
 * @defgroup PCRatings Parental control ratings.
 * @ingroup QBPCRating
 *
 * QBPCRating represents single rating of given standard.
 * QBPCRating has:
 * - standard: defines category (system) of rating. May be based on age or
 * some other conditions, e.g. presence of violence in given content.
 * - rating id: defines level of unsuitability
 * QBPCRating may be serialized (to SvString) and compared with other QBPCRatings.
 * @{
 **/

/**
 * Enumeration determining possible Parental Control standards
 * Don't change order.
 **/
typedef enum {
    QBPCRatingStandard_Unknown = -1, //!< unknown rating standard
    QBPCRatingStandard_FCC = 0, //!< FCC parental control rating system called "V-Chip", typically used for television
    QBPCRatingStandard_MPAA, //!< Motion Picture Association parental control system similar to "V-Chip" but used for movies
    QBPCRatingStandard_Mexico, //!< parental control system specific for Mexico
    QBPCRatingStandard_Cyprus, //!< parental control system specific for Cyprus
    QBPCRatingStandard_ViewRightIPTV,
    QBPCRatingStandard_BBFC, //!< British Board of Film Classification parental control system
    QBPCRatingStandard_age, //!< parental control system, where restrictions base on age
    QBPCRatingStandard_countryToAge, //!< parental control system, where restrictions base on age in specific country

    QBPCRatingStandard_MAX //!< count of supported rating standards
} QBPCRatingStandard;

#define QBRating_None 0

/**
 * Enumeration representing levels of FCC (V-Chip) parental control rating.
 **/
typedef enum {
    QBFCC_NR = QBRating_None,
    QBFCC_TVY,                  //!< Directed to All Children
    QBFCC_TVY7,                 //!< Directed to Older Children
    QBFCC_TVG,                  //!< General Audiences
    QBFCC_TVPG,                 //!< Parental Guidance Suggested
    QBFCC_TV14,                 //!<  Parents Strongly Cautioned
    QBFCC_TVMA,                 //!<  Mature Audience Only
    QBFCC_NRA,                  //!< Not Rated Adult
} QBFCCRating;

/**
 * Enumeration representing levels of MPAA parental control rating.
 **/
typedef enum {
    QBMPAA_NR = QBRating_None,
    QBMPAA_G,                 //!< General Audiences
    QBMPAA_PG,                //!< Parental Guidance Suggested
    QBMPAA_PG13,              //!< Parents Strongly Cautioned
    QBMPAA_R,                 //!< Restricted
    QBMPAA_NC17,              //!< No One 17 and Under Admitted
    QBMPAA_AO,                //!< Adults Only
    QBMPAA_X,                 //!< intended only for viewing by adults
    QBMPAA_XX,                //!< intended only for viewing by adults
    QBMPAA_XXX,               //!< intended only for viewing by adults
    /// Not Rated Adult
    QBMPAA_NRA                //!< QBMPAA_NRA
} QBMPAARating;

/**
 * Levels of extended DVB parental control rating for Mexico.
 **/
typedef enum {
    QBMexicoRating_None = QBRating_None,
    QBMexicoRating_AA = 0x45,           //!< Understandable for children under 7 years
    QBMexicoRating_A = 0x46,            //!< For all age groups
    QBMexicoRating_B = 0x47,            //!< For adolescents 12 years and older
    QBMexicoRating_B15 = 0x48,          //!< Not recommended for children under 15
    QBMexicoRating_C = 0x49,            //!< For adults 18 and older
    QBMexicoRating_D = 0x4A,            //!< Adult movies
} QBMexicoRating;

/**
 * Levels of extended DVB parental control rating for Cyprus.
 **/
typedef enum {
    QBCyprusRating_None = QBRating_None,
    QBCyprusRating_All = 0x01,          //!< allowed for all age groups
    QBCyprusRating_12 = 0x09,           //!< allowed for 12 years and older
    QBCyprusRating_15 = 0x0C,           //!< allowed for 15 years and older
    QBCyprusRating_18 = 0x0F,           //!< allowed for 18 years and older
    QBCyprusRating_AD = 0x42,           //!< intended only for viewing by adults
} QBCyprusRating;

typedef enum {
    QBViewRightIPTV_GeneralAudience = 0,        //!< only content suitable for general audience
    QBViewRightIPTV_AllowAllContent = 255,      //!< allow all content
} QBViewRightIPTVRating;

/**
 * Enumeration representing levels of BBFC parental control rating.
 **/
typedef enum {
    QBBBFC_NR = QBRating_None,
    QBBBFC_Universal,           //!< All aged admitted
    QBBBFC_ParentalGuidance,    //!< Parental Guidance Suggested
    QBBBFC_12A,                 //!< Suitable for people aged 12 or older
    QBBBFC_12,                  //!< Suitable for people aged 12 or older
    QBBBFC_15,                  //!< Suitable for people aged 15 or older
    QBBBFC_18,                  //!< Suitable for people aged 18 or older
    QBBBFC_Restricted18,        //!< Suitable for people aged 18 or older
} QBBBFCRating;

/**
 *  Enumeration representing possible results of comparing QBPCRatings.
 *  @see QBPCRatingCompareRatings
 *  Consider this example call: QBPCRatingCompareRatings(a, b, &error):
 */
typedef enum {
    QBPCRating_notComparable,   //!< ratings can't be compared (e.g. different standards, NULL ptr passed etc.)
    QBPCRating_Lower,           //!< rating b is greater than a
    QBPCRating_Equal,           //!< compared ratings are equal
    QBPCRating_Higher           //!< rating a is greater than b
} QBPCRating_ComparisonResult;

typedef struct QBPCRating_* QBPCRating;

/**
 * QBPCRating basic ctor
 *
 * @param[in] standardId rating standard
 * @param[in] ratingId rating value
 * @param[in] countryCode country prefix when using countryToAge standard, otherwise should be NULL
 * @return  pointer to created QBPCRating
 */
QBPCRating QBPCRatingCreate(QBPCRatingStandard standardId, int ratingId,
                            SvString countryCode, SvErrorInfo *errorOut);

/**
 * QBPCRating ctor converting string to rating
 *
 * @param[in] str handle to SvString including rating in format [STANDARD={VALUE}]
 * @return pointer to created QBPCRating
 */
QBPCRating QBPCRatingCreateFromString(SvString str, SvErrorInfo* errorOut);

/**
 * QBPCRating ctor converting standard and rating strings into QBPCRating
 *
 * @param[in]  standard    handle to SvString including standard (uppercase)
 * @param[in]  rating      handle to SvString including rating
 * @param[out] errorOut    error info
 * @return pointer to created QBPCRating
 */
QBPCRating
QBPCRatingCreateFromStandardAndRatingStrings(SvString standard, SvString rating, SvErrorInfo *errorOut);

/**
 * Getter function for rating standard
 *
 * @param[in] self QBPCRating handle
 * @return rating value
 */
QBPCRatingStandard QBPCRatingGetStandardId(QBPCRating self, SvErrorInfo* errorOut);

/**
 * Getter function for rating id (value)
 *
 * @param[in] self QBPCRating handle
 * @return rating value
 */
int QBPCRatingGetRatingId(QBPCRating self, SvErrorInfo* errorOut);

/**
 * Function creates SvString containing rating with standard name.
 *
 * @param[in] self QBPCRating handle
 * @return SvString containing rating name, NULL in case of error
 */
SvString QBPCRatingCreateRatingWithStandardName(QBPCRating self);

/**
 * Function creates SvString containing rating name only.
 *
 * @param[in] self QBPCRating handle
 * @return SvString containing rating name, NULL in case of error
 */
SvString QBPCRatingCreateRatingName(QBPCRating self);

/**
 * Function creates SvString containing country code.
 *
 * @param[in] self QBPCRating handle
 * @return SvString containing country code if QBPCRating is country to age type, NULL otherwise
 */
SvString QBPCRatingGetCountryCode(QBPCRating self);

/**
 * Function serializes rating into SvString
 *
 * @param[in] self QBPCRating handle
 * @return SvString with information about rating in format [STANDARD={RATING}]
 */
SvString QBPCRatingCreateStringRepresentation(QBPCRating self);

/**
 * Function compares two ratings.
 *
 * In general only ratings of same standards may be compared,
 * except age and country to age ratings which may be compared with each other.
 *
 * @param[in] self first rating to compare
 * @param[in] other second rating to compare
 * @param[out] string containing serialized rating's data
 * @return
 * - QBPCRating_Lower if self < other
 * - QBPCRating_Equal if self == other
 * - QBPCRating_Higher if self > other
 * - QBPCRating_NotComparable
 */
QBPCRating_ComparisonResult QBPCRatingCompareRatings(QBPCRating self, QBPCRating other,
                                                     SvErrorInfo* errorOut);

/**
 * Function returns ordinal number of rating id in given standard.
 *
 * Functions takes QBPCRating and identifies rating standard and id.
 * On that basis index of rating id in given standard is calculated.
 *
 * Function returns 0 for unknown rating id or -1 if NULL is passed as self.
 *
 * @param[in] self QBPCRating handle
 * @return index of rating id in given standard
 */
int QBPCRatingIndexInStandard(QBPCRating self);

/**
 * Function decides whether rating is rated as adult in standard
 * set for this rating.
 *
 * @param[in] ratingStr SvString handle
 * @return true if rating is rated as adult in given standard, otherwise false.
 */
bool QBPCRatingIsRatedAdult(SvString str);

/**
 * Function returns the runtime type identification
 * object of the QBPCRating class.
 *
 * @return QBPCRating type
 */
SvType QBPCRating_getType(void);
/**
 * @}
 **/
#endif // QBPCRATING_H
