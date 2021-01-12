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
#include <Logic/VoDMovieDetailsLogic.h>
#include <ContextMenus/QBContentSideMenu.h>
#include <Widgets/QBVoDGrid.h>


struct QBVoDMovieDetailsLogic_t {
    struct SvObject_ super_;
};

SvLocal SvType QBVoDMovieDetailsLogic_getType(void)
{
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBVoDMovieDetailsLogic",
                            sizeof(struct QBVoDMovieDetailsLogic_t),
                            SvObject_getType(),
                            &type,
                            NULL);
    }
    return type;
}

void
QBVoDMovieDetailsLogicBasicElementsInit(QBVoDMovieDetailsBasicElements basicElements)
{
    basicElements->title = NULL;
    basicElements->cover = NULL;
    basicElements->description = NULL;
    basicElements->movieInfo = NULL;
    basicElements->similarCarousel = NULL;
    basicElements->similarCarouseInitialFocusColumn = 0;
    basicElements->listStartMarker = NULL;
    basicElements->trailerPreview = NULL;

    basicElements->usesMovieInfo = true;
    basicElements->usesTrailerPreview = true;
    basicElements->usesSimilarCarousel = true;
}

QBVoDMovieDetailsLogic
QBVoDMovieDetailsLogicNew(AppGlobals appGlobals,
                          QBVoDMovieDetails details,
                          SvObject object,
                          SvHashTable actions,
                          QBContentAction action)
{
    QBVoDMovieDetailsLogic self = (QBVoDMovieDetailsLogic) SvTypeAllocateInstance(QBVoDMovieDetailsLogic_getType(), NULL);

    return self;
}

void
QBVoDMovieDetailsLogicCreateWidgets(QBVoDMovieDetailsLogic self, SvApplication app, SvWidget window, SvObject object, QBVoDMovieDetailsBasicElements basicElements)
{
}

SvDBRawObject
QBVoDMovieDetailsLogicGetAdaptedObject(QBVoDMovieDetailsLogic self,
                                       SvDBRawObject object)
{
    return NULL;
}

bool
QBVoDMovieDetailsLogicSimilarCarouselSlotSetObject(QBVoDMovieDetailsLogic self,
                                                   SvObject object,
                                                   SvWidget slot,
                                                   SvWidget icon)
{
    return false;
}

SvObject
QBVoDMovieDetailsLogicCreateContentMenu(QBVoDMovieDetailsLogic self)
{
    return NULL;
}

bool
QBVoDMovieDetailsLogicShowContentMenu(QBVoDMovieDetailsLogic self)
{
    return false;
}

bool
QBVoDMovieDetailsLogicHideContentMenu(QBVoDMovieDetailsLogic self)
{
    return false;
}

bool
QBVoDMovieDetailsLogicShowObjectDetails(QBVoDMovieDetailsLogic self)
{
    return false;
}

bool
QBVoDMovieDetailsLogicHideObjectDetails(QBVoDMovieDetailsLogic self)
{
    return false;
}

void
QBVoDMovieDetailsLogicSetActiveElement(QBVoDMovieDetailsLogic self,
                                       SvDBRawObject object)
{
}

void
QBVoDMovieDetailsLogicSetUpCarousel(QBVoDMovieDetailsLogic self,
                                    QBVoDMovieDetailsBasicElements basicElements)
{
}

bool
QBVoDMovieDetailsLogicHandleInputEvent(QBVoDMovieDetailsLogic self, SvInputEvent ev, SvObject data_)
{
    return false;
}

bool
QBVoDMovieDetailsLogicHandleCoverInputEvent(QBVoDMovieDetailsLogic self, SvInputEvent ev, SvObject data_)
{
    return false;
}

bool
QBVoDMovieDetailsLogicIsInstantTrailerPreview(QBVoDMovieDetailsLogic self)
{
    return false;
}
