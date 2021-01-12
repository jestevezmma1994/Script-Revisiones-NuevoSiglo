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

#include <Utils/QBVODUtils.h>

#include <Windows/QBVoDGridBrowser.h>
#include <Windows/Digitalsmiths/QBVoDMovieDetailsDS.h>
#include <Windows/QBVoDMovieDetails.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/QBAccessController/QBAccessControllerTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvObject.h>
#include <main.h>
#include <Utils/QBMovieUtils.h>
#include <QBSecureLogManager.h>
#include <SvDataBucket2/SvDBReference.h>
#include <QBJSONUtils.h>
#include <QBStringUtils.h>
#include <QBMWClient/QBMWClientEventReporter.h>
#include <SvCore/SvCoreErrorDomain.h>

bool
QBVODUtilsAdultShouldBeBlocked(AppGlobals appGlobals)
{
    return QBParentalControlLogicAdultIsBlocked(appGlobals->parentalControlLogic);
}

SvLocal SvString
QBVODUtilsGetCategoryRating(QBContentCategory category)
{
    SvString rating = NULL;
    SvValue ratingV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("ratings"));

    if (ratingV && SvObjectIsInstanceOf((SvObject) ratingV, SvValue_getType()) && SvValueIsString(ratingV)) {
        rating = SvValueGetString(ratingV);
    }
    return rating;
}

SvLocal bool
QBVODUtilsCategoryIsAdult(QBContentCategory category)
{
    SvValue isAdultV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("isAdult"));

    if (isAdultV && ((SvValueIsInteger(isAdultV) && SvValueGetInteger(isAdultV) != 0) || (SvValueIsBoolean(isAdultV) && SvValueGetBoolean(isAdultV)))) {
        return true;
    }

    return false;
}

bool
QBVODUtilsCategoryShouldBeBlocked(AppGlobals appGlobals, QBContentCategory category)
{
    if (QBVODUtilsCategoryIsAdult(category) && QBVODUtilsAdultShouldBeBlocked(appGlobals)) {
        return true;
    }

    SvString rating = QBVODUtilsGetCategoryRating(category);
    if (rating && !QBParentalControlRatingAllowed(appGlobals->pc, rating)) {
        return true;
    }

    return false;
}

bool
QBVODUtilsCategoryIsProtected(AppGlobals appGlobals, QBContentCategory category)
{
    if (QBVODUtilsCategoryIsAdult(category)) {
        return true;
    }

    SvString rating = QBVODUtilsGetCategoryRating(category);
    if (rating && !QBParentalControlRatingAllowed(appGlobals->pc, rating)) {
        return true;
    }

    return false;
}

SvArray QBVODUtilsCreateArrayOfCategoryCovers(AppGlobals appGlobals, QBContentCategory category, bool checkAdult)
{
    SvArray covers = NULL;

    if (!appGlobals || !category) {
        goto fini;
    }

    SvObject object = QBContentCategoryGetAttribute(category, SVSTRING("category_products"));
    if (object && SvObjectIsInstanceOf(object, SvArray_getType())) {
        SvArray tmpCovers = SvArrayCreate(NULL);
        SvArray products = (SvArray) object;
        const size_t count = SvArrayCount(products);

        for (size_t i = 0; i < count; i++) {
            SvObject product = SvArrayObjectAtIndex(products, i);

            if (!product || !SvObjectIsInstanceOf(product, SvHashTable_getType())) {
                continue;
            }

            SvObject isAdult = SvHashTableFind((SvHashTable) product, (SvObject) SVSTRING("is_adult"));
            SvObject coverUrl = SvHashTableFind((SvHashTable) product, (SvObject) SVSTRING("cover_url"));

            if (!isAdult || !SvObjectIsInstanceOf(isAdult, SvValue_getType()) || !SvValueIsInteger((SvValue) isAdult) ||
                !coverUrl || !SvObjectIsInstanceOf(coverUrl, SvValue_getType()) || !SvValueIsString((SvValue) coverUrl)) {
                continue;
            }

            if (checkAdult && SvValueGetInteger((SvValue) isAdult) && QBVODUtilsAdultShouldBeBlocked(appGlobals)) {
                continue;
            }

            SvArrayAddObject(tmpCovers, coverUrl);
        }

        covers = tmpCovers;
    } else {
        object = QBContentCategoryGetAttribute(category, SVSTRING("covers_array"));
        if (object && SvObjectIsInstanceOf(object, SvArray_getType())) {
            covers = (SvArray) SVRETAIN(object);
        }
    }

fini:
    return covers;
}

