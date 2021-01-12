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

#include "QBXHTMLContext.h"
#include <QBXHTMLWidgets/QBXHTMLQuickVisitor.h>
#include <QBXHTMLWidgets/QBXHTMLVisitor.h>
#include <QBXHTMLWidgets/QBXHTMLNodes.h>
#include <QBXHTMLWidgets/QBXHTMLNames.h>
#include <QBXHTMLWidgets/QBXHTMLWidgetInfo.h>
#include <QBXHTMLWidgets/QBXHTMLSubmitter.h>
#include <QBXHTMLWidgets/QBXHTMLUtils.h>
#include <QBConf.h>
#include <Services/QBXHTMLWindowsService.h>
#include <ContextMenus/QBOSKPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <QBOSK/QBOSKKey.h>
#include <Widgets/QBTitle.h>
#include <SWL/anim.h>
#include <SWL/label.h>
#include <SWL/viewport.h>
#include <SWL/QBFrame.h>
#include <SWL/input.h>
#include <SWL/button.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/types.h>
#include <settings.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBInput/QBInputCodes.h>
#include <SvCore/SvEnv.h>
#include <libintl.h>
#include <main.h>

//for inputs with no sequence number we start assigning artificial sequence numbers with this value
//if it is reasonably large we are able to assign sequence numbers manually so that inputs with assigned sequence number
//are accessed by user prior to those without a number assigned (range [0, XHTML_INPUT_NUMBERING_START]), or after
//then (range [XHTML_INPUT_NUMBERING_START+(number of inputs), INF])
#define XHTML_INPUT_NUMBERING_START 1000

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBXHTMLContextLogLevel", "0:error and warning, 1:state, 2:debug");

#define log_error(fmt, ...)   do { if (env_log_level() >= 0) SvLogError(COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) SvLogWarning(COLBEG() "[%s] " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
#define log_state(fmt, ...)   do { if (env_log_level() >= 1) SvLogNotice(COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
#define log_debug(fmt, ...)   do { if (env_log_level() >= 2) SvLogNotice("[%s] " fmt, __func__, ## __VA_ARGS__); } while (0)
/**
 * enum tells what is currently displayed on the screen
 */
typedef enum {
    /** loading animation */
    QBXHTMLContext_loading,
    /** xhtmlWindow */
    QBXHTMLContext_window,
    /** text message */
    QBXHTMLContext_message
} QBXHTMLContextDisplayedObject;

/**
 * state of response received after data sumbission
 * if state is incorrect we supposed to display window again and let user to enter correct data
 */
typedef enum {
    /** there is no response yet */
    QBXHTMLContext_lackOfResponse = -1,
    /** received status is correct */
    QBXHTMLContext_correct,
    /** received status is incorrect */
    QBXHTMLContext_error
} QBXHTMLContextResponseStatus;

typedef struct XHTMLInputQueueEntry_ {
    struct SvObject_ super;
    /** node of the tree that describes an input */
    QBXHTMLVisitableNode node;
    /** sequence number associated with a given input.
      * Decides on input traversal order.
      **/
    int number;
    /** flag telling if the input needs to be surrounded with frame to indicate that it is focused.
      * Some widgets indicate beeing focused already, so do not need frame around them (ie. buttons become orange)
      **/
    bool needsFrame;
} *XHTMLInputQueueEntry;

SvLocal SvType
XHTMLInputQueueEntry_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("XHTMLInputQueueEntry",
                            sizeof(struct XHTMLInputQueueEntry_),
                            SvObject_getType(), &type,
                            NULL);
    }

    return type;
}

struct QBXHTMLContext_ {
    struct QBWindowContext_t super_;
    /** global data handle */
    AppGlobals appGlobals;
    /** data of XHTML window displayed within this context */
    QBXHTMLWindow xhtmlWindow;
    /** widget ID */
    SvString widgetId;
    /** queue of all the inputs in the widget - used for traversing inputs by user clicking UP and DOWN */
    SvArray inputQueue;
    /** index of currently focused input */
    size_t curInputQueueIndex;
    /** message (most likely an error message) displayed instead of XHTML window */
    SvString message;
    /** side manu used for On Screen Keyboard */
    QBContextMenu sidemenu;
    /** widget that is focused */
    SvWidget focusedWidget;
    /** node from XHTML document that represents currently focused widget */
    QBXHTMLVisitableNode focusedNode;
    /** XHTML document used to recreate widget when popping context from stack */
    SvString xhtmlRebuildDocument;
    /** creator used to recreate XHTML document when popping context from stack */
    QBXHTMLWindowsCreator curCreator;
    /** handlers for all buttons with default actions */
    SvArray buttonHandlers;
    /** submitter to collecting and sending data from widgets */
    QBXHTMLSubmitter submitter;
    /** enum tells what is currently displayed on the screen */
    QBXHTMLContextDisplayedObject displayedObject;
    /** status returned after data submission*/
    int responseStatus;
};

typedef struct QBXHTMLContextWindow_ {
    struct SvObject_ super;

    SvWidget window;

    SvWidget viewport;
    SvWidget title;
    SvWidget waitWidget;
    SvWidget greeting;
    SvWidget message;
    SvWidget focusFrame;
    SvColor textColor;
    SvColor bgColor;

    SvFont font;
    int fontSize;

    QBXHTMLContext ctx;
} *QBXHTMLContextWindow;


