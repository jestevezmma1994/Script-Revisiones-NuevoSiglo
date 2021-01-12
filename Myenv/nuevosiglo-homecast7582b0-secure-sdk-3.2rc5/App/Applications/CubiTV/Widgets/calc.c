/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "calc.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/SvRBLocator.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/app.h>
#include <SWL/label.h>
#include <assert.h>
#include <math.h>
#include <QBOSK/QBOSKMap.h>
#include <QBOSK/QBOSKParser.h>
#include <QBOSK/QBOSKRenderer.h>
#include <QBOSK/QBOSK.h>
#include <QBOSK/QBOSKEvents.h>


typedef enum {
    QBCalcOperation_NONE,
    QBCalcOperation_MULTIPLY,
    QBCalcOperation_DIVIDE,
    QBCalcOperation_ADD,
    QBCalcOperation_SUBTRACT
} QBCalcOperation;


/**
 * Calculator widget class.
 * @class QBCalc
 * @extends SvObject
 **/
typedef struct QBCalc_ *QBCalc;

/**
 * QBCalc class internals.
 **/
struct QBCalc_ {
    /// super class
    struct SvObject_ super_;

    SvWidget w;
    SvWidget osk;
    SvWidget input;
    SvWidget inputBG;

    QBOSKRenderer oskRenderer;
    QBOSKMap oskMap;

    SvString numerator;
    SvString denominator;
    bool hasDot;

    double num;

    enum {
        QBCalcState_NOOP,
        QBCalcState_TYPING_FIRST_NUMBER,
        QBCalcState_TYPING_FIRST_OPERATION,
        QBCalcState_WAITING_FOR_FIRST_OPERATION,
        QBCalcState_TYPING_NEXT_NUMBER,
        QBCalcState_TYPED_NEXT_OPERATION,
        QBCalcState_WAITING_FOR_NEXT_OPERATION,
    } state;

    QBCalcOperation operation;
};

/*static const char *stateToString[] = {
    [QBCalcState_NOOP]                        = "QBCalcState_NOOP",
    [QBCalcState_TYPING_FIRST_NUMBER]         = "QBCalcState_TYPING_FIRST_NUMBER",
    [QBCalcState_TYPING_FIRST_OPERATION]      = "QBCalcState_TYPING_FIRST_OPERATION",
    [QBCalcState_WAITING_FOR_FIRST_OPERATION] = "QBCalcState_WAITING_FOR_FIRST_OPERATION",
    [QBCalcState_TYPING_NEXT_NUMBER]          = "QBCalcState_TYPING_NEXT_NUMBER",
    [QBCalcState_TYPED_NEXT_OPERATION]        = "QBCalcState_TYPED_NEXT_OPERATION",
    [QBCalcState_WAITING_FOR_NEXT_OPERATION]  = "QBCalcState_WAITING_FOR_NEXT_OPERATION"
};*/

SvLocal void QBCalcClean(SvApplication app, void *self_)
{
    QBCalc self = self_;

    self->w = NULL;
    SVRELEASE(self);
}

SvLocal QBCalcOperation QBCalcCharToOperation(char typed)
{
    switch (typed) {
        case '*':
            return QBCalcOperation_MULTIPLY;
        case '/':
            return QBCalcOperation_DIVIDE;
        case '+':
            return QBCalcOperation_ADD;
        case '-':
            return QBCalcOperation_SUBTRACT;
        default:
            return QBCalcState_NOOP;
    }
}

SvLocal void QBCalcAppend(QBCalc self, char typed)
{
    SvString newStr = SvStringCreateWithFormat("%s%c", self->hasDot ? SvStringCString(self->denominator) : SvStringCString(self->numerator), typed);

    if (self->hasDot) {
        SVRELEASE(self->denominator);
        self->denominator = newStr;
    } else {
        SVRELEASE(self->numerator);
        self->numerator = newStr;
    }
}

