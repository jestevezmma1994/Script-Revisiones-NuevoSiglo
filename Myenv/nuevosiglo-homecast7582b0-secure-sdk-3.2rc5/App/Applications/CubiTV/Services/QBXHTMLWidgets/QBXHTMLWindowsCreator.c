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

#include "QBXHTMLWindowsCreator.h"
#include <QBXHTMLWidgets/QBXHTMLSvWidgetsFactory.h>
#include <QBXHTMLWidgets/QBXHTMLImagesDownloader.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <SvHTTPClient/SvHTTPClientListener.h>
#include <SvHTTPClient/SvHTTPRequest.h>
#include <QBXHTMLWidgets/QBXHTMLNodes.h>
#include <QBXHTMLWidgets/QBXHTMLNames.h>
#include <QBXHTMLWidgets/QBXHTMLVisitor.h>
#include <QBXHTMLWidgets/QBXHTMLTreeBuilder.h>
#include <QBXHTMLWidgets/QBXHTMLAssembler.h>
#include <QBXHTMLWidgets/QBXHTMLTxtWidget.h>
#include <QBXHTMLWidgets/QBXHTMLQuickVisitor.h>
#include <QBXHTMLWidgets/QBXHTMLWindowsClient.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <settings.h>
#include <main.h>

struct QBXHTMLWindow_ {
    struct SvObject_ super;
    SvString windowId;
    SvString title;
    int refreshTime;
    SvWidget widget;
    QBActiveTree documentTree;
    SvString xhtmlDocument;
    SvHashTable widgetsInfo;
};

SvLocal void
QBXHTMLWindowDestroy(void* self_)
{
    QBXHTMLWindow self = (QBXHTMLWindow) self_;
    SVTESTRELEASE(self->title);
    SVTESTRELEASE(self->documentTree);
    SVTESTRELEASE(self->widgetsInfo);
    SVTESTRELEASE(self->xhtmlDocument);
    SVTESTRELEASE(self->windowId);
}

SvLocal SvType
QBXHTMLWindow_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXHTMLWindowDestroy
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXHTMLWindow", sizeof(struct QBXHTMLWindow_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

void
QBXHTMLWindowSetId(QBXHTMLWindow self, SvString id)
{
    if (!self || !id) {
        SvLogError("%s: null argument passed (self:%p, id:%p)", __func__, self, id);
        return;
    }
    SVTESTRELEASE(self->windowId);
    self->windowId = SVRETAIN(id);
}

SvString
QBXHTMLWindowGetTitle(QBXHTMLWindow self)
{
    return self->title;
}

SvString
QBXHTMLWindowGetId(QBXHTMLWindow self)
{
    return self->windowId;
}

int
QBXHTMLWindowGetRefreshTime(QBXHTMLWindow self)
{
    return self->refreshTime;
}

SvWidget
QBXHTMLWindowGetWidget(QBXHTMLWindow self)
{
    return self->widget;
}

QBActiveTree
QBXHTMLWindowGetXHTMLTree(QBXHTMLWindow self)
{
    return self->documentTree;
}

SvHashTable
QBXHTMLWindowGetWidgetsInfo(QBXHTMLWindow self)
{
    return self->widgetsInfo;
}

SvString
QBXHTMLWindowGetXHTMLDocument(QBXHTMLWindow self)
{
    return self->xhtmlDocument;
}


struct QBXHTMLWindowsCreator_ {
    struct SvObject_ super;
    /** Callback called when window is sucessfully created or error occurs */
    QBXHTMLWindowsCreatorCallback callback;
    /** First parameter of callback */
    void* callbackTarget;
    /** Tree structure representing XHTML document */
    QBActiveTree documentTree;
    /** Image downloader that can be used to obtain bitmaps for widgets */
    QBXHTMLImagesDownloader imgDownloader;
    /** Tree builder responsible for building documentTree */
    QBXHTMLTreeBuilder treeBuilder;
    /** Global data handle */
    AppGlobals appGlobals;
    /** result of the XHTML window creation process */
    QBXHTMLWindow product;
    /** window client that will provide communication with XHTML server */
    QBXHTMLWindowsClient xhtmlWindowsClient;
    /** request made to obtain xhtml window */
    QBXHTMLWindowsClientRequest request;
};

SvLocal void
QBXHTMLWindowsCreatorDestroy(void* self_)
{
    QBXHTMLWindowsCreator self = (QBXHTMLWindowsCreator) self_;
    SVTESTRELEASE(self->xhtmlWindowsClient);
    SVTESTRELEASE(self->request);
    SVTESTRELEASE(self->imgDownloader);
    SVTESTRELEASE(self->treeBuilder);
    SVTESTRELEASE(self->product);
}

SvLocal void QBXHTMLWindowsCreatorClientStateChanged(SvObject self_, QBXHTMLWindowsClientState state);

