/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBVODUTILS_H
#define QBVODUTILS_H

/**
 * @file QBVODUtils.h Utility functions operating one the VOD products
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <QBContentManager/QBContentCategory.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <QBApplicationController.h>
#include <main_decl.h>
#include <stdbool.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <SvDataBucket2/SvDBRawObject.h>

/**
 * @defgroup QBVODUtils Utility functions operating one the VOD products
 * @ingroup CubiTV_utils
 * @{
 **/

/**
 * Tell if the applications settings demand blocking adult products.
 * "Hide adult titles" and "Parental Control" states are checked.
 *
 * @param appGlobals    AppGlobals handle
 * @return              true if the product should be blocked
 */
bool
QBVODUtilsAdultShouldBeBlocked(AppGlobals appGlobals);

/**
 * Tell if parental control settings for that category demand
 * PIN checking.
 *
 * @param appGlobals    AppGlobals handle
 * @param category      QBContentCategory to be checked
 * @return              true if content category should be blocked
 */
bool
QBVODUtilsCategoryShouldBeBlocked(AppGlobals appGlobals, QBContentCategory category);

/**
 * @brief QBVODUtilsCategoryIsProtected
 * @param appGlobals    AppGlobals handle
 * @param category      QBContentCategory to be checked
 * @return              true if content category is protected in regard to Parental Control
 */
bool
QBVODUtilsCategoryIsProtected(AppGlobals appGlobals, QBContentCategory category);

/**
 * Create array of content category covers to be displayed. If cover is adult and should by
 * blocked then this cover is not returned in covers array.
 *
 * @param[in] appGlobals    appGlobals handle
 * @param[in] category      contact category handle
 * @param[in] checkAdult    if true when creating covers array and adult cover found it will be ommited
 * @return                  @c content category covers, @c NULL in case of error
 **/
SvArray
QBVODUtilsCreateArrayOfCategoryCovers(AppGlobals appGlobals, QBContentCategory category, bool checkAdult);

/**
 * Check if product is for adult
 *
 * @param product       SvDBRawObject to be checked
 * @return              true if product is for adult
 */
bool
QBVoDUtilsIsAdult(SvObject product);

/**
 * Pop all but one VOD related contexts.
 * This function is used for returning to the main VOD window.
 *
 * @param controller    application controller
 * @return              @c true if any contexts has been removed, @c false otherwise
 */
bool
QBVODUtilsPopVodContexts(QBApplicationController controller);

/**
 * Fraction of a movie that have to be watched in order to consider whole movie watched
 **/
#define QB_EVENT_FILTER_WATCHED_TRESHOLD (0.25)

/**
 *  Event reporter wrapper that will pair play and stop playback events and
 *  report appropriate events only if certain preconditions are met (for example
 *  at least 25% of the movie has been watched). This reporter bases its knowledge on
 *  bookmarks.
 **/
typedef struct QBEventReporterWatchedEventFilter_* QBEventReporterWatchedEventFilter;

/**
 * Create new reporter filter.
 *
 * @param[in] bookmarkManager   the bookmark manager for obtaining bookmarks of the movie
 * @param[in] movie             the movie that the reported events relate to
 * @param[in] eventReporter     actual reporter to use for event reporting
 * @param[out] errorOut         error output
 * @return                      new instance of event reporter filter or @c NULL in case of error;
 *
 **/
QBEventReporterWatchedEventFilter
QBEventReporterWatchedEventFilterCreate(QBBookmarkManager bookmarkManager,
                                        SvDBRawObject movie,
                                        SvObject eventReporter,
                                        SvErrorInfo* errorOut);

/**
 * Send secure log with selected movie's category information (in VOD menu)
 *
 * @param[in] id unique ID of selected category
 * @param[in] name selected category name
 * @param[in] level level in hierarchy (distance from root category of selected category)
 */
void
QBVoDUtilsLogMovieCategory(const SvString id, const SvString name, int level);

/**
 * Send secure log with movie details
 *
 * @param[in] movie object represented selected movie
 */
void
QBVoDUtilsLogMovieDetails(const SvDBRawObject movie);

/**
 * Send secure log with current and selected movie (from recommended movies) information
 *
 * @param[in] currentMovie object represented current movie
 * @param[in] recommendedMovie object represented selected movie (from recommended movies)
 */
void
QBVoDUtilsLogCurrentAndRecommendedMovieDetails(const SvDBRawObject currentMovie, const SvDBRawObject recommendedMovie);

/**
 * Send secure log information with movie and purchase details
 *
 * @param[in] movie object represented selected movie
 * @param[in] selectedOfferId ID of offer which was selected by user (there can be several offers for one movie)
 */
void
QBVoDUtilsLogMoviePurchase(const SvDBRawObject movie, const int selectedOfferId);


/**
 * @}
 **/

#endif // QBVODUTILS_H