bool
QBVoDUtilsIsAdult(SvObject product)
{
    if (!product)
        return true;

    if (SvObjectIsInstanceOf(product, SvDBRawObject_getType())) {
        SvValue attr = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) product, "belongs_to_adult_category");
        if (attr && ((SvValueIsInteger(attr) && SvValueGetInteger(attr) != 0) || (SvValueIsBoolean(attr) && SvValueGetBoolean(attr))))
            return true;

        attr = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) product, "is_adult");
        if (attr && ((SvValueIsInteger(attr) && SvValueGetInteger(attr) != 0) || (SvValueIsBoolean(attr) && SvValueGetBoolean(attr))))
            return true;
    } else if (SvObjectIsInstanceOf(product, QBContentCategory_getType())) {
        return QBVODUtilsCategoryIsAdult((QBContentCategory) product);
    }

    return false;
}

bool
QBVODUtilsPopVodContexts(QBApplicationController controller)
{
    SvArray contextStack = QBApplicationControllerGetContextStack(controller);
    if (!contextStack) {
        return false;
    }
    SvIterator iter = SvArrayGetReverseIterator(contextStack);
    SvObject ctx = NULL;
    ssize_t count = 0;
    SvObject lastVodContext = NULL;
    while ((ctx = SvIteratorGetNext(&iter)) != NULL &&
           (SvObjectIsInstanceOf(ctx, QBVoDGridBrowserContext_getType()) ||
            SvObjectIsInstanceOf(ctx, QBVoDMovieDetailsDSContext_getType()) ||
            SvObjectIsInstanceOf(ctx, QBVoDMovieDetailsContext_getType()))) {
        ++count;
        lastVodContext = ctx;
    }
    if (lastVodContext && SvObjectIsInstanceOf(lastVodContext, QBVoDGridBrowserContext_getType())) {
        QBVoDGridBrowserContextReset((QBVoDGridBrowserContext) lastVodContext);
    }
    for (ssize_t i = 0; i < count - 1; ++i) {
        QBApplicationControllerPopContext(controller);
    }
    SVRELEASE(contextStack);
    return (count > 1);
}

void
QBVoDUtilsLogMovieCategory(const SvString id, const SvString name, int level)
{
    char *escapedId = QBStringCreateJSONEscapedString(id ? SvStringCString(id) : "");
    char *escapedName = QBStringCreateJSONEscapedString(name ? SvStringCString(name) : "");

    QBSecureLogEvent("QBVoDUtils",
                     "Notice.Vod.MovieCategorySelected",
                     "JSON:{\"id\":\"%s\",\"name\":\"%s\",\"level\":\"%d\"}",
                     escapedId,
                     escapedName,
                     level);

    free(escapedId);
    free(escapedName);
}

