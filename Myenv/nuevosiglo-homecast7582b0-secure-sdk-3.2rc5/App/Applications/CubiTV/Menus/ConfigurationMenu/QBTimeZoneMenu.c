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

#include "QBTimeZoneMenu.h"

#include <libintl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrnoDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvLog.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <QBMenu/QBMenu.h>
#include <main.h>
#include <main_decl.h>

#include "QBTimeZoneMenuItemChoice.h"


#define ESCAPING_CHR '\\'

#define log_debug(fmt, ...) do { if (false) SvLogNotice( COLBEG() "QBTimeZoneMenu :: " fmt COLEND_COL(blue), ##__VA_ARGS__); } while (0)
#define log_error(fmt, ...) do { if (true) SvLogError( COLBEG() "QBTimeZoneMenu :: " fmt COLEND_COL(red), ##__VA_ARGS__); } while (0)

struct QBTimeZoneParser_t {
    const char *filePath;
    QBActiveTreeNode rootNode, node;
    SvHashTable nodesMap;

    char *tmpStr;
    size_t tmpLen;

    bool isSubcaption;
};
typedef struct QBTimeZoneParser_t *QBTimeZoneParser;

SvLocal QBActiveTreeNode
QBTimeZoneMenuCreateNode(QBTimeZoneParser self,
                         SvString nodePath,
                         SvErrorInfo *errorOut)
{
    QBActiveTreeNode node = NULL;
    SvErrorInfo error = NULL;

    if (!self)
        return NULL;

    if (SvHashTableFind(self->nodesMap, (SvGenericObject) nodePath)) {
        SvLogError("QBActiveTreeParser: duplicate tree node. Path: '%s'",
                SvStringCString(nodePath));
        goto fini;
    }

    if (!(node = QBActiveTreeNodeCreate(NULL, NULL, &error))) {
        SvLogError("QBActiveTreeParser: out of memory");
    } else {
        SvHashTableInsert(self->nodesMap,
                          (SvGenericObject) nodePath,
                          (SvGenericObject) node);
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return node;
}

SvLocal SvString
QBTimeZoneMenuGetSubcaption(QBTimeZoneParser self)
{
    SvString subcaption = NULL;

    char *tmpPtr = NULL;
    bool isEscaped = false;

    if (!(tmpPtr = strpbrk(self->tmpStr, "\\")))
        goto fini;

    while (*tmpPtr != '\0') {
        // Remove escaping
        if (*tmpPtr == ESCAPING_CHR && !isEscaped) {
            memcpy(tmpPtr, tmpPtr+1, strlen(tmpPtr));
            tmpPtr[strlen(tmpPtr)] = '\0';
            isEscaped = true;
            continue;
        }

        tmpPtr++;
        isEscaped = false;
    }

fini:
    subcaption = SvStringCreate(self->tmpStr, NULL);
    // Ignore everything that eventually could be after subcaption
    self->tmpStr = NULL;
    self->tmpLen = 0;
    return subcaption;
}

SvLocal SvString
QBTimeZoneMenuGetNodeID(QBTimeZoneParser self)
{
    SvString nodeID = NULL;

    char *tmpPtr = NULL;
    bool isEscaped = false;


    if (!(tmpPtr = strpbrk(self->tmpStr, "\\ /")))
        goto fini;

    while (*tmpPtr != '\0') {
        if ((*tmpPtr == ESCAPING_CHR || *tmpPtr == ' ' || *tmpPtr == '/') &&
                !isEscaped) {
                if (*tmpPtr != ESCAPING_CHR) {
                    self->isSubcaption = (*tmpPtr == ' ');
                    *tmpPtr = '\0';
                    break;
                }

                // Remove escaping
                memmove(tmpPtr, tmpPtr+1, strlen(tmpPtr));
                tmpPtr[strlen(tmpPtr)] = '\0';
                isEscaped = true;
                self->tmpLen--;
                continue;
        }
        tmpPtr++;
        isEscaped = false;
    }

fini:
    nodeID = SvStringCreate(self->tmpStr, NULL);
    if (strlen(self->tmpStr) != self->tmpLen) {
        self->tmpLen -= strlen(self->tmpStr)+1;
        self->tmpStr = tmpPtr+1;
    } else {
        self->tmpStr = NULL;
        self->tmpLen = 0;
    }
    return nodeID;
}

SvLocal SvString
QBTimeZoneMenuGetValue(QBTimeZoneParser self)
{
    char *tmpVal = strsep(&self->tmpStr, " ");
    if (!tmpVal)
        return NULL;

    self->tmpLen -= strnlen(tmpVal, self->tmpLen)+1;
    return SvStringCreate(tmpVal, NULL);
}

SvLocal int
QBTimeZoneMenuParseLine(QBTimeZoneParser self,
                        SvErrorInfo *errorOut)
{
    const char commentMark = '#';
    SvString value = NULL, nodeID = NULL, nodePath = NULL;
    QBActiveTreeNode tmpNode = NULL;
    int retval = -1;

    SvErrorInfo error = NULL;

    // Omit commented and empty lines
    if (self->tmpStr[0] == commentMark || self->tmpLen == 0)
        return 0;

    if (!(value = QBTimeZoneMenuGetValue(self)) || !self->tmpStr)
        goto fini;

    SvString tmpPath = NULL;
    while (self->tmpLen != 0 && !self->isSubcaption) {
        SVTESTRELEASE(nodeID);
        if (!(nodeID = QBTimeZoneMenuGetNodeID(self)))
            goto fini;

        tmpPath = nodePath ? SvStringCreateWithFormat("%s_%s", SvStringCString(nodePath), SvStringCString(nodeID)) : SVRETAIN(nodeID);
        SVTESTRELEASE(nodePath);
        nodePath = tmpPath;

        log_debug("Node value: %s, NodeID: %s", SvStringCString(value), SvStringCString(nodeID));
        if ((tmpNode = (QBActiveTreeNode)SvHashTableFind(self->nodesMap, (SvGenericObject)nodePath)))
        {
            self->node = tmpNode;
            continue;
        }

        if (!(tmpNode = QBTimeZoneMenuCreateNode(self, nodePath, &error)))
            goto fini;

        QBActiveTreeNodeSetAttribute(tmpNode, SVSTRING("caption"), (SvGenericObject)nodeID);
        QBActiveTreeNodeAddChildNode(self->node, tmpNode, &error);
        if (error)
            goto fini;

        self->node = tmpNode;
        SVTESTRELEASE(tmpNode);
        tmpNode = NULL;
    }

    log_debug("Node value set to: %s", SvStringCString(value));
    QBActiveTreeNodeSetAttribute(self->node, SVSTRING("configurationValue"), (SvGenericObject)value);
    QBActiveTreeNodeSetAttribute(self->node, SVSTRING("configurationName"), (SvGenericObject)SVSTRING("TIMEZONE"));

    // Search for subcaption
    if (self->tmpLen != 0) {
        SvString subcaption = NULL;
        log_debug("str: %s, len: %zu", self->tmpStr, self->tmpLen);
        if (!(subcaption = QBTimeZoneMenuGetSubcaption(self)))
            goto fini;

        log_debug("Subcaption: %s", SvStringCString(subcaption));
        QBActiveTreeNodeSetAttribute(self->node, SVSTRING("subcaption"), (SvGenericObject)subcaption);
        SVRELEASE(subcaption);
    }

    retval = 0;

fini:
    SvErrorInfoPropagate(error, errorOut);
    SVTESTRELEASE(value);
    SVTESTRELEASE(nodeID);
    SVTESTRELEASE(tmpNode);
    SVTESTRELEASE(nodePath);
    self->isSubcaption = false;
    return retval;
}

SvLocal bool
QBTimeZoneMenuParseFile(SvString filePath,
                        QBActiveTreeNode rootNode,
                        SvErrorInfo *errorOut)
{
    struct QBTimeZoneParser_t parser;
    SvErrorInfo error = NULL;

    char lineBuffer[512];
    FILE *fd = NULL;

    memset(&parser, 0, sizeof(parser));

    if (!filePath) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL file path passed");
        goto fini;
    } else if (!(parser.nodesMap = SvHashTableCreate(229, NULL))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't create SvHashTable");
        goto fini;
    } else if (!(fd = fopen(SvStringCString(filePath), "r"))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "can't open file '%s' for reading", SvStringCString(filePath));
        goto fini;
    }

    parser.rootNode = rootNode;
    while (1) {
        // Parse file line after line
        memset(lineBuffer, 0, sizeof(lineBuffer));
        if (!fgets(lineBuffer, sizeof(lineBuffer), fd))
            break;

        parser.tmpStr = strchr(lineBuffer, '\n');
        if (parser.tmpStr) {
            if (parser.tmpStr > lineBuffer && *(parser.tmpStr - 1) == '\r')
                *(parser.tmpStr - 1) = '\0';
            else
                *parser.tmpStr = '\0';
        }

        parser.tmpLen = strnlen(lineBuffer, sizeof(lineBuffer)-1);
        parser.tmpStr = lineBuffer;
        parser.node = parser.rootNode;
        if (QBTimeZoneMenuParseLine(&parser, &error) != 0) {
            if (!error)
                error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                          "Invalid file format");
            goto fini;
        }
    }

    if (ferror(fd)) {
        error = SvErrorInfoCreate(SvErrnoDomain, errno, "fgets failed");
        goto fini;
    }