SvLocal SvType
QBXHTMLWindowsCreator_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXHTMLWindowsCreatorDestroy
    };

    static const struct QBXHTMLWindowsClientListener_ clientMethods = {
        .stateChanged = QBXHTMLWindowsCreatorClientStateChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXHTMLWindowsCreator", sizeof(struct QBXHTMLWindowsCreator_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBXHTMLWindowsClientListener_getInterface(), &clientMethods,
                            NULL);
    }

    return type;
}

QBXHTMLWindowsCreator QBXHTMLWindowsCreatorCreate(AppGlobals appGlobals, QBXHTMLWindowsClient xhtmlWindowsClient)
{
    QBXHTMLWindowsCreator self = (QBXHTMLWindowsCreator) SvTypeAllocateInstance(QBXHTMLWindowsCreator_getType(), NULL);
    self->appGlobals = appGlobals;
    self->xhtmlWindowsClient = SVTESTRETAIN(xhtmlWindowsClient);
    self->request = NULL;
    return self;
}

SvLocal void
QBXHTMLWindowsCreatorTextCallback(SvObject visitor_, QBXHTMLVisitableNode node)
{
    SvStringBuffer strBuffer = QBXHTMLQuickVisitorGetData((QBXHTMLQuickVisitor) visitor_);
    SvString text = QBXHTMLVisitableNodeGetAttribute(node, QB_XHTML_ATTR_TEXT);
    if (text) {
        SvStringBufferAppendString(strBuffer, text, NULL);
        SvStringBufferAppendCString(strBuffer, " ", NULL);
    }
}


SvLocal void
QBXHTMLWindowsCreatorTitleCallback(SvObject visitor_, QBXHTMLVisitableNode node)
{
    QBNodeVisitorVisitNodeChildren(visitor_, node);
}

SvLocal void
QBXHTMLWindowsCreator_ImgReadyCallback(void *self_)
{
    QBXHTMLWindowsCreator self = (QBXHTMLWindowsCreator) self_;

    svSettingsPushComponent("XHTMLWidgets.settings");
    int width = svSettingsGetInteger("QBXHTMLContextWindow.Viewport", "width", 1000);
    int height = svSettingsGetInteger("QBXHTMLContextWindow.Viewport", "height", 500);
    svSettingsPopComponent();

    QBXHTMLSvWidgetFactory wFactoryVisitor = QBXHTMLSvWidgetFactoryCreate(self->appGlobals->res, self->imgDownloader, NULL);
    QBXHTMLAssembler assembler = QBXHTMLAssemblerCreate(self->product->documentTree, (SvObject) wFactoryVisitor, NULL);
    SvWidget widget = (SvWidget) QBXHTMLAssemblerAssemble(assembler, width, height, NULL);
    self->product->widgetsInfo = SVTESTRETAIN(QBXHTMLAssemblerGetWidgetsInfo(assembler));
    SVRELEASE(wFactoryVisitor);
    SVRELEASE(assembler);

    QBXHTMLVisitableNode titleNode = QBXHTMLWidgetsFindNode(self->product->documentTree, SVSTRING(QB_XHTML_TYPE_TITLE), NULL, NULL);

    if (titleNode) {
        SvStringBuffer titleBuffer = SvStringBufferCreate(NULL);
        QBXHTMLQuickVisitor titleVisitor = QBXHTMLQuickVisitorCreate((void *) titleBuffer, false, NULL);
        QBXHTMLQuickVisitorSetTitleCallback(titleVisitor, QBXHTMLWindowsCreatorTitleCallback);
        QBXHTMLQuickVisitorSetTextCallback(titleVisitor, QBXHTMLWindowsCreatorTextCallback);
        QBNodeVisitorVisitNode((SvObject) titleVisitor, titleNode);
        self->product->title = SvStringBufferCreateContentsString(titleBuffer, NULL);
        SVRELEASE(titleVisitor);
        SVRELEASE(titleBuffer);
    }

    SVRELEASE(self->imgDownloader);
    self->imgDownloader = NULL;

    if (!widget) {
        self->callback(self->callbackTarget, NULL, QBXHTMLWindowsCreatorErrorCode_parsingError);
    } else {
        self->product->widget = widget;
        self->callback(self->callbackTarget, self->product, QBXHTMLWindowsCreatorErrorCode_ok);
    }
}