SvLocal void
QBXHTMLContextButtonPushed(QBXHTMLContext self, QBXHTMLVisitableNode node);


// QBXHTMLButtonHandler class

typedef struct QBXHTMLButtonHandler_ {
    struct SvObject_ super_;
    QBXHTMLContext ctx;
    QBXHTMLVisitableNode node;
} *QBXHTMLButtonHandler;

SvLocal void
QBXHTMLButtonHandlerDestroy(void *self_)
{
    QBXHTMLButtonHandler self = self_;
    SVTESTRELEASE(self->node);
}

SvLocal void
QBXHTMLButtonHandlerHandleEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBXHTMLButtonHandler self = (QBXHTMLButtonHandler) self_;

    if (SvObjectIsInstanceOf((SvObject) event_, SvButtonPushedEvent_getType())) {
        QBXHTMLContextButtonPushed(self->ctx, self->node);
    }
}

SvLocal SvType
QBXHTMLButtonHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXHTMLButtonHandlerDestroy
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBXHTMLButtonHandlerHandleEvent
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXHTMLButtonHandler",
                            sizeof(struct QBXHTMLButtonHandler_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }

    return type;
}

SvLocal QBXHTMLButtonHandler
QBXHTMLButtonHandlerCreate(QBXHTMLContext ctx, QBXHTMLVisitableNode node)
{
    QBXHTMLButtonHandler self = (QBXHTMLButtonHandler) SvTypeAllocateInstance(QBXHTMLButtonHandler_getType(), NULL);
    if (self) {
        self->ctx = ctx;
        self->node = SVRETAIN(node);
    }
    return self;
}


SvLocal void
QBXHTMLContextWindowDestroy(void *self_)
{
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) self_;
    assert(!self->window);
    SVRELEASE(self->font);
}

SvLocal void
QBXHTMLContextWindowHandlePeerEvent(SvObject self_, QBPeerEvent event, SvObject sender);

SvLocal SvType
QBXHTMLContextWindow_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        static const struct SvObjectVTable_ objectVTable = {
            .destroy = QBXHTMLContextWindowDestroy
        };
        static const struct QBPeerEventReceiver_ eventReceiverMethods = {
            .handleEvent = QBXHTMLContextWindowHandlePeerEvent
        };

        SvTypeCreateManaged("QBXHTMLContextWindow",
                            sizeof(struct QBXHTMLContextWindow_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }
    return type;
}

SvLocal void QBXHTMLContextWindowClean(SvApplication app, void *self_)
{
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) self_;
    self->window = NULL;
    SVRELEASE(self);
}

SvLocal void
QBXHTMLContextWindowOskDigitTyped(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBXHTMLContextWindow self = self_;

    if (key->type == QBOSKKeyType_enter) {
        QBContextMenuHide(self->ctx->sidemenu, false);
        svInputSetText(self->ctx->focusedWidget, SvStringCString(input));
        return;
    }

    size_t validLength = 0;
    size_t numDots = 0;
    const char* content = SvStringCString(input);
    for (; validLength < SvStringLength(input); ++validLength) {
        if (content[validLength] == '.') {
            ++numDots;
            if (numDots > 1) {
                break;
            }
        } else if ((content[validLength] >= '0' && content[validLength] <= '9') || content[validLength] == '-') {
            //ok
        } else {
            break;
        }
    }

    if (SvStringEqualToCString(key->ID, "changeSign")) {
        SvString newStr;
        if (validLength == 0 || SvStringCString(input)[0] != '-') {
            newStr = SvStringCreateWithFormat("-%.*s", (int) validLength, content);
        } else {
            newStr = SvStringCreateSubString(input, 1, validLength - 1, NULL);
        }
        QBOSKPaneSetInput(pane, newStr);
        SVRELEASE(newStr);
    } else if (validLength != SvStringLength(input)) {
        SvString newStr = SvStringCreateWithFormat("%.*s", (int) validLength, content);
        QBOSKPaneSetInput(pane, newStr);
        SVRELEASE(newStr);
    }
}


SvLocal void
QBXHTMLContextWindowOskKeyTyped(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBXHTMLContextWindow self = self_;
    if (key->type == QBOSKKeyType_enter) {
        QBContextMenuHide(self->ctx->sidemenu, false);
        svInputSetText(self->ctx->focusedWidget, SvStringCString(input));
    }
}