SvLocal SvString
QBVoDUtilsCreateJSONWithMovieDetails(const SvDBRawObject movie)
{
    assert(movie);

    // Creates JSON string in such format: "id":"MOVIE_ID","name":"MOVIE_NAME","runtime":"MOVIE_RUNTIME","rating":"MOVIE_RATING"

    SvString id = NULL;
    const SvValue idValue = SvDBObjectGetID((SvDBObject) movie);
    if (idValue && SvObjectIsInstanceOf((SvObject) idValue, SvValue_getType()) && SvValueIsString(idValue))
        id = SvValueGetString(idValue);

    const SvString name = QBMovieUtilsGetAttr(movie, "name", NULL);
    const SvString runtime = QBMovieUtilsCreateStrFromAttrInteger(movie, "runtime", "%i", NULL);
    const SvString rating = QBMovieUtilsGetAttr(movie, "ratings", NULL);

    char *escapedId = QBStringCreateJSONEscapedString(id ? SvStringCString(id) : "");
    char *escapedName = QBStringCreateJSONEscapedString(name ? SvStringCString(name) : "");
    char *escapedRuntime = QBStringCreateJSONEscapedString(runtime ? SvStringCString(runtime) : "");
    char *escapedRating = QBStringCreateJSONEscapedString(rating ? SvStringCString(rating) : "");

    const SvString movieJSON = SvStringCreateWithFormat("\"id\":\"%s\",\"name\":\"%s\",\"runtime\":\"%s minutes\",\"rating\":\"%s\"",
                                                        escapedId,
                                                        escapedName,
                                                        escapedRuntime,
                                                        escapedRating);

    SVTESTRELEASE(runtime);
    free(escapedId);
    free(escapedName);
    free(escapedRuntime);
    free(escapedRating);

    return movieJSON;
}

void
QBVoDUtilsLogMovieDetails(const SvDBRawObject movie)
{
    if (!movie)
        return;

    const SvString movieJSON = QBVoDUtilsCreateJSONWithMovieDetails(movie);

    QBSecureLogEvent("QBVoDUtils",
                     "Notice.Vod.MovieMoreInfoSelected",
                     "JSON:{%s}",
                     SvStringGetCString(movieJSON));

    SVRELEASE(movieJSON);
}

void
QBVoDUtilsLogCurrentAndRecommendedMovieDetails(const SvDBRawObject currentMovie, const SvDBRawObject recommendedMovie)
{
    if (!currentMovie || !recommendedMovie)
        return;

    const SvString currentMovieJSON = QBVoDUtilsCreateJSONWithMovieDetails(currentMovie);
    const SvString recommendedMovieJSON = QBVoDUtilsCreateJSONWithMovieDetails(recommendedMovie);

    QBSecureLogEvent("QBVoDUtils",
                     "Notice.Vod.MovieRecommendedSelected",
                     "JSON:{\"currentMovie\":{%s},\"recommendedMovie\":{%s}}",
                     SvStringGetCString(currentMovieJSON),
                     SvStringGetCString(recommendedMovieJSON));

    SVRELEASE(currentMovieJSON);
    SVRELEASE(recommendedMovieJSON);
}

