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


#include <main.h>
#include <error_codes.h>
#include <Logic/VoDLogic.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBMemoryInfo.h>


struct QBVoDLogic_t{
    struct SvObject_ super_;
    AppGlobals appGlobals;
};

SvLocal void QBVoDLogicDestroy(void *self_)
{
}

SvLocal SvType QBVoDLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDLogicDestroy
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBVoDLogic",
                            sizeof(struct QBVoDLogic_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

QBVoDLogic QBVoDLogicNew(AppGlobals appGlobals)
{
    QBVoDLogic self = (QBVoDLogic) SvTypeAllocateInstance(QBVoDLogic_getType(), NULL);
    self->appGlobals = appGlobals;

    return self;
}

int QBVoDLogicInputHandler(QBVoDLogic self, SvInputEvent ev, SvObject data_)
{
    return 0;
}

QBWindowContext QBVoDLogicCreateContextForObject(QBVoDLogic self, SvObject tree, SvObject provider, SvObject path, SvObject service, SvObject object)
{
    return NULL;
}

SvLocal SvString
QBVODLogicGetCategoryRating(QBContentCategory category)
{
    SvString rating = NULL;
    SvValue ratingV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("ratings"));

    if (ratingV && SvObjectIsInstanceOf((SvObject) ratingV, SvValue_getType()) && SvValueIsString(ratingV)) {
        rating = SvValueGetString(ratingV);
    }
    return rating;
}

bool
QBVODLogicCategoryShouldBeBlocked(AppGlobals appGlobals, QBContentCategory category)
{
    SvString rating = QBVODLogicGetCategoryRating(category);
    if (rating && !QBParentalControlRatingAllowed(appGlobals->pc, rating)) {
        return true;
    }

    return false;
}

bool
QBVODLogicCategoryIsProtected(AppGlobals appGlobals, QBContentCategory category)
{
    SvString rating = QBVODLogicGetCategoryRating(category);
    if (rating && !QBParentalControlRatingAllowed(appGlobals->pc, rating)) {
        return true;
    }

    return false;
}

int
QBVODLogicGetAllowedSizeOfBuffer(void)
{
    return QBMemoryInfoGetFreeRam() / 2;
}

bool
QBVODLogicAreMoviePlaybackPCAllowed(AppGlobals appGlobals, SvDBRawObject movie)
{
    if (!appGlobals || !movie) {
        return true;
    }

    // movie ratings
    SvValue val = (SvValue) SvDBRawObjectGetAttrValue(movie, "ratings");
    if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
        if (!QBParentalControlRatingAllowed(appGlobals->pc, SvValueGetString(val))) {
            // it could be authenticated using PC_MENU domain using the same plugin
            SvObject pluginPC = QBAccessManagerGetDomainPlugin(appGlobals->accessMgr, SVSTRING("PC"));
            SvObject pluginPC_MENU = QBAccessManagerGetDomainPlugin(appGlobals->accessMgr, SVSTRING("PC_MENU"));
            if (!SvObjectEquals(pluginPC, pluginPC_MENU)) {
                return false;
            }
        }
    }

    return true;
}

SvString QBVODLogicCreateNoRatingMessage(void)
{
    return SvStringCreate("", NULL);
}

bool QBVODLogicShouldResetProviderIfNotSpecifiedByMW(void)
{
    return false;
}