SvLocal void QBCalcResetInput(QBCalc self)
{
    SVRELEASE(self->numerator);
    SVRELEASE(self->denominator);
    self->hasDot = false;
    self->numerator = SvStringCreate("", NULL);
    self->denominator = SvStringCreate("", NULL);
}

SvLocal void QBCalcReset(QBCalc self)
{
    QBCalcResetInput(self);
    self->state = QBCalcState_TYPING_FIRST_NUMBER;
}

SvLocal double QBCalcGetValue(QBCalc self)
{
    const char *buf = SvStringCString(self->numerator);

    double value = (double) atoll(buf[0] == '-' ? buf + 1 : buf) + ((double) atoll(SvStringCString(self->denominator))) / pow(10, SvStringLength(self->denominator));

    if (buf[0] == '-')
        value = -value;

    return value;
}

SvLocal void QBCalcCalculateValue(QBCalc self, float value)
{
    switch (self->operation) {
        case QBCalcOperation_MULTIPLY:
            self->num *= (double) value;
            break;
        case QBCalcOperation_DIVIDE:
            self->num /= (double) value;
            break;
        case QBCalcOperation_SUBTRACT:
            self->num -= (double) value;
            break;
        case QBCalcOperation_ADD:
            self->num += (double) value;
            break;
        default:
            break;
    }
    if (isnan(self->num))
        self->num = 0;
    if (isinf(self->num))
        self->num = 0;
}

SvLocal void QBCalcShow(QBCalc self, double value)
{
    SvString num = SvStringCreateWithFormat("%.10f", value);
    int dotIdx = SvStringLength(num) - 1;
    int lastNonZero = SvStringLength(num) - 1;
    const char *buf = SvStringCString(num);
    for (; dotIdx >= 0; dotIdx--) {
        if (buf[dotIdx] == '.')
            break;
    }
    if (dotIdx >= 0) {
        while (lastNonZero >= 0 && buf[lastNonZero] == '0')
            lastNonZero--;
    }

    SVRELEASE(self->numerator);
    SVRELEASE(self->denominator);

    if (dotIdx == -1 || dotIdx == lastNonZero) {
        if (dotIdx == -1) {
            self->numerator = SVRETAIN(num);
        } else {
            self->numerator = SvStringCreateSubString(num, 0, dotIdx, NULL);
        }
        self->denominator = SvStringCreate("", NULL);
        self->hasDot = false;
    } else {
        if (dotIdx > 0) {
            self->numerator = SvStringCreateSubString(num, 0, dotIdx, NULL);
        } else {
            self->numerator = SvStringCreate("", NULL);
        }
        self->denominator = SvStringCreateSubString(num, dotIdx + 1, lastNonZero - 1 - dotIdx + 1, NULL);
        self->hasDot = true;
    }

    SVRELEASE(num);
}

