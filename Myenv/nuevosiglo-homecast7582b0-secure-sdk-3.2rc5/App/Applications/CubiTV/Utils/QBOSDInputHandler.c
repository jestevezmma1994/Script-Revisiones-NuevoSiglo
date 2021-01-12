/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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
#include "QBOSDInputHandler.h"

#include <main.h>
#include <Services/core/playlistManager.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBInput/QBInputCodes.h>

struct QBOSDInputHandler_ {
    int selection;              ///< selected channel number
    int selectionDigits;        ///< number of already typed digits
    int selectionDigitsMax;     ///< maximum number of Digits in channel number
    bool changeChannel;         ///< do we have to reload channel
};

QBOSDInputHandler QBOSDInputHandlerCreate(void)
{
    QBOSDInputHandler self = calloc(1, sizeof(struct QBOSDInputHandler_));
    QBOSDInputHandlerResetState(self);
    return self;
}

void
QBOSDInputHandlerDestroy(QBOSDInputHandler self)
{
    free(self);
}
void QBOSDInputHandlerResetState(QBOSDInputHandler self)
{
    self->selection = -1;
    self->selectionDigits = 0;
    self->changeChannel = false;
}

bool QBOSDInputHandlerHandleInput(QBOSDInputHandler self, QBInputEvent ev, SvTimerId *selectionTimer, SvWidget owner, QBPlaylistManager playlists)
{
    self->changeChannel = false;
    if (ev.type != QBInputEventType_keyTyped) {
        return false;
    }
    bool ret = false;

    unsigned keyCode = ev.u.key.code;
    if (keyCode >= '0' && keyCode <= '9') {
        int digit = keyCode - '0';
        if (*selectionTimer)
            svAppTimerStop(owner->app, *selectionTimer);

        if (self->selectionDigits == 0) {
            self->selectionDigitsMax = 3;
            size_t channelCount = SvInvokeInterface(SvEPGChannelView, QBPlaylistManagerGetCurrent(playlists), getCount);
            if (channelCount > 999) {
                self->selectionDigitsMax = 4;
            } else if (channelCount > 0) {
                SvTVChannel maxChannel = SvInvokeInterface(SvEPGChannelView, QBPlaylistManagerGetCurrent(playlists), getByIndex, channelCount - 1);
                if (SvInvokeInterface(SvEPGChannelView, QBPlaylistManagerGetCurrent(playlists), getChannelNumber, maxChannel) > 999) {
                    self->selectionDigitsMax = 4;
                } else {
                    self->selectionDigitsMax = 3;
                }
            }
            self->selection = digit;
        } else {
            self->selection *= 10;
            self->selection += digit;
        }

        self->selectionDigits++;
        if (self->selectionDigits == self->selectionDigitsMax) {
            self->changeChannel = true;
        } else {
            *selectionTimer = svAppTimerStart(owner->app, owner, 2.0, true);
        }

        ret = true;
    } else if (ev.type == QBInputEventType_keyTyped) {
        if (keyCode == QBKEY_ENTER && self->selectionDigits > 0) {
            self->changeChannel = true;
            ret = true;
        }
    }
    return ret;
}

int QBOSDInputHandlerGetChannel(QBOSDInputHandler self)
{
    return self->selection;
}

int QBOSDInputHandlerGetSelection(QBOSDInputHandler self)
{
    return self->selectionDigits;
}

int QBOSDInputHandlerGetSelectionMax(QBOSDInputHandler self)
{
    return self->selectionDigitsMax;
}

bool QBOSDInputHandlerGetChangeChannnel(QBOSDInputHandler self)
{
    return self->changeChannel;
}