SvLocal void
QBXHTMLContextWindowHighlightCurInputOskCallback(SvObject visitor_, QBXHTMLVisitableNode node)
{
    QBXHTMLQuickVisitor visitor = (QBXHTMLQuickVisitor) visitor_;
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) QBXHTMLQuickVisitorGetData(visitor);
    QBXHTMLContext ctx = self->ctx;

    SvString type = QBXHTMLVisitableNodeGetAttribute(node, QB_XHTML_ATTR_TYPE);
    int maxChars = QBXHTMLVisitableNodeGetAttributeAsInt(node, QB_XHTML_ATTR_MAX_LENGTH, 100, NULL);
    if (!type || SvStringEqualToCString(type, QB_XHTML_VALUE_TEXT)) {
        svSettingsPushComponent("XHTMLWidgets.settings");
        svSettingsSetInteger("OSKPane", "maxChars", maxChars);
        QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
        QBOSKPaneInit(oskPane, ctx->appGlobals->scheduler, ctx->sidemenu, 1, SVSTRING("OSKPane"),
                      QBXHTMLContextWindowOskKeyTyped, self, NULL);
        svSettingsPopComponent();
        char *text = svInputGetText(ctx->focusedWidget);
        QBOSKPaneSetRawInput(oskPane, text);
        free(text);

        QBContextMenuPushPane(ctx->sidemenu, (SvGenericObject) oskPane);
        QBContextMenuShow(ctx->sidemenu);
        SVRELEASE(oskPane);
    } else if (SvStringEqualToCString(type, QB_XHTML_VALUE_NUMERIC)) {
        svSettingsPushComponent("XHTMLWidgets.settings");
        svSettingsSetInteger("OSKPane", "maxChars", maxChars);
        QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
        QBOSKPaneInitLayout(oskPane, ctx->appGlobals->scheduler, ctx->sidemenu, "Digits.oskmap", 1, SVSTRING("OSKPane"),
                            QBXHTMLContextWindowOskDigitTyped, self, NULL);
        svSettingsPopComponent();
        char *text = svInputGetText(ctx->focusedWidget);
        QBOSKPaneSetRawInput(oskPane, text);
        free(text);

        QBContextMenuPushPane(ctx->sidemenu, (SvGenericObject) oskPane);
        QBContextMenuShow(ctx->sidemenu);
        SVRELEASE(oskPane);
    }
}

SvLocal void
QBXHTMLContextWindowHighlightCurInput(QBXHTMLContextWindow self)
{
    QBXHTMLContext ctx = self->ctx;

    if (SvArrayCount(ctx->inputQueue) == 0) {
        return;
    }

    XHTMLInputQueueEntry entry = (XHTMLInputQueueEntry) SvArrayAt(ctx->inputQueue, ctx->curInputQueueIndex);
    SvHashTable widgetsInfo = QBXHTMLWindowGetWidgetsInfo(ctx->xhtmlWindow);
    QBXHTMLWidgetInfo inputInfo = (QBXHTMLWidgetInfo) SvHashTableFind(widgetsInfo, (SvObject) entry->node);
    if (inputInfo) {
        SvWidget widget = (SvWidget) QBXHTMLWidgetInfoGetWidget(inputInfo);
        if (!widget) {
            return;
        }

        svWidgetSetFocus(widget);
        ctx->focusedWidget = widget;
        ctx->focusedNode = entry->node;

        if (entry->needsFrame) {
            int x = QBXHTMLWidgetInfoGetGlobalX(inputInfo) - 15;
            int y = QBXHTMLWidgetInfoGetGlobalY(inputInfo) - 5;
            int width = QBXHTMLWidgetInfoGetWidth(inputInfo) + 30;
            int height = QBXHTMLWidgetInfoGetHeight(inputInfo) + 10;

            self->focusFrame->off_x = x;
            self->focusFrame->off_y = y;
            QBFrameSetSize(self->focusFrame, width, height);
            svWidgetSetHidden(self->focusFrame, false);
        } else {
            svWidgetSetHidden(self->focusFrame, true);
        }
    }
}

typedef struct InputVisitorData {
    /** counter used to assign sequence number to inpuets whose sequence nnumber is not defined in XHTML document*/
    int noInputIdCounter;
    /** array of XHTMLInputQueueEntry objects. This array will be used to generate input queue, by sorting it according to sequence number*/
    SvArray result;
} InputVisitorData;

SvLocal void
QBXHTMLContextInputQueueCreatorCallback(SvObject inputVisitor_, XHTMLInputQueueEntry entry, QBXHTMLVisitableNode node)
{
    QBXHTMLQuickVisitor inputVisitor = (QBXHTMLQuickVisitor) inputVisitor_;
    InputVisitorData* data = (InputVisitorData *) QBXHTMLQuickVisitorGetData(inputVisitor);
    int number = QBXHTMLVisitableNodeGetAttributeAsInt(node, QB_XHTML_ATTR_SEQUENCE, -1, NULL);
    if (number < 0) {
        entry->number = data->noInputIdCounter;
        ++data->noInputIdCounter;
    } else {
        entry->number = number;
    }
    entry->node = node;
    SvArrayAddObject(data->result, (SvObject) entry);
}


SvLocal void
QBXHTMLContextNoFrameInputQueueCreatorCallback(SvObject inputVisitor_, QBXHTMLVisitableNode node)
{
    XHTMLInputQueueEntry entry = (XHTMLInputQueueEntry) SvTypeAllocateInstance(XHTMLInputQueueEntry_getType(), NULL);
    QBXHTMLContextInputQueueCreatorCallback(inputVisitor_, entry, node);
    entry->needsFrame = false;
    SVRELEASE(entry);
}