SvLocal void QBCalcHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBCalc self = (QBCalc) self_;

    if (unlikely(!self->w))
        return;

    if (!SvObjectIsInstanceOf((SvObject) event_, QBOSKKeyPressedEvent_getType()))
        return;

    QBOSKKeyPressedEvent event = (QBOSKKeyPressedEvent) event_;
    if (!event->super_.key)
        return;

    unsigned int layout = QBOSKMapFindLayout(self->oskMap, SVSTRING("default"));

    if (event->super_.key->type == QBOSKKeyType_default) {
        SvString val = event->super_.key->variants[layout].value;
        if (!val)
            return;

        if (SvStringEqualToCString(val, "CLR")) {
            QBCalcReset(self);
        } else if (SvStringEqualToCString(val, "changeSign")) {
            if (self->state == QBCalcState_TYPING_FIRST_NUMBER || self->state == QBCalcState_TYPING_NEXT_NUMBER) {
                SvString newStr;
                if (SvStringLength(self->numerator) == 0 || SvStringCString(self->numerator)[0] != '-')
                    newStr = SvStringCreateWithFormat("-%s", SvStringCString(self->numerator));
                else
                    newStr = SvStringCreateSubString(self->numerator, 1, SvStringLength(self->numerator) - 1, NULL);

                SVRELEASE(self->numerator);
                self->numerator = newStr;
            }
        } else {
            const char typed = SvStringCString(val)[0];

            if (typed >= '0' && typed <= '9') {
                if (self->state == QBCalcState_NOOP) {
                    QBCalcAppend(self, typed);
                    self->state = QBCalcState_TYPING_FIRST_NUMBER;
                } else if (self->state == QBCalcState_TYPING_FIRST_NUMBER || self->state == QBCalcState_TYPING_NEXT_NUMBER) {
                    QBCalcAppend(self, typed);
                } else if (self->state == QBCalcState_WAITING_FOR_FIRST_OPERATION) {
                    return;
                } else if (self->state == QBCalcState_TYPING_FIRST_OPERATION || self->state == QBCalcState_TYPED_NEXT_OPERATION) {
                    QBCalcResetInput(self);
                    QBCalcAppend(self, typed);
                    self->state = QBCalcState_TYPING_NEXT_NUMBER;
                } else if (self->state == QBCalcState_WAITING_FOR_NEXT_OPERATION) {
                    QBCalcReset(self);
                    QBCalcAppend(self, typed);
                    self->state = QBCalcState_TYPING_FIRST_NUMBER;
                }
            } else if (typed == '.') {
                if (self->hasDot || (self->state != QBCalcState_TYPING_FIRST_NUMBER && self->state != QBCalcState_TYPING_NEXT_NUMBER))
                    return;
                self->hasDot = true;
            } else if (typed != '*' && typed != '/' && typed != '+' && typed != '-' && typed != '=') {
                return;
            } else {
                if (!SvStringLength(self->numerator) && !SvStringLength(self->denominator))
                    return;

                if (typed == '=') {
                    if (self->state == QBCalcState_TYPING_FIRST_NUMBER) {
                        self->state = QBCalcState_WAITING_FOR_FIRST_OPERATION;
                        return;
                    } else if (self->state == QBCalcState_WAITING_FOR_FIRST_OPERATION)
                        return;
                    else if (self->state == QBCalcState_TYPING_FIRST_OPERATION) {
                        QBCalcCalculateValue(self, self->num);
                        QBCalcShow(self, self->num);
                        self->state = QBCalcState_WAITING_FOR_FIRST_OPERATION;
                    } else if (self->state == QBCalcState_TYPED_NEXT_OPERATION || self->state == QBCalcState_TYPING_NEXT_NUMBER) {
                        QBCalcCalculateValue(self, QBCalcGetValue(self));
                        QBCalcShow(self, self->num);
                        self->state = QBCalcState_WAITING_FOR_NEXT_OPERATION;
                    }
                } else {
                    if (self->state == QBCalcState_TYPING_FIRST_NUMBER || self->state == QBCalcState_TYPING_FIRST_OPERATION || self->state == QBCalcState_WAITING_FOR_FIRST_OPERATION) {
                        self->num = QBCalcGetValue(self);
                        self->state = QBCalcState_TYPING_FIRST_OPERATION;
                        self->operation = QBCalcCharToOperation(typed);
                        return;
                    } else if (self->state == QBCalcState_TYPING_NEXT_NUMBER) {
                        QBCalcCalculateValue(self, QBCalcGetValue(self));
                        QBCalcShow(self, self->num);

                        self->state = QBCalcState_TYPED_NEXT_OPERATION;
                        self->operation = QBCalcCharToOperation(typed);
                    } else if (self->state == QBCalcState_TYPED_NEXT_OPERATION || self->state == QBCalcState_WAITING_FOR_NEXT_OPERATION) {
                        self->operation = QBCalcCharToOperation(typed);
                        self->state = QBCalcState_TYPED_NEXT_OPERATION;
                        return;
                    }
                }
            }
        }
    } else if (event->super_.key->type == QBOSKKeyType_backspace) {
        if (self->state != QBCalcState_TYPING_FIRST_NUMBER && self->state != QBCalcState_TYPING_NEXT_NUMBER)
            return;
        if (SvStringLength(self->denominator)) {
            SvString newStr = SvStringCreateSubString(self->denominator, 0, SvStringLength(self->denominator) - 1, NULL);
            SVRELEASE(self->denominator);
            self->denominator = newStr;
        } else if (self->hasDot) {
            self->hasDot = false;
        } else {
            SvString newStr;
            if (SvStringLength(self->numerator) == 2 && SvStringCString(self->numerator)[0] == '-')
                newStr = SvStringCreate("", NULL);
            else
                newStr = SvStringCreateSubString(self->numerator, 0, SvStringLength(self->numerator) - 1, NULL);
            SVRELEASE(self->numerator);
            self->numerator = newStr;
        }
    }

    char *buf;
    if (self->hasDot)
        asprintf(&buf, "%s.%s", SvStringCString(self->numerator), SvStringCString(self->denominator));
    else
        asprintf(&buf, "%s", SvStringCString(self->numerator));

    svLabelSetText(self->input, buf);
    free(buf);
}