SvLocal void QBXHTMLWindowsCreatorTransferFinished(QBXHTMLWindowsCreator self)
{
    SvErrorInfo error = NULL;
    self->product->documentTree = SVTESTRETAIN(QBXHTMLTreeBuilderGetResult(self->treeBuilder, &error));
    SVRELEASE(self->treeBuilder);
    self->treeBuilder = NULL;

    if (error) {
        SvErrorInfoPropagate(error, NULL);
        self->callback(self->callbackTarget, NULL, QBXHTMLWindowsCreatorErrorCode_parsingError);
        return;
    }

    self->imgDownloader = QBXHTMLImagesDownloaderCreate(self->product->documentTree,
                                                        QBXHTMLWindowsCreator_ImgReadyCallback,
                                                        (void *) self,
                                                        NULL);
    QBXHTMLImagesDownloaderDownloadImages(self->imgDownloader);
}


SvLocal QBXHTMLWindowsCreatorErrorCode QBXHTMLWindowsCreatorGetErrorCodeFromClientState(QBXHTMLWindowsClientState state)
{
    switch (state) {
        case QBXHTMLWindowsClientState_transferCancelled:
            return QBXHTMLWindowsCreatorErrorCode_transferCanceled;
        default:
            return QBXHTMLWindowsCreatorErrorCode_dataNotAvailable;
    }
}

SvLocal void
QBXHTMLWindowsCreatorClientStateChanged(SvObject self_, QBXHTMLWindowsClientState state)
{
    QBXHTMLWindowsCreator self = (QBXHTMLWindowsCreator) self_;
    SvObject response = QBXHTMLWindowsClientRequestGetResponse(self->request);

    if (state == QBXHTMLWindowsClientState_gotResponse && response) {
        if (SvObjectIsInstanceOf(response, SvHashTable_getType())) {
            SvObject id = (SvObject) SvHashTableFind((SvHashTable) response, (SvObject) SVSTRING("id"));
            if (id && SvObjectIsInstanceOf(id, SvValue_getType()) && SvValueIsString((SvValue) id)) {
                SVTESTRELEASE(self->product->windowId);
                self->product->windowId = SVRETAIN(SvValueGetString((SvValue) id));
            }

            SvObject refreshTime = (SvObject) SvHashTableFind((SvHashTable) response, (SvObject) SVSTRING("refreshTime"));
            if (refreshTime && SvObjectIsInstanceOf(refreshTime, SvValue_getType()) && SvValueIsInteger((SvValue) refreshTime)) {
                self->product->refreshTime = SvValueGetInteger((SvValue) refreshTime);
            }

            SvObject xhtml = (SvObject) SvHashTableFind((SvHashTable) response, (SvObject) SVSTRING("html"));
            if (xhtml && SvObjectIsInstanceOf(xhtml, SvValue_getType()) && SvValueIsString((SvValue) xhtml)) {
                SVTESTRELEASE(self->product->xhtmlDocument);
                self->product->xhtmlDocument = SVRETAIN(SvValueGetString((SvValue) xhtml));
                QBXHTMLTreeBuilderFeedData(self->treeBuilder, self->product->xhtmlDocument);
                QBXHTMLWindowsCreatorTransferFinished(self);
            }
        }
    } else {
        SVRELEASE(self->treeBuilder);
        self->treeBuilder = NULL;
        self->callback(self->callbackTarget, NULL, QBXHTMLWindowsCreatorGetErrorCodeFromClientState(state));
    }
    SVTESTRELEASE(self->request);
    self->request = NULL;
}

void
QBXHTMLWindowsCreatorCreateWindowFromURL(QBXHTMLWindowsCreator self,
                                         SvURL url,
                                         SvString id,
                                         QBXHTMLWindowsCreatorCallback callback,
                                         void *target)
{
    self->callback = callback;
    self->callbackTarget = target;

    SVTESTRELEASE(self->treeBuilder);
    self->treeBuilder = QBXHTMLTreeBuilderCreate(NULL);

    SVTESTRELEASE(self->product);
    self->product = (QBXHTMLWindow) SvTypeAllocateInstance(QBXHTMLWindow_getType(), NULL);

    if (self->request)
        QBXHTMLWindowsClientRequestStop(self->request);

    self->request = SVTESTRETAIN(QBXHTMLWindowsClientGetWindow(self->xhtmlWindowsClient, SvURLString(url), id, (SvObject) self));
}

void
QBXHTMLWindowsCreatorCreateWindowFromXHTML(QBXHTMLWindowsCreator self,
                                           SvString xhtml,
                                           QBXHTMLWindowsCreatorCallback callback,
                                           void *target
                                           )
{
    self->callback = callback;
    self->callbackTarget = target;

    SVTESTRELEASE(self->treeBuilder);
    self->treeBuilder = QBXHTMLTreeBuilderCreate(NULL);

    SVTESTRELEASE(self->product);
    self->product = (QBXHTMLWindow) SvTypeAllocateInstance(QBXHTMLWindow_getType(), NULL);
    self->product->xhtmlDocument = SVRETAIN(xhtml);

    QBXHTMLTreeBuilderFeedData(self->treeBuilder, xhtml);
    QBXHTMLWindowsCreatorTransferFinished(self);
}