SvLocal void
QBXHTMLContextFrameInputQueueCreatorCallback(SvObject inputVisitor_, QBXHTMLVisitableNode node)
{
    XHTMLInputQueueEntry entry = (XHTMLInputQueueEntry) SvTypeAllocateInstance(XHTMLInputQueueEntry_getType(), NULL);
    QBXHTMLContextInputQueueCreatorCallback(inputVisitor_, entry, node);
    entry->needsFrame = true;
    SVRELEASE(entry);
}

SvLocal int QBXHTMLContextInputQueueCompare(void *prv,
                                            SvGenericObject objectA,
                                            SvGenericObject objectB)
{
    XHTMLInputQueueEntry a = (XHTMLInputQueueEntry) objectA;
    XHTMLInputQueueEntry b = (XHTMLInputQueueEntry) objectB;
    if (a->number < b->number) {
        return -1;
    } else {
        return 1;
    }
}

SvLocal void
QBXHTMLContextWindowCreateInputQueue(QBXHTMLContextWindow self)
{
    QBXHTMLContext ctx = self->ctx;

    SVTESTRELEASE(ctx->inputQueue);
    ctx->inputQueue = SvArrayCreate(NULL);
    QBActiveTree documentTree = QBXHTMLWindowGetXHTMLTree(ctx->xhtmlWindow);

    InputVisitorData inputVisitorData;
    inputVisitorData.noInputIdCounter = XHTML_INPUT_NUMBERING_START;
    inputVisitorData.result = ctx->inputQueue;
    QBXHTMLQuickVisitor inputVisitor = QBXHTMLQuickVisitorCreate(&inputVisitorData, false, NULL);
    QBXHTMLQuickVisitorSetInputCallback(inputVisitor, QBXHTMLContextFrameInputQueueCreatorCallback);
    QBXHTMLQuickVisitorSetSelectCallback(inputVisitor, QBXHTMLContextNoFrameInputQueueCreatorCallback);
    QBXHTMLQuickVisitorSetButtonCallback(inputVisitor, QBXHTMLContextNoFrameInputQueueCreatorCallback);
    QBNodeVisitorVisitTreePreorder((SvObject) inputVisitor, documentTree);
    SVRELEASE(inputVisitor);

    SvArraySortWithCompareFn(ctx->inputQueue, QBXHTMLContextInputQueueCompare, NULL);
    ctx->curInputQueueIndex = 0;
    QBXHTMLContextWindowHighlightCurInput(self);
}


SvLocal void
QBXHTMLContextWindowShowLoading(QBXHTMLContextWindow self)
{
    svWidgetSetHidden(self->message, true);
    svWidgetSetHidden(self->viewport, true);
    svWidgetSetHidden(self->waitWidget, false);
    svWidgetSetHidden(self->greeting, false);
}

SvLocal void
QBXHTMLContextWindowConnectInputCallback(SvObject inputVisitor_, QBXHTMLVisitableNode node)
{
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) QBXHTMLQuickVisitorGetData((QBXHTMLQuickVisitor) inputVisitor_);
    QBXHTMLContext ctx = self->ctx;

    SvString type = QBXHTMLVisitableNodeGetAttribute(node, QB_XHTML_ATTR_TYPE);
    if (!type || SvStringEqualToCString(type, QB_XHTML_VALUE_TEXT) || SvStringEqualToCString(type, QB_XHTML_VALUE_NUMERIC)) {
        SvHashTable widgetsInfo = QBXHTMLWindowGetWidgetsInfo(ctx->xhtmlWindow);
        QBXHTMLWidgetInfo inputInfo = (QBXHTMLWidgetInfo) SvHashTableFind(widgetsInfo, (SvGenericObject) node);
        if (!inputInfo) {
            return;
        }
        SvWidget input = QBXHTMLWidgetInfoGetWidget(inputInfo);
        if (!input) {
            return;
        }
        QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
        QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvInputSelectedEvent_getType(), input->prv, NULL);
    }
}

SvLocal void
QBXHTMLContextWindowConnectButtonCallback(SvObject inputVisitor_, QBXHTMLVisitableNode node)
{
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) QBXHTMLQuickVisitorGetData((QBXHTMLQuickVisitor) inputVisitor_);
    QBXHTMLContext ctx = self->ctx;

    SvString type = QBXHTMLVisitableNodeGetAttribute(node, QB_XHTML_ATTR_TYPE);
    if (type && (SvStringEqualToCString(type, QB_XHTML_VALUE_CANCEL) || SvStringEqualToCString(type, QB_XHTML_VALUE_SUBMIT))) {
        SvHashTable widgetsInfo = QBXHTMLWindowGetWidgetsInfo(ctx->xhtmlWindow);
        QBXHTMLWidgetInfo buttonInfo = (QBXHTMLWidgetInfo) SvHashTableFind(widgetsInfo, (SvGenericObject) node);
        if (!buttonInfo) {
            return;
        }
        SvWidget button = QBXHTMLWidgetInfoGetWidget(buttonInfo);
        if (!button) {
            return;
        }
        QBXHTMLButtonHandler handler = QBXHTMLButtonHandlerCreate(ctx, node);
        if (handler) {
            QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
            QBEventBusRegisterReceiverForSender(eventBus, (SvObject) handler, SvButtonPushedEvent_getType(), button->prv, NULL);
            SvArrayAddObject(ctx->buttonHandlers, (SvObject) handler);
            SVRELEASE(handler);
        }
    }
}

