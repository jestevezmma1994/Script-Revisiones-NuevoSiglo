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

#include "channelchoice.h"
#include <QBGlobalWindowManager.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <SWL/label.h>
#include <main.h>
#include <QBApplicationController.h>


struct QBChannelChoice_t {
    struct QBGlobalWindow_t super_;
    int num;
    SvWidget numWidget;
    SvTimerId timer;

    AppGlobals appGlobals;

    QBChannelChoiceCallback callback;
    void* callbackPrv;
};
typedef struct QBChannelChoice_t* QBChannelChoice;

SvLocal void QBChannelChoiceDestroy(void *ptr)
{
    QBChannelChoice cc = ptr;
    if(cc->timer) {
        QBApplicationControllerRemoveGlobalWindow(cc->appGlobals->controller, (QBGlobalWindow) cc);
    }
}

SvLocal SvType QBChannelChoice_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBChannelChoiceDestroy
        }
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelChoice",
                            sizeof(struct QBChannelChoice_t),
                            QBGlobalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }
    return type;
}

SvLocal void QBChannelChoiceSwitchChannels(QBChannelChoice cc)
{
    if(cc->callback)
        cc->callback(cc->callbackPrv, cc->num);
}

SvLocal void QBChannelChoiceLocalClean(QBChannelChoice cc)
{
    QBApplicationControllerRemoveGlobalWindow(cc->appGlobals->controller, (QBGlobalWindow) cc);
    cc->num = 0;
    cc->timer = 0;
}

SvLocal void QBChannelChoiceTimeout(SvWidget w, SvTimerEvent e)
{
    QBChannelChoice cc = w->prv;
    if(cc->timer != e->id)
        return;

    QBChannelChoiceSwitchChannels(cc);
    QBChannelChoiceLocalClean(cc);
}

SvLocal void QBChannelChoiceShow(QBChannelChoice cc)
{
    if(cc->timer) {
        svAppTimerStop(cc->numWidget->app, cc->timer);
        cc->timer = svAppTimerStart(cc->numWidget->app, cc->super_.window, 2, 1);
    } else if(QBApplicationControllerAddGlobalWindow(cc->appGlobals->controller, (QBGlobalWindow) cc)) {
        cc->timer = svAppTimerStart(cc->numWidget->app, cc->super_.window, 2, 1);
    } else {
        cc->num = 0;
    }
}

SvLocal void QBChannelChoiceClean(SvApplication app, void *ptr)
{

}

QBGlobalWindow QBChannelChoiceCreate(AppGlobals appGlobals)
{
    SvApplication app = appGlobals->res;

    QBChannelChoice cc = (QBChannelChoice) SvTypeAllocateInstance(QBChannelChoice_getType(), NULL);
    QBGlobalWindow super = (QBGlobalWindow) cc;
    svSettingsPushComponent("ChannelChoice.settings");

    SvWindow window = svSettingsWidgetCreate(app, "ChannelChoice");
    QBGlobalWindowInit(super, window, SVSTRING("ChannelChoice"));

    cc->numWidget = svLabelNewFromSM(app, "ChannelChoice.Num");
    svWidgetSetTimerEventHandler(super->window, QBChannelChoiceTimeout);
    super->window->prv = cc;
    super->window->clean = QBChannelChoiceClean;
    svSettingsWidgetAttach(super->window, cc->numWidget, "ChannelChoice.Num", 1);
    svSettingsPopComponent();

    cc->appGlobals = appGlobals;

    return (QBGlobalWindow) cc;
}

void QBChannelChoiceSetCallback(QBGlobalWindow cc_, QBChannelChoiceCallback callback, void *prv)
{
    QBChannelChoice cc = (QBChannelChoice) cc_;
    cc->callback = callback;
    cc->callbackPrv = prv;
}

bool QBChannelChoiceInputEventHandler(QBGlobalWindow cc_, SvInputEvent e)
{
    QBChannelChoice cc = (QBChannelChoice) cc_;

    if(e->ch < '0' || e->ch > '9')
        return false;
    cc->num = cc->num * 10 + e->ch - '0';

    if (cc->num > 1000) {
        QBChannelChoiceLocalClean(cc);
    } else {
        char *buf;
        asprintf(&buf, "%i", cc->num);
        svLabelSetText(cc->numWidget, buf);
        free(buf);

        QBChannelChoiceShow(cc);
    }

    return true;
}
