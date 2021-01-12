/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely ad
** exclusively reserved to ad by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make ay other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange a appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_DATE_H_
#define QB_DATE_H_

/**
 * @file QBDate.h
 * @brief Date representation
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBDate Date representation
 * @ingroup SvCore
 * @{
 **/

/**
 * Date representation.
 **/
typedef struct QBDate_ {
    uint16_t year;      ///< year
    uint8_t mday;       ///< day of the month (in 1-31 range)
    uint8_t mon;        ///< months since January (in 0-11 range)
} QBDate;

/**
 * Construct date.
 *
 * @param[in] year      years since year 0
 * @param[in] mday      day of the month (in 1-31 range)
 * @param[in] mon       months since January (in 0-11 range)
 * @return              constructed date
 **/
static inline QBDate
QBDateConstruct(uint16_t year,
                uint8_t mday,
                uint8_t mon)
{
    return (QBDate) {
               .year = year,
               .mday = mday,
               .mon = mon,
    };
}

/**
 * Validate Gregorian calendar date with respect to Julian calendar being used
 * before 1582 year and date corrections in 1582.
 *
 * @param[in] date      date
 * @return              @c true if date is valid, @c false otherwise
 **/
static inline bool
QBDateValidate(const QBDate date)
{
    if (date.year == 0)
        return false;

    if (date.mon > 11)
        return false;

    if (date.mday == 0)
        return false;

    // julian calendar correction after reform
    if (date.year == 1582 &&
        date.mon == 9 &&
        (date.mday >= 5 &&
         date.mday <= 14)) {
        return false;
    }

    if (date.mon == 1) {
        if ((date.year % 4 != 0 ||
            // leap years correction after reform
            (date.year > 1582 && date.year % 100 == 0 && date.year % 400 != 0))
            && date.mday > 28) {
            return false;
        } else if (date.mday > 29) {
            // leap year
            return false;
        }
    } else if ((date.mon == 3 || // April
                date.mon == 5 || // June
                date.mon == 8 || // September
                date.mon == 10)  // November
               && date.mday > 30) {
        return false;
    } else if (date.mday > 31) {
        return false;
    }

    return true;
}

/**
 * Check if two dates are equal.
 *
 * @param[in] date     date
 * @param[in] other    other date
 * @return             @c true if dates are equal, @c false otherwise
 **/
static inline bool
QBDateEquals(const QBDate date, const QBDate other)
{
    return (date.mday == other.mday &&
            date.mon == other.mon &&
            date.year == other.year);
}

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_DATE_H_ */