void
QBVoDUtilsLogMoviePurchase(const SvDBRawObject movie, const int selectedOfferId)
{
    if (!movie)
        return;

    // Movie details
    const SvString movieJSON = QBVoDUtilsCreateJSONWithMovieDetails(movie);

    // Purchase details
    bool isPromotion = false;
    SvString currency = NULL;
    int price = 0;
    int rentalPeriod = 0;

    const SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(movie, "offers");
    if (offers && SvObjectIsInstanceOf((SvObject) offers, SvArray_getType())) {
        SvIterator it = SvArrayIterator(offers);
        SvDBReference offerRef = NULL;
        while ((offerRef = (SvDBReference) SvIteratorGetNext(&it))) {
            // Find offer which was selected.
            const SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject(offerRef);
            if (!offer)
                continue;

            const SvValue offerIdValue = SvDBObjectGetID((SvDBObject) offer);
            if (!offerIdValue)
                continue;

            int offerId = -1;
            if (SvValueIsInteger(offerIdValue))
                offerId = SvValueGetInteger(offerIdValue);
            else if (SvValueIsString(offerIdValue))
                if (QBStringToInteger(SvStringCString(SvValueGetString(offerIdValue)), &offerId) == -1)
                    continue;

            if (offerId == selectedOfferId) {
                // Selected offer found. Get all needed data and stop searching.

                // Promotional flags
                const SvValue promotionValue = (SvValue) SvDBRawObjectGetAttrValue(offer, "promotional");
                if (promotionValue && SvObjectIsInstanceOf((SvObject) promotionValue, SvValue_getType()) && SvValueIsInteger(promotionValue))
                    isPromotion = SvValueGetInteger(promotionValue);

                // Currency
                const SvValue currencyValue = (SvValue) SvDBRawObjectGetAttrValue(offer, "currency");
                if (currencyValue && SvObjectIsInstanceOf((SvObject) currencyValue, SvValue_getType()) && SvValueIsString(currencyValue))
                    currency = SvValueGetString(currencyValue);

                // Price
                const SvValue priceValue = (SvValue) SvDBRawObjectGetAttrValue(offer, "price");
                if (priceValue && SvObjectIsInstanceOf((SvObject) priceValue, SvValue_getType()) && SvValueIsInteger(priceValue))
                    price = SvValueGetInteger(priceValue);

                // Rental period
                const SvValue rentalPeriodValue = (SvValue) SvDBRawObjectGetAttrValue(offer, "rental_period");
                if (rentalPeriodValue && SvObjectIsInstanceOf((SvObject) rentalPeriodValue, SvValue_getType()) && SvValueIsInteger(rentalPeriodValue))
                    rentalPeriod = SvValueGetInteger(rentalPeriodValue);

                // Stop searching
                break;
            }
        }
    }

    char *escapedCurrency = QBStringCreateJSONEscapedString(currency ? SvStringCString(currency) : "");

    QBSecureLogEvent("QBVoDUtils",
                     "Notice.Vod.MovieRentSelected",
                     "JSON:{\"movie\":{%s},"
                     "\"offer\":{\"id\":\"%d\",\"price\":\"%d.%02d %s\",\"rentalPerdiod\":\"%d %s\",\"isPromotion\":\"%s\"}}",
                     SvStringGetCString(movieJSON),
                     selectedOfferId,
                     price / 100,
                     price % 100,
                     escapedCurrency,
                     (rentalPeriod > 60) ? (rentalPeriod / 60) : (rentalPeriod), // convert to hours if possible
                     (rentalPeriod > 60) ? "hours" : "minutes",
                     isPromotion ? "true" : "false");

    SVRELEASE(movieJSON);
    free(escapedCurrency);
}

struct QBEventReporterWatchedEventFilter_ {
    struct SvObject_ super;             ///< super class
    QBBookmarkManager bookmarkManager;  ///< bookmark manager for determining the movie bookmark position
    SvObject eventReporter;             ///< actual event reporter to forward the events to
    SvString productId;                 ///< id of a movie - needed to obtain a bookmark position
    int runtimeSecs;                    ///< runtime of a movie - needed to determine percentage of played content
    bool moviePlayed;                   ///< flag determining if a "play" event has occured before "watched" event.
    bool isAdult;                       ///< flag telling if movie is adult
};

SvLocal void
QBEventReporterWatchedEventFilter__dtor__(void* self_)
{
    QBEventReporterWatchedEventFilter self = (QBEventReporterWatchedEventFilter) self_;
    SVTESTRELEASE(self->bookmarkManager);
    SVTESTRELEASE(self->eventReporter);
    SVTESTRELEASE(self->productId);
}

SvLocal void
QBEventReporterWatchedEventFilterSetListener(SvObject self_,
                                             SvObject listener,
                                             SvErrorInfo *errorOut)
{
    QBEventReporterWatchedEventFilter self = (QBEventReporterWatchedEventFilter) self_;
    SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, setListener, listener, errorOut);
}

SvLocal double
QBEventReporterWatchedEventFilterGetFractionPlayed(QBEventReporterWatchedEventFilter self)
{
    if (self->bookmarkManager && self->productId && self->runtimeSecs) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->bookmarkManager, self->productId);
        if (bookmark) {
            double position = QBBookmarkGetPosition(bookmark);
            double fraction = position / self->runtimeSecs;
            return fraction;
        }
    }
    return 0.0;
}