SvLocal void QBCalcDestroy(void *self_)
{
    QBCalc self = self_;

    assert(!self->w);

    SVRELEASE(self->oskMap);
    SVRELEASE(self->oskRenderer);
    SVRELEASE(self->numerator);
    SVRELEASE(self->denominator);
}

SvLocal SvType QBCalc_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCalcDestroy
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBCalcHandlePeerEvent
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBCalc",
                            sizeof(struct QBCalc_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBCalcFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    if (e->kind != SvFocusEventKind_GET)
        return;

    QBCalc self = w->prv;

    svWidgetSetFocus(self->osk);
}

SvWidget QBCalcNew(SvApplication app, SvScheduler scheduler, const char *widgetName)
{
    QBCalc self = (QBCalc) SvTypeAllocateInstance(QBCalc_getType(), NULL);

    self->numerator = SvStringCreate("", NULL);
    self->denominator = SvStringCreate("", NULL);

    self->oskRenderer = QBOSKRendererCreate(NULL);
    const char *file = "Calc.oskmap";

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
    SvRBLocator locator = QBResourceManagerGetResourceLocator(resourceManager);
    char *mapFilePath = SvRBLocatorFindFile(locator, file);
    if (!mapFilePath) {
        SvLogError("%s: Cannot find map xmp file", __func__);
        abort();
    }

    self->oskMap = QBOSKMapCreateFromFile(mapFilePath, NULL);
    free(mapFilePath);

    self->w = svSettingsWidgetCreate(app, widgetName);
    self->w->prv = self;
    self->w->clean = QBCalcClean;

    char *inputName;
    asprintf(&inputName, "%s.Input.Box", widgetName);
    self->inputBG = svSettingsWidgetCreate(app, inputName);
    svSettingsWidgetAttach(self->w, self->inputBG, inputName, 0);
    free(inputName);
    asprintf(&inputName, "%s.Input.Label", widgetName);
    self->input = svLabelNewFromSM(app, inputName);
    svSettingsWidgetAttach(self->inputBG, self->input, inputName, 0);
    free(inputName);

    char *oskName;
    asprintf(&oskName, "%s.OSK", widgetName);
    self->osk = QBOSKNew(app, oskName, NULL);
    svWidgetSetFocusable(self->osk, true);
    svSettingsWidgetAttach(self->w, self->osk, oskName, 0);
    free(oskName);

    QBOSKSetKeyboardMap(self->osk, self->oskRenderer, self->oskMap, SVSTRING("default"), NULL);
    QBOSKRendererStart(self->oskRenderer, scheduler, NULL);

    svWidgetSetFocusEventHandler(self->w, QBCalcFocusEventHandler);

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, QBOSKKeyPressedEvent_getType(), self->osk->prv, NULL);

    return self->w;
}