SvLocal void
QBXHTMLContextWindowShowWindow(QBXHTMLContextWindow self, QBXHTMLContext ctx)
{
    SvWidget xhtmlWidget = QBXHTMLWindowGetWidget(ctx->xhtmlWindow);
    SvString xhtmlTitle = QBXHTMLWindowGetTitle(ctx->xhtmlWindow);
    QBActiveTree tree = QBXHTMLWindowGetXHTMLTree(ctx->xhtmlWindow);

    QBXHTMLQuickVisitor connectInputsVisitor = QBXHTMLQuickVisitorCreate(self, false, NULL);
    QBXHTMLQuickVisitorSetInputCallback(connectInputsVisitor, QBXHTMLContextWindowConnectInputCallback);
    QBXHTMLQuickVisitorSetButtonCallback(connectInputsVisitor, QBXHTMLContextWindowConnectButtonCallback);
    QBNodeVisitorVisitTreePreorder((SvObject) connectInputsVisitor, tree);
    SVRELEASE(connectInputsVisitor);


    svWidgetSetHidden(self->waitWidget, true);
    svWidgetSetHidden(self->greeting, true);
    svWidgetSetHidden(self->message, true);
    SvWidget oldWidget = svViewPortGetContents(self->viewport);
    svViewPortSetContents(self->viewport, xhtmlWidget);

    if (oldWidget && oldWidget != xhtmlWidget) {
        svWidgetDestroy(oldWidget);
    }

    svWidgetSetHidden(self->viewport, false);

    if (xhtmlTitle) {
        QBTitleSetText(self->title, xhtmlTitle, 0);
    }

    QBXHTMLContextWindowCreateInputQueue(self);
}

SvLocal bool
QBXHTMLContextIsWidgetEmbeded(QBXHTMLContext ctx)
{
    if (!ctx->super_.window || !ctx->super_.window->prv)
        return false;

    QBXHTMLContextWindow self = (QBXHTMLContextWindow) ctx->super_.window->prv;
    if (!svViewPortGetContents(self->viewport))
        return false;

    return true;
}

SvLocal void
QBXHTMLContextWindowShowMessage(QBXHTMLContextWindow self, QBXHTMLContext ctx)
{

    svWidgetSetHidden(self->viewport, true);
    svWidgetSetHidden(self->greeting, true);
    svWidgetSetHidden(self->waitWidget, true);
    svLabelSetText(self->message, SvStringCString(ctx->message));
    svWidgetSetHidden(self->message, false);
    svWidgetSetFocus(self->viewport);
}

SvLocal void
QBXHTMLContextRefreshDisplay(QBXHTMLContext ctx)
{
    if (!ctx->super_.window) {
        return;
    }
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) ctx->super_.window->prv;
    log_debug("displayedObject: %d", ctx->displayedObject);

    if (ctx->displayedObject == QBXHTMLContext_message && ctx->message) {
        QBXHTMLContextWindowShowMessage(self, ctx);
    } else if (ctx->displayedObject == QBXHTMLContext_window && ctx->xhtmlWindow) {
        QBXHTMLContextWindowShowWindow(self, ctx);
    } else {
        QBXHTMLContextWindowShowLoading(self);
    }
}

SvLocal bool
QBXHTMLContextWindowInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) w->prv;
    QBXHTMLContext ctx = self->ctx;

    if (ctx->displayedObject == QBXHTMLContext_window && e->ch == QBKEY_DOWN && ctx->inputQueue) {
        ++ctx->curInputQueueIndex;
        if (ctx->curInputQueueIndex >= SvArrayCount(ctx->inputQueue)) {
            ctx->curInputQueueIndex = 0;
        }
        QBXHTMLContextWindowHighlightCurInput(self);
        return true;
    } else if (ctx->displayedObject == QBXHTMLContext_window && e->ch == QBKEY_UP && ctx->inputQueue) {
        if (ctx->curInputQueueIndex == 0) {
            ctx->curInputQueueIndex = SvArrayCount(ctx->inputQueue);
        }
        --ctx->curInputQueueIndex;
        QBXHTMLContextWindowHighlightCurInput(self);
        return true;
    } else if (ctx->displayedObject == QBXHTMLContext_message && e->ch == QBKEY_ENTER) {
        if (ctx->responseStatus == QBXHTMLContext_correct)
            QBApplicationControllerPopContext(ctx->appGlobals->controller);
        else {
            ctx->displayedObject = QBXHTMLContext_window;
            QBXHTMLContextRefreshDisplay(ctx);
        }
        return true;
    }
    return false;
}

SvLocal void
QBXHTMLContextRemoveWidget(QBXHTMLContext ctx)
{
    if (ctx->xhtmlWindow && !QBXHTMLContextIsWidgetEmbeded(ctx)) {
        svWidgetDestroy(QBXHTMLWindowGetWidget(ctx->xhtmlWindow));
    }
    SVTESTRELEASE(ctx->xhtmlWindow);
    ctx->xhtmlWindow = NULL;
}

SvLocal void
QBXHTMLContextShowCStringMessage(QBXHTMLContext ctx, const char *message_)
{
    SvString message = SvStringCreate(message_, NULL);
    QBXHTMLContextSetMessage(ctx, message);
    SVRELEASE(message);
}

