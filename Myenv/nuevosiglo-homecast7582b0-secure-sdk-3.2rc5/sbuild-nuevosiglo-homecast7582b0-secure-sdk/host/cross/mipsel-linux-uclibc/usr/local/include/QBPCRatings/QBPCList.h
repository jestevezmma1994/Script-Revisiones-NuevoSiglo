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
#ifndef QBPCLIST_H
#define QBPCLIST_H

#include <QBPCRatings/QBPCRating.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>
/**
 * @file QBPCList.h
 * @brief Parental controls ratings list
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

/**
 * @defgroup PCRatings Parental control ratings.
 *
 * Parental control ratings contain information about content of movies, TV programs, etc.
 * in context of it's suitability for younger audience.
 * Each rating has two fields:
 *  - standard (e.g. age or some system based on symbols)
 *  - rating id (information about level of inappropriateness)
 * Played movie (program, ..) has rating describing it's content.
 * One movie may have few ratings, but only one rating of each standard is allowed.
 * QBPCRating class represents single rating.
 *
 * User can block given ratings, that means each time content that has blocked rating
 * is going to be played the pin pop-up will appear. Content will be played only after
 * entering correct pin. All blocked ratings are stored in QBPCList class.
 *
 **/

/**
 * @defgroup PCRatings Parental control ratings.
 * @ingroup QBPCList
 *
 * QBPCList is container for ratings.
 * Ratings of each standard are kept in separate sub-list.
 * QBPCList allows:
 * - locking / unlocking / checking status of QBPCRatings
 * - enabling / disabling / clearing / checking status of standards
 * Ratings that are not present in list are considered to be unlocked.
 * @{
 **/

/**
 * Storage class for QBPCRatings.
 */
typedef struct QBPCList_* QBPCList;

/**
 * Function creates empty list.
 *
 * @param[out] errorOut - error handle
 * @return created QBPCList handle
 */
QBPCList QBPCListCreate(SvErrorInfo* errorOut);

/**
 * Function locks ratings from passed string.
 *
 * Function expects string containing information about ratings to lock.
 * Format should be like this: [STANDARD1={RATING1, RATING2..}][STANDARD2={RATING1, RATING2..}]...
 * Function creates ratings to lock from passed argument str and locks them in self.
 *
 * @param self[in] QBPCList handle
 * @param str[in] string containing ratings
 * @param errorOut[out] error handle
 */
void QBPCListLockRatingsFromString(QBPCList self, SvString str, SvErrorInfo* errorOut);

/**
 * Function creates string representing ratings locked in QBPCList.
 * Output string has format: [STANDARD1={RATING1, RATING2..}][STANDARD2={RATING1, RATING2..}]...
 * Function creates string, so user is responsible for releasing it.
 *
 * @param self[in] QBPCList handle
 * @param errorOut[out] error handle
 * @return created string with ratings
 */
SvString QBPCListCreateLockedRatingsString(QBPCList self, SvErrorInfo* errorOut);

/**
 * Function locks single rating in given list.
 *
 * @param self[in] QBPCList handle
 * @param rating[in] rating to lock
 * @param errorOut[out] error handle
 */
void QBPCListLockRating(QBPCList self, QBPCRating rating, SvErrorInfo* errorOut);

/**
 * Function unlocks single rating in given list.
 *
 * @param self[in] QBPCList handle
 * @param rating[in] rating to unlock
 * @param errorOut[out] error handle
 */
void QBPCListUnlockRating(QBPCList self, QBPCRating rating, SvErrorInfo* errorOut);

/**
 * Function checks if passed rating is locked in given list.
 *
 * @param self[in] QBPCList handle
 * @param rating[in] rating to check
 * @param errorOut[out] error handle
 * @return true if rating is locked, false otherwise
 */
bool QBPCListIsRatingLocked(QBPCList self, QBPCRating rating, SvErrorInfo* errorOut);

/**
 * Function unlocks all ratings of given standard in list.
 *
 * @param self[in] QBPCList handle
 * @param standard standard to clear
 * @param errorOut[out] error handle
 */
void QBPCListClearStandard(QBPCList self, QBPCRatingStandard standard, SvErrorInfo* errorOut);

/**
 * Function checks if all ratings of given standard are unlocked.
 *
 * @param self[in] QBPCList handle
 * @param standard standard to clear
 * @param errorOut[out] error handle
 */
bool QBPCListIsStandardCleared(QBPCList self, QBPCRatingStandard, SvErrorInfo* errorOut);

/**
 * Function creates array with ratings of given standard that are locked in passed list.
 * Function creates array, so user is responsible for releasing it.
 * Array contains QBPCRatings.
 *
 * @param self QBPCList handle
 * @param standard
 * @param errorOut[out] error handle
 * @return SvArray of locked QBPCRatings or NULL if there weren't any locked ratings
 */
SvArray QBPCListCreateLockedRatingsListByStandard(QBPCList self, QBPCRatingStandard standard, SvErrorInfo* errorOut);

/**
 * Function creates array with standards that are enabled in passed list.
 * Function creates array, so user is responsible for releasing it.
 * Array contains QBPCRatingStandards as integers.
 * @param self[in] QBPCList handle
 * @return SvArray of SvValues representing enabled standards
 */
SvArray QBPCListCreateUsedStandardsList(QBPCList self, SvErrorInfo* errorOut);
/**
 * @}
 **/
#endif // QBPCLIST_H