SvLocal int
QBEventReporterWatchedEventFilterSendEvent(SvObject self_, SvScheduler scheduler, QBMWClientEventReporterEventType eventType, SvErrorInfo *errorOut)
{
    QBEventReporterWatchedEventFilter self = (QBEventReporterWatchedEventFilter) self_;

    if (self->isAdult && eventType == QBMWClientEventReporter_addToWatched) {
        SvLogNotice("Skipping adding to watchlist for adult content");
        return 0;
    }

    if (eventType == QBMWClientEventReporter_play) {
        if (QBEventReporterWatchedEventFilterGetFractionPlayed(self) < QB_EVENT_FILTER_WATCHED_TRESHOLD) {
            self->moviePlayed = true; //consider sending watched event in the future only if the treshold has not been crossed yet
        }
    } else if (eventType == QBMWClientEventReporter_watched) {
        bool report = false;
        if (self->moviePlayed && QBEventReporterWatchedEventFilterGetFractionPlayed(self) > QB_EVENT_FILTER_WATCHED_TRESHOLD) {
            report = true;
        }
        self->moviePlayed = false;
        if (!report) {
            return 0;
        }
    }
    return SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, sendEvent, scheduler, eventType, errorOut);
}

SvLocal int
QBEventReporterWatchedEventFilterStop(SvObject self_, SvErrorInfo *errorOut)
{
    QBEventReporterWatchedEventFilter self = (QBEventReporterWatchedEventFilter) self_;
    return SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, stop, errorOut);
}

SvLocal SvType
QBEventReporterWatchedEventFilter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBEventReporterWatchedEventFilter__dtor__
    };
    static SvType type = NULL;

    static const struct QBMWClientEventReporter_ eventReporter_methods = {
        .setListener = QBEventReporterWatchedEventFilterSetListener,
        .sendEvent   = QBEventReporterWatchedEventFilterSendEvent,
        .stop        = QBEventReporterWatchedEventFilterStop
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBEventReporterWatchedEventFilter",
                            sizeof(struct QBEventReporterWatchedEventFilter_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMWClientEventReporter_getInterface(), &eventReporter_methods,
                            NULL);
    }

    return type;
}


QBEventReporterWatchedEventFilter
QBEventReporterWatchedEventFilterCreate(QBBookmarkManager bookmarkManager, SvDBRawObject movie, SvObject eventReporter, SvErrorInfo* errorOut)
{
    SvErrorInfo error = NULL;
    QBEventReporterWatchedEventFilter self = NULL;
    if (!movie || !eventReporter || !SvObjectIsImplementationOf(eventReporter, QBMWClientEventReporter_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "QBEventReporterWatchedEventFilterCreate bad parameters movie: %p, eventReporter: %p", movie, eventReporter);
        goto out;
    }
    self = (QBEventReporterWatchedEventFilter) SvTypeAllocateInstance(QBEventReporterWatchedEventFilter_getType(), &error);
    if (unlikely(!self)) {
        goto out;
    }

    self->eventReporter = SVRETAIN(eventReporter);
    self->bookmarkManager = SVTESTRETAIN(bookmarkManager);

    SvValue productIdV = SvDBObjectGetID((SvDBObject) movie);
    if (productIdV && SvValueIsString(productIdV)) {
        self->productId = SVRETAIN(SvValueGetString(productIdV));
    }

    SvValue val = NULL;
    if ((val = (SvValue) SvDBRawObjectGetAttrValue(movie, "runtime")) != NULL) {
        if (val && SvObjectIsInstanceOf((SvGenericObject) val, SvValue_getType())
            && SvValueIsInteger(val)) {
            self->runtimeSecs = SvValueGetInteger(val) * 60;
        }
    }

    if ((val = (SvValue) SvDBRawObjectGetAttrValue(movie, "is_adult")) != NULL) {
        if (val && SvObjectIsInstanceOf((SvGenericObject) val, SvValue_getType())) {
            if (SvValueIsInteger(val)) {
                self->isAdult = SvValueGetInteger(val) != 0;
            } else if (SvValueIsBoolean(val)) {
                self->isAdult = SvValueGetBoolean(val);
            }
        }
    }
out:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}