SvLocal void
QBXHTMLContextWidgetRecreated(void* ctx_, QBXHTMLWindow window, QBXHTMLWindowsCreatorErrorCode errorCode)
{
    QBXHTMLContext ctx = (QBXHTMLContext) ctx_;

    if (errorCode == QBXHTMLWindowsCreatorErrorCode_ok && window) {
        QBXHTMLContextRemoveWidget(ctx);
        ctx->xhtmlWindow = SVRETAIN(window);
        QBXHTMLWindowSetId(ctx->xhtmlWindow, ctx->widgetId);
        QBXHTMLContextRefreshDisplay(ctx);
    } else {
        QBXHTMLContextShowCStringMessage(ctx, gettext("Service unavailable"));
    }
    SVRELEASE(ctx->curCreator);
    ctx->curCreator = NULL;
}

SvLocal void
QBXHTMLContextWindowHandleResponseWidget(QBXHTMLContext ctx, SvString widget)
{
    SVTESTRELEASE(ctx->widgetId);
    ctx->widgetId = SVTESTRETAIN(QBXHTMLWindowGetId(ctx->xhtmlWindow));
    SVTESTRELEASE(ctx->curCreator);
    ctx->curCreator = QBXHTMLWindowsCreatorCreate(ctx->appGlobals, NULL);
    ctx->displayedObject = QBXHTMLContext_window;
    QBXHTMLWindowsCreatorCreateWindowFromXHTML(ctx->curCreator, widget, QBXHTMLContextWidgetRecreated, ctx);
}

SvLocal void
QBXHTMLContextWindowHandleResponse(QBXHTMLContext ctx, SvHashTable response)
{
    SvString type = QBXHTMLUtilsGetString(response, SVSTRING("type"));
    if (!type) {
        log_error("there is no response type");
        QBXHTMLContextShowCStringMessage(ctx, gettext("Incorrect response type"));
        return;
    }

    if (SvStringEqualToCString(type, "text")) {
        SvString messageStr = QBXHTMLUtilsGetString(response, SVSTRING("message"));
        if (messageStr) {
            log_debug("message:%s", SvStringCString(messageStr));
            QBXHTMLContextSetMessage(ctx, messageStr);
        } else {
            log_error("there is no message in response");
            QBXHTMLContextShowCStringMessage(ctx, gettext("Incorrect message"));
        }
    } else if (SvStringEqualToCString(type, "widget")) {
        SvString htmlStr = QBXHTMLUtilsGetString(response, SVSTRING("html"));
        if (htmlStr) {
            log_debug("htmlStr:%s", SvStringCString(htmlStr));
            QBXHTMLContextWindowHandleResponseWidget(ctx, htmlStr);
        } else {
            log_error("there is no html data in response");
            QBXHTMLContextShowCStringMessage(ctx, gettext("Incorrect message"));
        }
    } else {
        log_error("incorrect response type (expected: text|widget,  received: %s", type ? SvStringCString(type) : "n/a");
        QBXHTMLContextShowCStringMessage(ctx, gettext("Incorrect response type"));
    }
}

SvLocal void
QBXHTMLContextSubmitterCallback(SvObject self_, SvHashTable response, QBXHTMLWindowsClientState state)
{
    QBXHTMLContext self = (QBXHTMLContext) self_;

    if (state == QBXHTMLWindowsClientState_gotResponse && response) {
        SvString id = QBXHTMLUtilsGetString(response, SVSTRING("id"));
        SvString widgetId = QBXHTMLWindowGetId(self->xhtmlWindow);
        log_debug("id:%s, widgetId: %s", id ? SvStringCString(id) : "n/a", widgetId ? SvStringCString(widgetId) : "n/a");
        if (!SvObjectEquals((SvObject) widgetId, (SvObject) id)) {
            log_error("incorrect widgets id (expected: %s, received: %s)", widgetId ? SvStringCString(widgetId) : "n/a", id ? SvStringCString(id) : "n/a");
            QBXHTMLContextShowCStringMessage(self, gettext("Incorrect widget id"));
            return;
        }

        SvObject statusObj = SvHashTableFind(response, (SvObject) SVSTRING("status"));
        if (!statusObj || !SvObjectIsInstanceOf(statusObj, SvValue_getType()) || !SvValueIsInteger((SvValue) statusObj)) {
            log_error("incorrect status:%p", statusObj);
            QBXHTMLContextShowCStringMessage(self, gettext("Incorrect response status"));
            return;
        } else {
            int stat = SvValueGetInteger((SvValue) statusObj);
            self->responseStatus = stat;
            log_debug("status:%d", stat);
            QBXHTMLContextWindowHandleResponse(self, response);
        }
    } else {
        log_error("error while submission data from widget");
        QBXHTMLContextShowCStringMessage(self, gettext("Error while data submission"));
    }
}