fini:
    if (fd)
        fclose(fd);
    SVTESTRELEASE(parser.nodesMap);
    SvErrorInfoPropagate(error, errorOut);

    if (error) {
        QBActiveTreeNodeRemoveSubTrees(parser.rootNode, NULL);
    }
    return error == NULL;
}

SvLocal SvString
QBTimeZoneMenuResolveAndCreateFilePath(void)
{
    char filePath[PATH_MAX + 1];
    struct stat st;
    snprintf(filePath, sizeof(filePath), "%s/usr/local/share/CubiTV/Configurations/timezones", SvGetRuntimePrefix());
    if (stat(filePath, &st) < 0 || !S_ISREG(st.st_mode)) {
        snprintf(filePath, sizeof(filePath), "%s/etc/timezones", SvGetRuntimePrefix());
        if (stat(filePath, &st) < 0 || !S_ISREG(st.st_mode))
            snprintf(filePath, sizeof(filePath), "/etc/timezones");
    }

    return SvStringCreate(filePath, NULL);
}

void QBTimeZoneMenuRegister(SvWidget menu,
                            QBTreePathMap pathMap,
                            AppGlobals appGlobals)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("TimeZone"));
    SvObject path = node ? QBActiveTreeCreateNodePath(appGlobals->menuTree, QBActiveTreeNodeGetID(node)) : NULL;
    if (!node || !path)
        return;

    SvString filePath = QBTimeZoneMenuResolveAndCreateFilePath();
    QBTimeZoneMenuParseFile(filePath, node, NULL);
    SVRELEASE(filePath);

    SvObject itemChoice = (SvObject) QBTimeZoneMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    QBTreePathMapInsert(pathMap, path, itemChoice, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);

    SVRELEASE(path);
}
