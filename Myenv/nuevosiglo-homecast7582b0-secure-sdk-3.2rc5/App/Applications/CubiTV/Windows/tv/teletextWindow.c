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

#include "teletextWindow.h"

#include <CUIT/Core/event.h>
#include <QBTeletextDisplayer.h>
#include <QBApplicationController.h>
#include <QBInput/QBInputCodes.h>
#include <main.h>
#include <settings.h>
#include <Windows/newtv.h>
#include <iso_639_table.h>


struct QBTeletextWindow_t {
    struct QBLocalWindow_t super_;
    AppGlobals appGlobals;

    SvWidget teletextDisplayer;
    int teletextInSubsMode;

    QBTeletextZoom zoomMode;

    int startPage;
    bool visible, transparent;
};

SvLocal void
QBTeletextWindowDestroy(void *self_)
{
    QBTeletextWindow self = self_;
    svWidgetDestroy(self->super_.window);
}

SvLocal SvType QBTeletextWindow_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBTeletextWindowDestroy
        }
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTeletextWindow",
                            sizeof(struct QBTeletextWindow_t),
                            QBLocalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}

SvLocal bool QBTeletextWindowInputEventHandler(SvWidget w, SvInputEvent ie)
{
    QBTeletextWindow self = w->prv;
    if(self->teletextInSubsMode)
        return false;

    switch(ie->ch){
        case QBKEY_RED:
            switch(self->zoomMode) {
                case QBTeletextZoom_Normal:
                    self->zoomMode = QBTeletextZoom_Top;
                    break;
                case QBTeletextZoom_Top:
                    self->zoomMode = QBTeletextZoom_Bottom;
                    break;
                case QBTeletextZoom_Bottom:
                    self->zoomMode = QBTeletextZoom_Normal;
                    break;
            }
            QBTeletextDisplayerSetZoom(self->teletextDisplayer, self->zoomMode);
            break;
        case QBKEY_PGDN:
        case QBKEY_PGUP:
        case QBKEY_CHUP:
        case QBKEY_CHDN:
        case QBKEY_ENTER:
        case QBKEY_INFO:
        case QBKEY_FUNCTION:
            break;
        default:
            return false;
    }
    return true;
}

SvLocal void
QBTeletextWindowClean(SvApplication app, void *prv)
{
}

QBTeletextWindow QBTeletextWindowNew(AppGlobals appGlobals, QBTeletextReceiver receiver)
{
    SvApplication app = appGlobals->res;
    QBTeletextWindow self = (QBTeletextWindow) SvTypeAllocateInstance(QBTeletextWindow_getType(), NULL);
    SvWidget window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    svWidgetSetName(window, "QBTeletextWindow");
    QBLocalWindowInit((QBLocalWindow) self, window, QBLocalWindowTypeFocusable);
    window->prv = self;
    svWidgetSetInputEventHandler(window, QBTeletextWindowInputEventHandler);
    window->clean = QBTeletextWindowClean;

    self->appGlobals = appGlobals;
    svSettingsPushComponent("teletext.settings");

    self->teletextDisplayer = QBTeletextDisplayerNew(app, "displayer");
    svWidgetAttach(window, self->teletextDisplayer, 0, 0, 2);
    svSettingsPopComponent();

    QBTeletextDisplayerSetReceiver(self->teletextDisplayer, receiver);

    return self;
}

void QBTeletextWindowSetReceiver(QBTeletextWindow self, QBTeletextReceiver receiver)
{
    QBTeletextDisplayerSetReceiver(self->teletextDisplayer, receiver);
}

void QBTeletextWindowSetStartPage(QBTeletextWindow self, struct svdataformat* format, const char *lang)
{
    int i, j;
    if(!format) {
        self->startPage = 0;
        goto fini;
    }
    if(self->startPage)
        return;
    for (i = 0; i < format->ts.teletext_cnt; i++) {
        for (j = 0; j < format->ts.teletext[i]->cnt; j++) {
            struct sv_teletext_service *teletext = &format->ts.teletext[i]->tab[j];
            if(teletext->type == 1 && !iso639TerminologicalCompare(teletext->lang, lang)) {
                self->startPage = 0x100 * teletext->magazine + teletext->page;
                goto fini;
            }
        }
    }
    self->startPage = 0x100;
fini:
    QBTeletextWindowSetPage(self, self->startPage ? self->startPage : 0x100);
}

void QBTeletextWindowSetPage(QBTeletextWindow self, int pageNum)
{
    QBTeletextDisplayerSetPage(self->teletextDisplayer, pageNum);
};

void QBTeletextWindowStop(QBTeletextWindow self)
{
#if 0
  if (self->teletextFilter)
      QBTunerFilterStop(self->teletextFilter);
  if (self->teletextManager)
      QBTeletextManagerStop(self->teletextManager);
#endif
}

void QBTeletextWindowShow(QBTeletextWindow self)
{
    QBApplicationControllerAddLocalWindow(self->appGlobals->controller, (QBLocalWindow) self);
    QBTeletextDisplayerSetActive(self->teletextDisplayer, true);
    QBTeletextDisplayerRefreshPage(self->teletextDisplayer);
    self->visible = true;
}

void QBTeletextWindowHide(QBTeletextWindow self)
{
    QBApplicationControllerRemoveLocalWindow(self->appGlobals->controller, (QBLocalWindow)self);
    QBTeletextDisplayerSetActive(self->teletextDisplayer, false);
    self->visible = false;
}

bool QBTeletextWindowIsVisible(QBTeletextWindow self)
{
    return self->visible;
}

void QBTeletextWindowSetTransparent(QBTeletextWindow self, bool transparent)
{
    self->transparent = transparent;
    QBTeletextSetTransparent(self->teletextDisplayer, transparent);
}

bool QBTeletextWindowIsTransparent(QBTeletextWindow self)
{
    return self->transparent;
}

void QBTeletextWindowSetHints(QBTeletextWindow self, const char* hintedLang)
{
    QBTeletextDisplayerSetHints(self->teletextDisplayer, hintedLang);
}