SvLocal void
QBXHTMLContextWindowHandlePeerEvent(SvObject self_, QBPeerEvent event, SvObject sender)
{
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) self_;
    QBXHTMLContext ctx = self->ctx;

    log_debug("displayedObject:%d", ctx->displayedObject);

    if (ctx->displayedObject == QBXHTMLContext_window && SvObjectIsInstanceOf((SvObject) event, SvInputSelectedEvent_getType())) {
        QBXHTMLQuickVisitor oskVisitor = QBXHTMLQuickVisitorCreate(self, false, NULL);
        QBXHTMLQuickVisitorSetInputCallback(oskVisitor, QBXHTMLContextWindowHighlightCurInputOskCallback);
        QBNodeVisitorVisitNode((SvObject) oskVisitor, ctx->focusedNode);
        SVRELEASE(oskVisitor);
    }
}

SvLocal void
QBXHTMLContextButtonPushed(QBXHTMLContext self, QBXHTMLVisitableNode node)
{
    if (self->displayedObject == QBXHTMLContext_window) {
        SvString type = QBXHTMLVisitableNodeGetAttribute(node, QB_XHTML_ATTR_TYPE);
        if (type && SvStringEqualToCString(type, QB_XHTML_VALUE_SUBMIT)) {
            SvHashTable widgetsInfo = QBXHTMLWindowGetWidgetsInfo(self->xhtmlWindow);
            SvString widgetId = QBXHTMLWindowGetId(self->xhtmlWindow);
            self->displayedObject = QBXHTMLContext_loading;
            self->responseStatus = QBXHTMLContext_lackOfResponse;
            QBXHTMLContextRefreshDisplay(self);
            if (!QBXHTMLSubmitterSubmit(self->submitter, widgetsInfo, widgetId, node, QBXHTMLContextSubmitterCallback, (SvObject) self)) {
                QBXHTMLContextShowCStringMessage(self, gettext("Can't submit data form XHTMLWindows"));
            }
        } else {
            QBApplicationControllerPopContext(self->appGlobals->controller);
        }
    }
}

SvLocal SvWidget
QBXHTMLContextWindowCreate(QBXHTMLContext ctx, SvApplication app)
{
    QBXHTMLContextWindow self = (QBXHTMLContextWindow) SvTypeAllocateInstance(QBXHTMLContextWindow_getType(), NULL);
    if (!self) {
        log_error("can't create XHTML window");
        return NULL;
    }

    self->ctx = ctx;

    const char *componentName = "XHTMLWidgets.settings";
    svSettingsPushComponent(componentName);

    SvWidget window = svSettingsWidgetCreate(app, "QBXHTMLContextWindow");

    if (svSettingsIsWidgetDefined("CustomerLogo")) {
        SvWidget logo = svSettingsWidgetCreate(app, "CustomerLogo");
        if (logo)
            svSettingsWidgetAttach(window, logo, "CustomerLogo", 1);
    }

    self->waitWidget = svSettingsWidgetCreate(app, "QBXHTMLContextWindow.WaitAnimation");
    float duration = svSettingsGetDouble("QBXHTMLContextWindow.WaitAnimation", "stepDuration", 0.2f);
    int bmpCnt = svSettingsGetInteger("QBXHTMLContextWindow.WaitAnimation", "steps", 0);
    if (bmpCnt > 0) {
        SvBitmap* bitmaps = calloc(bmpCnt, sizeof(SvBitmap));
        int i;
        for (i = 0; i < bmpCnt; i++) {
            char frameName[16];
            snprintf(frameName, sizeof(frameName), "frame%i", i);
            bitmaps[i] = SVRETAIN(svSettingsGetBitmap("QBXHTMLContextWindow.WaitAnimation", frameName));
        }
        SvEffect effect = svEffectAnimNew(self->waitWidget, bitmaps, bmpCnt, SV_EFFECT_ANIM_FORWARD, SV_ANIM_LOOP_INFINITE, duration);
        for (i = 0; i < bmpCnt; i++)
            SVRELEASE(bitmaps[i]);
        free(bitmaps);
        svAppRegisterEffect(app, effect);
    }

    svSettingsWidgetAttach(window, self->waitWidget, "QBXHTMLContextWindow.WaitAnimation", 1);

    self->greeting = svLabelNewFromSM(app, "QBXHTMLContextWindow.Greeting");
    svSettingsWidgetAttach(window, self->greeting, "QBXHTMLContextWindow.Greeting", 1);

    self->message = svLabelNewFromSM(app, "QBXHTMLContextWindow.Message");
    svSettingsWidgetAttach(window, self->message, "QBXHTMLContextWindow.Message", 1);

    self->title = QBTitleNew(app, "Title", ctx->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, self->title, "Title", 1);
    SvString titleStr = SvStringCreate("", NULL);
    QBTitleSetText(self->title, titleStr, 0);
    SVRELEASE(titleStr);

    self->font = svSettingsCreateFont("QBXHTMLContextWindow", "font");
    self->fontSize = svSettingsGetInteger("QBXHTMLContextWindow", "fontSize", -1);
    self->textColor = svSettingsGetColor("QBXHTMLContextWindow", "textColor", 0xffffffff);
    self->bgColor = svSettingsGetColor("QBXHTMLContextWindow", "bgColor", -1);

    self->viewport = svViewPortNew(app, "QBXHTMLContextWindow.Viewport");
    svWidgetSetFocusable(self->viewport, true);
    svSettingsWidgetAttach(window, self->viewport, "QBXHTMLContextWindow.Viewport", 1);
    svWidgetSetFocus(self->viewport);

    self->focusFrame = QBFrameCreateFromSM(app, "QBXHTMLContextWindow.Viewport.FocusFrame");
    svSettingsWidgetAttach(self->viewport, self->focusFrame, "QBXHTMLContextWindow.Viewport.FocusFrame", 2);
    svWidgetSetHidden(self->focusFrame, true);


    ctx->sidemenu = QBContextMenuCreateFromSettings(componentName, ctx->appGlobals->controller, ctx->appGlobals->res, SVSTRING("SideMenu"));

    svSettingsPopComponent();

    self->window = window;
    window->prv = self;
    window->clean = QBXHTMLContextWindowClean;
    svWidgetSetInputEventHandler(window, QBXHTMLContextWindowInputEventHandler);

    return window;
}

SvLocal void
QBXHTMLContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBXHTMLContext self = (QBXHTMLContext) self_;

    SvWidget window = QBXHTMLContextWindowCreate(self, app);
    self->super_.window = window;

    if (self->xhtmlRebuildDocument) {
        SVTESTRELEASE(self->curCreator);
        self->curCreator = QBXHTMLWindowsCreatorCreate(self->appGlobals, NULL);
        QBXHTMLWindowsCreatorCreateWindowFromXHTML(self->curCreator, self->xhtmlRebuildDocument, QBXHTMLContextWidgetRecreated, self);
        SVRELEASE(self->xhtmlRebuildDocument);
        self->xhtmlRebuildDocument = NULL;
    } else {
        QBXHTMLContextRefreshDisplay(self);
    }
}

SvLocal void
QBXHTMLContextDestroyWindow(QBWindowContext self_)
{
    QBXHTMLContext self = (QBXHTMLContext) self_;

    if (self->xhtmlWindow) {
        SVTESTRELEASE(self->widgetId);
        self->widgetId = SVTESTRETAIN(QBXHTMLWindowGetId(self->xhtmlWindow));
        //destroy everything except xhtml document text so that we can recreate widget when needed.
        SVTESTRELEASE(self->xhtmlRebuildDocument);
        self->xhtmlRebuildDocument = SVRETAIN(QBXHTMLWindowGetXHTMLDocument(self->xhtmlWindow));
        QBXHTMLContextRemoveWidget(self);
        SVTESTRELEASE(self->inputQueue);
        self->inputQueue = NULL;
        self->curInputQueueIndex = 0;
        self->focusedWidget = NULL;
        self->focusedNode = NULL;
    }

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;

    SVTESTRELEASE(self->sidemenu);
    self->sidemenu = NULL;
}

SvLocal void
QBXHTMLContextDestroy(void *self_)
{
    QBXHTMLContext self = (QBXHTMLContext) self_;

    QBXHTMLContextRemoveWidget(self);
    SVTESTRELEASE(self->xhtmlRebuildDocument);
    SVTESTRELEASE(self->inputQueue);
    SVTESTRELEASE(self->message);
    SVTESTRELEASE(self->curCreator);
    SVTESTRELEASE(self->widgetId);
    SVRELEASE(self->buttonHandlers);
    SVRELEASE(self->submitter);
}

SvLocal SvType
QBXHTMLContext_getType(void)
{
    static SvType type = NULL;
    static struct QBWindowContextVTable_ context_methods = {
        .super_        = {
            .destroy   = QBXHTMLContextDestroy
        },
        .createWindow  = QBXHTMLContextCreateWindow,
        .destroyWindow = QBXHTMLContextDestroyWindow
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXHTMLContext",
                            sizeof(struct QBXHTMLContext_),
                            QBWindowContext_getType(), &type,
                            QBWindowContext_getType(), &context_methods,
                            NULL);
    }

    return type;
}

QBWindowContext
QBXHTMLContextCreate(AppGlobals appGlobals)
{
    QBXHTMLContext ctx = (QBXHTMLContext) SvTypeAllocateInstance(QBXHTMLContext_getType(), NULL);

    ctx->appGlobals = appGlobals;
    ctx->buttonHandlers = SvArrayCreateWithCapacity(3, NULL);
    QBXHTMLWindowsClient client = QBXHTMLWindowsServiceGetXHTMLWindowsClient(ctx->appGlobals->xhtmlWindowsService);
    ctx->submitter = QBXHTMLSubmitterCreate(client, NULL);
    ctx->responseStatus = QBXHTMLContext_lackOfResponse;
    return (QBWindowContext) ctx;
}

void
QBXHTMLContextSetWidget(QBXHTMLContext self, QBXHTMLWindow xhtmlWindow)
{
    QBXHTMLContextRemoveWidget(self);
    self->xhtmlWindow = SVRETAIN(xhtmlWindow);
    self->displayedObject = QBXHTMLContext_window;
    QBXHTMLContextRefreshDisplay(self);
}

void
QBXHTMLContextSetMessage(QBXHTMLContext self, SvString message)
{
    SVTESTRELEASE(self->message);
    self->message = SVRETAIN(message);
    self->displayedObject = QBXHTMLContext_message;
    QBXHTMLContextRefreshDisplay(self);
}
