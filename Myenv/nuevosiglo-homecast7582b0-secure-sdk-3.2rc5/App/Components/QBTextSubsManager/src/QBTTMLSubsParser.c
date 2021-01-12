/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBTTMLSubsParser.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvStringBuffer.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvData.h>

#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvTextTypes.h>
#include <CAGE/Core/Sv2DRect.h>
#include <QBStringUtils.h>
#include <QBTextUtils/base64.h>

#include <ctype.h>
#include <expat.h>
#include <string.h>
#include <stdlib.h>

#define XML_NS "http://www.w3.org/XML/1998/namespace:"
#define TT_NS "http://www.w3.org/ns/ttml:"
#define STYLE_NS "http://www.w3.org/ns/ttml#styling:"
#define STYLE_NS2 "http://www.w3.org/ns/ttml#style:"
#define SMPTETT_NS "http://www.smpte-ra.org/schemas/2052-1/2010/smpte-tt:"

// namespaces below are defined in old spec http://www.w3.org/TR/2009/CR-ttaf1-dfxp-20090924/#vocabulary-namespaces
// and are only kept for compatibility with old software (e.g. Harmonic encoder)
#define TT_NS2 "http://www.w3.org/2006/10/ttaf1:"
#define STYLE_NS3 "http://www.w3.org/2006/10/ttaf1#styling:"

// namespaces below are defined in old spec http://www.w3.org/TR/2009/CR-ttaf1-dfxp-20090924/#vocabulary-namespaces
// and are only kept for compatibility with old software (e.g. Harmonic encoder)
#define TT_NS2 "http://www.w3.org/2006/10/ttaf1:"
#define STYLE_NS3 "http://www.w3.org/2006/10/ttaf1#styling:"

#define log_parse(fmt, ...) if (0) { SvLogNotice("TTML parse:: " fmt, ## __VA_ARGS__); }

#define INITIAL_STACK_CAPACITY 16

typedef enum {
    xmlState_error = -2,
    xmlState_ignore = -1,
    xmlState_inRoot = 0,
    xmlState_inTT,
    xmlState_inHead,
    xmlState_inLayout,
    xmlState_inRegion,
    xmlState_inStyling,
    xmlState_inStyle,
    xmlState_inBody,
    xmlState_inDiv,
    xmlState_inP,
    xmlState_inSpan,
    // Hard line break
    xmlState_inBr,
    xmlState_inMetadata,
    // SMPTE-TT tags
    xmlState_inSMPTEImage
} xmlState;

struct stackElem_ {
    char *tag;
    SvObject object;
};

// helper macro to avoid cancatenating name_space and pattern literals at runtime
#define LITERAL_COMBINE(name_space, pattern) name_space pattern, pattern

/* Helper function */
SvLocal bool compareAttr(const char *attr, const char *name_space_pattern, const char *pattern)
{
    if (!attr || !name_space_pattern || !pattern) {
        SvLogWarning("%s(): NULL argument passed attr=%p name_space_pattern=%p pattern=%p", __func__, attr, name_space_pattern, pattern);
        return false;
    }
    return (!strcmp(attr, name_space_pattern) || !strcmp(attr, pattern));
}

SvLocal void
QBTTMLContainerDestroy(void *self_)
{
    QBTTMLContainer self = self_;

    SVTESTRELEASE(self->region);
    SVTESTRELEASE(self->style);

    SVTESTRELEASE(self->id);
}

SvLocal void
QBTTMLStyleDestroy(void *self_)
{
    QBTTMLStyle self = self_;

    SVTESTRELEASE(self->rawAttrs);
}

SvLocal SvType
QBTTMLStyle_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLStyleDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLStyle",
                            sizeof(struct QBTTMLStyle_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal QBTTMLStyle
QBTTMLStyleCreate(void)
{
    QBTTMLStyle self = (QBTTMLStyle) SvTypeAllocateInstance(QBTTMLStyle_getType(), NULL);

    self->extent.w.unitType = unitType_percentage;
    self->extent.w.v = 100.0;
    self->extent.h.unitType = unitType_percentage;
    self->extent.h.v = 100.0;

    return self;
}

SvLocal SvType
QBTTMLContainer_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLContainerDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLContainer",
                            sizeof(struct QBTTMLContainer_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBTTMLEmbeddableContainerDestroy(void *self_)
{
    QBTTMLEmbeddableContainer self = self_;

    SVTESTRELEASE(self->descendants);
}

SvLocal SvType
QBTTMLEmbeddableContainer_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLEmbeddableContainerDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLEmbeddableContainer",
                            sizeof(struct QBTTMLEmbeddableContainer_),
                            QBTTMLContainer_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal SvType
QBTTMLRegion_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLRegion", sizeof(struct QBTTMLRegion_),
                            QBTTMLContainer_getType(), &type, NULL);
    }


    return type;
}

SvLocal void
QBTTMLDivDestroy(void *self_)
{
    QBTTMLDiv self = self_;

    SVTESTRELEASE(self->backgroundImageId);
    SVTESTRELEASE(self->backgroundImage);
}

SvLocal SvType
QBTTMLDiv_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLDivDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLDiv",
                            sizeof(struct QBTTMLDiv_),
                            QBTTMLEmbeddableContainer_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBTTMLParagraphDestroy(void *self_)
{
    // QBTTMLParagraph self = self_;
}

SvLocal SvType
QBTTMLParagraph_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLParagraphDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLParagraph",
                            sizeof(struct QBTTMLParagraph_),
                            QBTTMLEmbeddableContainer_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBTTMLSpanDestroy(void *self_)
{
    QBTTMLSpan self = self_;

    SVTESTRELEASE(self->text);
}

SvLocal SvType
QBTTMLSpan_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLSpanDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLSpan",
                            sizeof(struct QBTTMLSpan_),
                            QBTTMLEmbeddableContainer_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBTTMLMetadataDestroy(void *self_)
{
    QBTTMLMetadata self = self_;

    SVTESTRELEASE(self->rawAttrs);
    SVTESTRELEASE(self->descendants);
}

SvLocal SvType
QBTTMLMetadata_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLMetadataDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLMetadata",
                            sizeof(struct QBTTMLMetadata_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBTTMLSMPTETTImageDestroy(void *self_)
{
    QBTTMLSMPTETTImage self = self_;

    SVTESTRELEASE(self->imageData);
}

SvLocal SvType
QBTTMLSMPTETTImage_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLSMPTETTImageDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLSMPTETTImage",
                            sizeof(struct QBTTMLSMPTETTImage_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

struct QBTTMLSubsParser_ {
    struct SvObject_ super_;

    XML_Parser parser;
    SvErrorInfo error;

    struct stackElem_ *stack;
    unsigned stackSize, stackCapacity;
    unsigned ignoreLevel;

    xmlState parserState;

    QBTTMLRegion defaultRegion;
    QBTTMLStyle defaultStyle;

    SvHashTable regions;
    SvHashTable styles;
    SvHashTable images;

    SvArray divs;
    SvArray paragraphs;

    QBTTMLDocumentProperties properties;

    SvStringBuffer buf;
};

SvLocal void
QBTTMLSubsParserDestroy(void *self_)
{
    QBTTMLSubsParser self = self_;

    QBTTMLSubsParserReset(self);

    SVTESTRELEASE(self->defaultRegion);
    SVTESTRELEASE(self->defaultStyle);

    SVTESTRELEASE(self->regions);
    SVTESTRELEASE(self->styles);
    SVTESTRELEASE(self->images);
    SVTESTRELEASE(self->divs);
    SVTESTRELEASE(self->paragraphs);

    SVTESTRELEASE(self->buf);

    free(self->stack);
}

SvLocal SvType
QBTTMLSubsParser_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTTMLSubsParserDestroy
    };
    static SvType type = NULL;


    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLSubsParser",
                            sizeof(struct QBTTMLSubsParser_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBTTMLSubsParser
QBTTMLSubsParserCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBTTMLSubsParser self = NULL;

    if (!(self = (QBTTMLSubsParser) SvTypeAllocateInstance(QBTTMLSubsParser_getType(), &error))) {
        goto out;
    }

    self->stack = malloc(INITIAL_STACK_CAPACITY * sizeof(struct stackElem_));
    if (!self->stack) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory, "malloc");
        goto out;
    }
    self->stackCapacity = INITIAL_STACK_CAPACITY;


    self->regions = SvHashTableCreate(3, NULL);
    self->styles = SvHashTableCreate(3, NULL);
    self->images = SvHashTableCreate(3, NULL);

    self->divs = SvArrayCreateWithCapacity(7, NULL);
    self->paragraphs = SvArrayCreateWithCapacity(7, NULL);


    self->defaultRegion = (QBTTMLRegion) SvTypeAllocateInstance(QBTTMLRegion_getType(), &error);
    self->defaultStyle = QBTTMLStyleCreate();
    ((QBTTMLContainer) (self->defaultRegion))->style = SVRETAIN(self->defaultStyle);


    self->buf = SvStringBufferCreateWithCapacity(1024, &error);

out:
    SvErrorInfoPropagate(error, errorOut);
    if (error && self) {
        SVRELEASE(self);
        self = NULL;
    }

    return self;
}

SvLocal double parseTimeExpr(const char *str)
{
    size_t len;
    char *newPtr;
    unsigned int hours, minutes, intseconds;
    double seconds = 0.0;
    if (!str || !(len = strlen(str))) {
        return -1.0;
    }

    hours = strtoul(str, &newPtr, 10);

    if (newPtr && *newPtr == ':') {
        minutes = strtoul(newPtr + 1, &newPtr, 10);
        if (newPtr && *newPtr == ':') {
            if ((intseconds = strtoul(newPtr + 1, &newPtr, 10)) < 60 && newPtr) {
                if (*newPtr == ':') {
                    unsigned int frames = strtoul(newPtr + 1, &newPtr, 10);
                    if (frames < 30)
                        seconds = (double) frames / 30.0;
                } else if (*newPtr == '.') {
                    char *endPtr;
                    ++newPtr;   // move to the first digit
                    unsigned int frac = strtoul(newPtr, &endPtr, 10);
                    unsigned int fracSize = 1;
                    // count numbers of digits read
                    while (newPtr != endPtr) {
                        fracSize *= 10;
                        ++newPtr;
                    }
                    seconds = (double) frac / fracSize;
                }
                seconds += intseconds + (minutes + hours * 60) * 60;
                return seconds;
            }
        }
    } else {
        ssize_t parsed = QBStringToDouble(str, &seconds);
        log_parse("parsed: %zd len %zu secs: %lf rest: %s", parsed, len, seconds, str + parsed);
        if (parsed >= (ssize_t) len) {
            return -1.0;
        }
        if (parsed + 1 == (ssize_t) len) {
            switch (str[parsed]) {
                case 'h':
                    seconds *= 3600;
                    break;
                case 'm':
                    seconds *= 60;
                    break;
                case 's':
                default:
                    break;
            }
            return seconds;
        } else if (parsed + 2 == (ssize_t) len) {
        }
    }

    return -1.0;
}

SvLocal SvColor
parseColor(const char *str)
{
    size_t len;
    if (!str || !(len = strlen(str))) {
        return COLOR(0, 0, 0, 0);
    }

    if (str[0] == '#' && (len == 7 || len == 9)) {
        char *sep;
        unsigned int v = strtoul(str + 1, &sep, 16);

        if (sep && *sep == '\0') {
            if (len == 7) {
                v = (v << 8) | 0xFFU;
            }
            return (SvColor) v;
        }

        return COLOR(0, 0, 0, 0);
    }

    if (!strcmp(str, "transparent")) {
        return COLOR(0, 0, 0, 0);
    } else if (!strcmp(str, "black")) {
        return COLOR(0, 0, 0, 0xFF);
    } else if (!strcmp(str, "silver")) {
        return COLOR(0xC0, 0xC0, 0xC0, 0xFF);
    } else if (!strcmp(str, "gray")) {
        return COLOR(0x80, 0x80, 0x80, 0xFF);
    } else if (!strcmp(str, "white")) {
        return COLOR(0xFF, 0xFF, 0xFF, 0xFF);
    } else if (!strcmp(str, "maroon")) {
        return COLOR(0x80, 0, 0, 0xFF);
    } else if (!strcmp(str, "red")) {
        return COLOR(0xFF, 0, 0, 0xFF);
    } else if (!strcmp(str, "purple")) {
        return COLOR(0x80, 0, 0x80, 0xFF);
    } else if (!strcmp(str, "fuchsia") || !strcmp(str, "magenta")) {
        return COLOR(0xFF, 0, 0xFF, 0xFF);
    } else if (!strcmp(str, "green")) {
        return COLOR(0, 0x80, 0, 0xFF);
    } else if (!strcmp(str, "lime")) {
        return COLOR(0, 0xFF, 0, 0xFF);
    } else if (!strcmp(str, "olive")) {
        return COLOR(0x80, 0x80, 0, 0xFF);
    } else if (!strcmp(str, "yellow")) {
        return COLOR(0xFF, 0xFF, 0, 0xFF);
    } else if (!strcmp(str, "blue")) {
        return COLOR(0, 0, 0xFF, 0xFF);
    } else if (!strcmp(str, "teal")) {
        return COLOR(0, 0x80, 0x80, 0xFF);
    } else if (!strcmp(str, "navy")) {
        return COLOR(0, 0, 0x80, 0xFF);
    } else if (!strcmp(str, "aqua") || !strcmp(str, "cyan")) {
        return COLOR(0, 0xFF, 0xFF, 0xFF);
    }

    return COLOR(0, 0, 0, 0);
}

SvLocal int
parseLength(const char *str, struct lengthDesc *out)
{
    size_t parsed;
    size_t len;

    if (!out || !str || !(len = strlen(str))) {
        return -1;
    }

    parsed = QBStringToDouble(str, &out->v);

    if (parsed == 0 || parsed >= len) {
        return -1;
    }

    if (!strncmp(str + parsed, "%", 1)) {
        out->unitType = unitType_percentage;
        parsed += 1;
        if (out->v < 0.0 || out->v > 100.0) {
            return -1;
        }
    } else if (!strncmp(str + parsed, "px", 2)) {
        out->unitType = unitType_pixel;
        parsed += 2;
    } else if (!strncmp(str + parsed, "c", 1)) {
        out->unitType = unitType_c;
        parsed += 1;
    } else if (!strncmp(str + parsed, "em", 2)) {
        out->unitType = unitType_em;
        parsed += 2;
    } else {
        return -1;
    }

    return parsed;
}

SvLocal int
stackGrow(QBTTMLSubsParser self)
{
    self->stackCapacity *= 2;
    if (likely(self->stack = realloc(self->stack, self->stackCapacity * sizeof(struct stackElem_)))) {
        return 0;
    }
    return -1;
}

SvLocal int
stackPush(QBTTMLSubsParser self, const char *tag)
{
    if (self->stackSize >= self->stackCapacity) {
        if (stackGrow(self) < 0) {
            return -1;
        }
    }

    if (!(self->stack[self->stackSize].tag = strdup(tag))) {
        return -1;
    }
    self->stack[self->stackSize].object = NULL;
    ++self->stackSize;

    return 0;
}

SvLocal int
stackPop(QBTTMLSubsParser self)
{
    if (self->stackSize == 0) {
        return -1;
    }

    free(self->stack[self->stackSize - 1].tag);
    SVTESTRELEASE(self->stack[self->stackSize - 1].object);

    --self->stackSize;

    return -1;
}

SvLocal const char *
stackTop(QBTTMLSubsParser self)
{
    if (self->stackSize == 0) {
        return NULL;
    }

    return self->stack[self->stackSize - 1].tag;
}

SvLocal SvObject
stackGetObj(QBTTMLSubsParser self)
{
    if (self->stackSize == 0) {
        return NULL;
    }

    return self->stack[self->stackSize - 1].object;
}

SvLocal int
stackSetObj(QBTTMLSubsParser self, SvObject obj)
{
    if (self->stackSize == 0) {
        return -1;
    }

    SVTESTRETAIN(obj);
    SVTESTRELEASE(self->stack[self->stackSize - 1].object);
    self->stack[self->stackSize - 1].object = obj;

    return 0;
}

SvLocal xmlState
tagToXmlState(const char *tag)
{
    if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "tt")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "tt"))) {
        return xmlState_inTT;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "head")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "head"))) {
        return xmlState_inHead;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "body")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "body"))) {
        return xmlState_inBody;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "styling")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "styling"))) {
        return xmlState_inStyling;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "layout")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "layout"))) {
        return xmlState_inLayout;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "region")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "region"))) {
        return xmlState_inRegion;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "style")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "style"))) {
        return xmlState_inStyle;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "div")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "div"))) {
        return xmlState_inDiv;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "p")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "p"))) {
        return xmlState_inP;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "span")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "span"))) {
        return xmlState_inSpan;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
        return xmlState_inMetadata;
    } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "image")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "image"))) {
        return xmlState_inSMPTEImage;
    }

    return xmlState_error;
}

SvLocal xmlState
checkPushedState(QBTTMLSubsParser self, const char *tag)
{
    switch (self->parserState) {
        case xmlState_inRoot:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "tt")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "tt"))) {
                return xmlState_inTT;
            }
            break;
        case xmlState_inTT:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "head")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "head"))) {
                return xmlState_inHead;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "body")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "body"))) {
                return xmlState_inBody;
            }
            break;
        case xmlState_inHead:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "styling")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "styling"))) {
                return xmlState_inStyling;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "layout")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "layout"))) {
                return xmlState_inLayout;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        case xmlState_inLayout:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "region")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "region"))) {
                return xmlState_inRegion;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        case xmlState_inRegion:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "style")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "style"))) {
                return xmlState_inStyle;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        case xmlState_inStyling:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "style")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "style"))) {
                return xmlState_inStyle;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        case xmlState_inBody:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "div")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "div"))) {
                return xmlState_inDiv;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        case xmlState_inDiv:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "p")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "p"))) {
                return xmlState_inP;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "div")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "div"))) {
                return xmlState_inDiv;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        case xmlState_inP:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "span")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "span"))) {
                return xmlState_inSpan;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "br")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "br"))) {
                return xmlState_inBr;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        case xmlState_inSpan:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "span")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "span"))) {
                return xmlState_inSpan;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "br")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "br"))) {
                return xmlState_inBr;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        case xmlState_inMetadata:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            } else if (compareAttr(tag, LITERAL_COMBINE(SMPTETT_NS, "image"))) {
                return xmlState_inSMPTEImage;
            }
            break;
        case xmlState_inBr:
            if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "span")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "span"))) {
                return xmlState_inSpan;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "br")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "br"))) {
                return xmlState_inBr;
            } else if (compareAttr(tag, LITERAL_COMBINE(TT_NS, "metadata")) || compareAttr(tag, LITERAL_COMBINE(TT_NS2, "metadata"))) {
                return xmlState_inMetadata;
            }
            break;
        default:
            break;
    }

    return xmlState_ignore;
}

SvLocal SvHashTable
attrsToHash(const char * *attrs)
{
    SvHashTable res = SvHashTableCreate(20, NULL);
    if (!res) {
        return NULL;
    }
    for (int i = 0; attrs[i] && attrs[i + 1]; i += 2) {
        log_parse("attr %s %s", attrs[i], attrs[i + 1]);
        SvString key = SvStringCreate(attrs[i], NULL);
        SvString val = SvStringCreate(attrs[i + 1], NULL);
        if (key && val) {
            SvHashTableInsert(res, (SvObject) key, (SvObject) val);
        }
        SVTESTRELEASE(key);
        SVTESTRELEASE(val);
    }
    return res;
}

SvLocal void
parseStyle(QBTTMLStyle style, SvHashTable attrs)
{
    SvIterator it;
    SvString key, val;

    if (!attrs) {
        return;
    }

    it = SvHashTableKeysIterator(attrs);

    if (!style->rawAttrs) {
        style->rawAttrs = SvHashTableCreate(7, NULL);
    }

    while ((key = (SvString) SvIteratorGetNext(&it))) {
        log_parse("Key: %s", SvStringCString(key));
        val = (SvString) SvHashTableFind(attrs, (SvObject) key);
        if (compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS, "color")) ||
            compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS2, "color")) ||
            compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS3, "color"))) {
            style->textColor = parseColor(SvStringCString(val));
        } else if (compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS, "backgroundColor")) ||
                   compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS2, "backgroundColor")) ||
                   compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS3, "backgroundColor"))) {
            style->backgroundColor = parseColor(SvStringCString(val));
        } else if (compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS, "fontSize")) ||
                   compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS2, "fontSize")) ||
                   compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS3, "fontSize"))) {
            struct lengthDesc ld;
            parseLength(SvStringCString(val), &ld);
        } else if (compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS, "extent")) ||
                   compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS2, "extent")) ||
                   compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS3, "extent"))) {
            const char *cval = SvStringCString(val);
            struct lengthDesc ld1, ld2;
            int parsed = parseLength(cval, &ld1);
            log_parse("parse_len: %d", parsed);
            if (parsed < 0) {
                log_parse("invalid length");
                break;
            }
            if (cval[parsed] != ' ') {
                log_parse("invalid separator");
                break;
            }
            parsed = parseLength(cval + parsed + 1, &ld2);

            log_parse("Ld1 %lf Ld2 %lf", ld1.v, ld2.v);

            style->extent.w = ld1;
            style->extent.h = ld2;
        } else if (compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS, "origin")) ||
                   compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS2, "origin")) ||
                   compareAttr(SvStringCString(key), LITERAL_COMBINE(STYLE_NS3, "origin"))) {
            const char *cval = SvStringCString(val);
            struct lengthDesc ld1, ld2;
            int parsed = parseLength(cval, &ld1);
            log_parse("parse_len: %d", parsed);
            if (parsed < 0) {
                log_parse("invalid length");
                break;
            }
            if (cval[parsed] != ' ') {
                log_parse("invalid separator");
                break;
            }
            parsed = parseLength(cval + parsed + 1, &ld2);

            log_parse("Ld1 %lf Ld2 %lf", ld1.v, ld2.v);

            style->origin.x = ld1;
            style->origin.y = ld2;
        } else {
            continue;
        }

        SvHashTableInsert(style->rawAttrs, (SvObject) key, (SvObject) val);
    }
}


SvLocal void
copyStyle(QBTTMLStyle dest, QBTTMLStyle src)
{
    dest->textColor = src->textColor;
    dest->backgroundColor = src->backgroundColor;
    dest->align = src->align;
    dest->fontSize = src->fontSize;

    dest->origin = src->origin;
    dest->extent = src->extent;
}


SvLocal void
mergeStyle(QBTTMLStyle dest, QBTTMLStyle src)
{
    if (src && dest) {
        parseStyle(dest, src->rawAttrs);
    }
}


SvLocal void
parseContainer(QBTTMLSubsParser self, QBTTMLContainer cont, SvHashTable attrs)
{
    SvIterator it;
    SvString key, val;

    if (!attrs) {
        return;
    }

    it = SvHashTableKeysIterator(attrs);

    while ((key = (SvString) SvIteratorGetNext(&it))) {
        val = (SvString) SvHashTableFind(attrs, (SvObject) key);
        if (compareAttr(SvStringCString(key), LITERAL_COMBINE(TT_NS, "begin")) || compareAttr(SvStringCString(key), LITERAL_COMBINE(TT_NS2, "begin"))) {
            cont->start = parseTimeExpr(SvStringCString(val));
            log_parse("Start: %lf", cont->start);
        } else if (compareAttr(SvStringCString(key), LITERAL_COMBINE(TT_NS, "end")) || compareAttr(SvStringCString(key), LITERAL_COMBINE(TT_NS2, "end"))) {
            cont->end = parseTimeExpr(SvStringCString(val));
            log_parse("End: %lf", cont->end);
        } else if (compareAttr(SvStringCString(key), LITERAL_COMBINE(XML_NS, "id"))) {
            cont->id = SVRETAIN(val);
        } else if (compareAttr(SvStringCString(key), LITERAL_COMBINE(TT_NS, "region")) || compareAttr(SvStringCString(key), LITERAL_COMBINE(TT_NS2, "region"))) {
            SvObject region = SvHashTableFind(self->regions, (SvObject) val);
            if (region) {
                SVRETAIN(region);
                SVTESTRELEASE(cont->region);
                cont->region = (QBTTMLRegion) region;
            }
        } else if (compareAttr(SvStringCString(key), LITERAL_COMBINE(TT_NS, "style")) || compareAttr(SvStringCString(key), LITERAL_COMBINE(TT_NS2, "style"))) {
            SvObject style = SvHashTableFind(self->styles, (SvObject) val);
            if (style) {
                mergeStyle(cont->style, (QBTTMLStyle) style);
            }
        }
    }
}


SvLocal void
initializeStyle(QBTTMLSubsParser self, QBTTMLStyle style, SvHashTable attrs)
{
    SvString styleName = (SvString) SvHashTableFind(attrs, (SvObject) SVSTRING(TT_NS "style"));
    if (!styleName) {
        styleName = (SvString) SvHashTableFind(attrs, (SvObject) SVSTRING(TT_NS2 "style"));
    }
    if (!styleName) {
        styleName = (SvString) SvHashTableFind(attrs, (SvObject) SVSTRING("style"));
    }
    QBTTMLStyle parentStyle;

    if (styleName && (parentStyle = (QBTTMLStyle) SvHashTableFind(self->styles, (SvObject) styleName))) {
        mergeStyle(style, parentStyle);
    } else {
        mergeStyle(style, self->defaultStyle);
    }
}


SvLocal void
initializeContainer(QBTTMLSubsParser self, QBTTMLContainer new, QBTTMLContainer parent)
{
    if (!parent || !parent->region) {
        new->region = SVRETAIN(self->defaultRegion);
    } else {
        new->region = SVRETAIN(parent->region);
    }

    new->style = (QBTTMLStyle) SvTypeAllocateInstance(QBTTMLStyle_getType(), NULL);

    if (parent && parent->style) {
        copyStyle(new->style, parent->style);
    } else {
        copyStyle(new->style, self->defaultStyle);
    }


    if (parent) {
        new->start = parent->start;
        new->end = parent->end;
    }
}

SvLocal void
parseDocumentProperties(QBTTMLSubsParser self)
{
}

SvLocal SvArray
createImagesArrayFromMetadata(QBTTMLMetadata metaData)
{
    SvArray images = SvArrayCreate(NULL);
    SvObject obj;
    SvIterator it = SvArrayIterator(metaData->descendants);
    while ((obj = SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf(obj, QBTTMLSMPTETTImage_getType())) {
            continue;
        }
        SvArrayAddObject(images, obj);
    }

    return images;
}

SvLocal SvString
createLocalBackgroundImageId(SvString backgroundImageId_)
{
    const char *backgroundImageId = SvStringCString(backgroundImageId_);
    if (backgroundImageId[0] != '#') {
        return NULL;
    }

    return SvStringCreateSubString(backgroundImageId_, 1, SvStringLength(backgroundImageId_) - 1, NULL);
}

SvLocal void
embeddableContainerAddAnnonTextSpan(QBTTMLSubsParser self, QBTTMLEmbeddableContainer cont, SvString annonSpanText)
{
    // assert - help for clang false positive
    assert(cont);

    QBTTMLSpan anonSpan = (QBTTMLSpan) SvTypeAllocateInstance(QBTTMLSpan_getType(), NULL);
    initializeContainer(self, (QBTTMLContainer) anonSpan, (QBTTMLContainer) cont);
    anonSpan->text = SVTESTRETAIN(annonSpanText);
    if (!cont->descendants) {
        cont->descendants = SvArrayCreateWithCapacity(10, NULL);
    }
    SvArrayAddObject(cont->descendants, (SvObject) anonSpan);
    SVRELEASE(anonSpan);
}

SvLocal void
startElementHandler(void *self_, const XML_Char *name, const XML_Char * *attrs)
{
    QBTTMLSubsParser self = self_;
    SvObject newObject = NULL;
    SvObject currentObject = NULL;
    SvHashTable attributes = NULL;
    SvObject objId = NULL;
    xmlState newState, oldState = self->parserState;

    log_parse("start element %s", name);

    currentObject = stackGetObj(self);
    stackPush(self, (const char *) name);
    newState = checkPushedState(self, name);

    attributes = attrsToHash(attrs);

    if (SvStringBufferLength(self->buf) && (oldState == xmlState_inSpan || oldState == xmlState_inP)) {
        SvString annonSpanText = SvStringBufferCreateContentsString(self->buf, NULL);
        embeddableContainerAddAnnonTextSpan(self, (QBTTMLEmbeddableContainer) currentObject, annonSpanText);
        SVTESTRELEASE(annonSpanText);
    }


    if (self->parserState != xmlState_ignore && newState == xmlState_ignore) {
        self->ignoreLevel = self->stackSize;
    }

    objId = SvHashTableFind(attributes, (SvObject) SVSTRING(XML_NS "id"));

    switch (newState) {
        case xmlState_inRegion:
            newObject = SvTypeAllocateInstance(QBTTMLRegion_getType(), &self->error);
            break;
        case xmlState_inStyle:
            if (self->parserState == xmlState_inRegion) {
                newObject = SVRETAIN(((QBTTMLContainer) currentObject)->style);
            } else {
                newObject = (SvObject) QBTTMLStyleCreate();
            }
            break;
        case xmlState_inDiv:
            newObject = SvTypeAllocateInstance(QBTTMLDiv_getType(), &self->error);
            break;
        case xmlState_inP:
            newObject = SvTypeAllocateInstance(QBTTMLParagraph_getType(), &self->error);
            break;
        case xmlState_inSpan:
            newObject = SvTypeAllocateInstance(QBTTMLSpan_getType(), &self->error);
            break;
        case xmlState_inTT:
            parseDocumentProperties(self);
            break;
        case xmlState_inMetadata:
            newObject = SvTypeAllocateInstance(QBTTMLMetadata_getType(), &self->error);
            break;
        case xmlState_inBr:
            //Add hard line break
            embeddableContainerAddAnnonTextSpan(self, (QBTTMLEmbeddableContainer) currentObject, SVSTRING("\n"));
            break;
        case xmlState_inSMPTEImage:
            newObject = SvTypeAllocateInstance(QBTTMLSMPTETTImage_getType(), &self->error);
            break;
        case xmlState_inRoot:
        case xmlState_inHead:
        case xmlState_inBody:
        case xmlState_inLayout:
        case xmlState_inStyling:
        default:
            break;
    }

    if (newObject) {
        QBTTMLStyle style = NULL;
        if (SvObjectIsInstanceOf(newObject, QBTTMLContainer_getType())) {
            initializeContainer(self, (QBTTMLContainer) newObject, (QBTTMLContainer) currentObject);
            parseContainer(self, (QBTTMLContainer) newObject, attributes);
            style = ((QBTTMLContainer) newObject)->style;
        } else if (SvObjectIsInstanceOf(newObject, QBTTMLStyle_getType())) {
            style = (QBTTMLStyle) newObject;
            initializeStyle(self, style, attributes);
        }

        if (SvObjectIsInstanceOf(newObject, QBTTMLDiv_getType())) {
            SvString backgroundImageId = (SvString) SvHashTableFind(attributes, (SvObject) SVSTRING(SMPTETT_NS "backgroundImage"));
            if (!backgroundImageId) {
                backgroundImageId = (SvString) SvHashTableFind(attributes, (SvObject) SVSTRING("backgroundImage"));
            }
            // we handle only local images
            ((QBTTMLDiv) newObject)->backgroundImageId = backgroundImageId ? createLocalBackgroundImageId(backgroundImageId) : NULL;
        }

        if (style) {
            parseStyle(style, attributes);
        }

        if (self->parserState == xmlState_inLayout && SvObjectIsInstanceOf(newObject, QBTTMLRegion_getType())) {
            SvHashTableInsert(self->regions, objId, newObject);
            log_parse("Added global region %s", SvStringCString((SvString) objId));
        } else if (self->parserState == xmlState_inStyling && SvObjectIsInstanceOf(newObject, QBTTMLStyle_getType())) {
            SvHashTableInsert(self->styles, objId, newObject);
            log_parse("Added global style %s", SvStringCString((SvString) objId));
        } else if (self->parserState == xmlState_inMetadata && SvObjectIsInstanceOf(newObject, QBTTMLSMPTETTImage_getType())) {
            if (objId) {
                SvHashTableInsert(self->images, objId, newObject);
                log_parse("Added global image %s", SvStringCString((SvString) objId));
            }
        }
    }

    stackSetObj(self, newObject);
    SVTESTRELEASE(newObject);

    self->parserState = newState;

    log_parse("New state %d", self->parserState);


    SvStringBufferReset(self->buf);

    SVTESTRELEASE(attributes);
}

SvLocal void
endElementHandler(void *self_, const XML_Char *name)
{
    QBTTMLSubsParser self = self_;
    xmlState oldState = self->parserState;
    SvObject poppedObject = stackGetObj(self);
    SvObject currentObject;

    log_parse("end element %s", name);

    if (strcmp((const char *) name, stackTop(self))) {
        self->parserState = xmlState_error;
        return;
    }

    SVTESTRETAIN(poppedObject);
    stackPop(self);

    if (self->parserState == xmlState_ignore && self->stackSize >= self->ignoreLevel) {
        SVTESTRELEASE(poppedObject);
        return;
    }

    if (!self->stackSize) {
        self->parserState = xmlState_inRoot;
        SVTESTRELEASE(poppedObject);
        return;
    }
    currentObject = stackGetObj(self);
    self->parserState = tagToXmlState(stackTop(self));

    if (SvStringBufferLength(self->buf) && poppedObject) {
        if ((oldState == xmlState_inSpan) && !((QBTTMLEmbeddableContainer) poppedObject)->descendants) {
            ((QBTTMLSpan) poppedObject)->text = SvStringBufferCreateContentsString(self->buf, NULL);
        } else if (oldState == xmlState_inSpan || oldState == xmlState_inP) {
            QBTTMLEmbeddableContainer cont = (QBTTMLEmbeddableContainer) poppedObject;
            QBTTMLSpan anonSpan = (QBTTMLSpan) SvTypeAllocateInstance(QBTTMLSpan_getType(), NULL);
            initializeContainer(self, (QBTTMLContainer) anonSpan, (QBTTMLContainer) poppedObject);
            anonSpan->text = SvStringBufferCreateContentsString(self->buf, NULL);
            if (!cont->descendants)
                cont->descendants = SvArrayCreateWithCapacity(10, NULL);
            SvArrayAddObject(cont->descendants, (SvObject) anonSpan);
            SVRELEASE(anonSpan);
        } else if (oldState == xmlState_inSMPTEImage && SvStringBufferLength(self->buf) < 1024 * 100) {
            QBTTMLSMPTETTImage image = (QBTTMLSMPTETTImage) poppedObject;
            const char *base64Data = SvStringBufferGetStorage(self->buf);
            SvData tmpImageData = SvDataCreateWithLength(3 * SvStringBufferGetLength(self->buf) / 4, NULL);
            int tmpImageDataLen = QBBase64Decode_(base64Data, SvStringBufferGetLength(self->buf), (unsigned char *) SvDataBytes(tmpImageData), true);
            if (tmpImageDataLen > 0)
                image->imageData = SvDataCreateSubrange(tmpImageData, 0, tmpImageDataLen, NULL);
            SVRELEASE(tmpImageData);
        }
    }

    if (SvObjectIsInstanceOf(poppedObject, QBTTMLParagraph_getType())) {
        SvArrayAddObject(self->paragraphs, poppedObject);
    }

    if (SvObjectIsInstanceOf(poppedObject, QBTTMLDiv_getType())) {
        SvString backgroundImageId = ((QBTTMLDiv) poppedObject)->backgroundImageId;
        if (backgroundImageId) {
            QBTTMLSMPTETTImage image = (QBTTMLSMPTETTImage) SvHashTableFind(self->images, (SvObject) backgroundImageId);
            if (image) {
                QBTTMLDiv divElem = (QBTTMLDiv) poppedObject;
                SVTESTRELEASE(divElem->backgroundImage);
                divElem->backgroundImage = SVTESTRETAIN(image->imageData);
            }
        }
    }

    switch (self->parserState) {
        case xmlState_inBody:
            SvArrayAddObject(self->divs, poppedObject);
            break;
        case xmlState_inDiv:
            if (SvObjectIsInstanceOf(poppedObject, QBTTMLMetadata_getType()) && SvObjectIsInstanceOf(currentObject, QBTTMLDiv_getType())) {
                QBTTMLDiv cont = (QBTTMLDiv) currentObject;
                SvArray images = createImagesArrayFromMetadata((QBTTMLMetadata) poppedObject);
                // In div metadata only one image does make sense
                if (SvArrayCount(images) == 1) {
                    QBTTMLSMPTETTImage image = (QBTTMLSMPTETTImage) SvArrayAt(images, 0);
                    cont->backgroundImage = SVTESTRETAIN(image->imageData);
                }
                SVTESTRELEASE(images);
            }
        // No break here on purpose
        case xmlState_inP:
        case xmlState_inSpan:
            if (currentObject && SvObjectIsInstanceOf(currentObject, QBTTMLEmbeddableContainer_getType())) {
                QBTTMLEmbeddableContainer cont = (QBTTMLEmbeddableContainer) currentObject;
                if (!cont->descendants) {
                    cont->descendants = SvArrayCreateWithCapacity(10, NULL);
                }
                if (poppedObject) {
                    SvArrayAddObject(cont->descendants, poppedObject);
                }
            }
            break;
        case xmlState_inMetadata:
            if (currentObject && SvObjectIsInstanceOf(currentObject, QBTTMLMetadata_getType())) {
                QBTTMLMetadata metadata = (QBTTMLMetadata) currentObject;
                if (!metadata->descendants) {
                    metadata->descendants = SvArrayCreateWithCapacity(10, NULL);
                }
                if (poppedObject) {
                    SvArrayAddObject(metadata->descendants, poppedObject);
                }
            }
            break;
        case xmlState_inRoot:
        case xmlState_inTT:
        case xmlState_inHead:
        case xmlState_inLayout:
        case xmlState_inStyling:
        case xmlState_inRegion:
        case xmlState_inStyle:
        case xmlState_inBr:
        default:
            break;
    }

    SVTESTRELEASE(poppedObject);

    if (!self->stackSize) {
        return;
    }

    SvStringBufferReset(self->buf);
}

SvLocal void characterDataHandler(void *self_, const XML_Char *s, int length)
{
    QBTTMLSubsParser self = self_;

    SvStringBufferAppendCStringWithLength(self->buf, s, length, NULL);
}

SvLocal void resetProperties(QBTTMLSubsParser self)
{
    // The choice of values 32 and 15 are based on this being the maximum number of columns and rows defined by [CEA-608-E].
    // https://www.w3.org/TR/ttml1/#parameter-attribute-cellResolution
    self->properties.cellResolution.x = 32;
    self->properties.cellResolution.y = 15;

    self->properties.frameRate = 30;
}

void
QBTTMLSubsParserParse(QBTTMLSubsParser self, unsigned char *data, size_t length, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!self->parser) {
        if (!(self->parser = XML_ParserCreateNS("UTF-8", ':'))) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory, "Couldn't create XML parser");
            goto out;
        }
        XML_SetHashSalt(self->parser, rand());
        XML_SetUserData(self->parser, self);
        XML_SetElementHandler(self->parser, startElementHandler, endElementHandler);
        XML_SetCharacterDataHandler(self->parser, characterDataHandler);
    }

    if (XML_Parse(self->parser, (const char *) data, length, 0) == XML_STATUS_ERROR) {
        int off, len;
        XML_GetInputContext(self->parser, &off, &len);
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           self->error, "SWL/XmlPage: parsing error '%s' (line %d, column %d)",
                                           XML_ErrorString(XML_GetErrorCode(self->parser)),
                                           (int) XML_GetCurrentLineNumber(self->parser),
                                           (int) XML_GetCurrentColumnNumber(self->parser));
        if (self->error) {
            self->error = NULL;
        }
        goto out;
    }
out:
    SvErrorInfoPropagate(error, errorOut);
    return;
}

SvArray
QBTTMLSubsParserGetParagraphs(QBTTMLSubsParser self)
{
    return self->paragraphs;
}

SvArray
QBTTMLSubsParserGetDivs(QBTTMLSubsParser self)
{
    return self->divs;
}

struct QBTTMLDocumentProperties_
QBTTMLSubsParserGetDocumentProperties(QBTTMLSubsParser self)
{
    return self->properties;
}

void
QBTTMLSubsParserReset(QBTTMLSubsParser self)
{
    self->parserState = xmlState_inRoot;
    while (self->stackSize)
        stackPop(self);
    self->ignoreLevel = 0;
    resetProperties(self);
    SvHashTableRemoveAllObjects(self->regions);
    SvHashTableRemoveAllObjects(self->styles);
    SvHashTableRemoveAllObjects(self->images);
    SvArrayRemoveAllObjects(self->divs);
    SvArrayRemoveAllObjects(self->paragraphs);

    if (self->parser) {
        XML_ParserFree(self->parser);
        self->parser = NULL;
    }
}

SvString
QBTTMLParagraphCreateText(QBTTMLParagraph self)
{
    QBTTMLSpan span;
    SvIterator it;
    SvString text;
    SvStringBuffer buf = SvStringBufferCreateWithCapacity(32, NULL);

    //TODO: Add proper Metadata/Animation anonymous spans handling
    SvArray descendants = ((QBTTMLEmbeddableContainer) self)->descendants;
    if (descendants) {
        it = SvArrayIterator(((QBTTMLEmbeddableContainer) self)->descendants);
        while ((span = (QBTTMLSpan) SvIteratorGetNext(&it))) {
            if (span->text) {
                SvStringBufferAppendCString(buf, SvStringCString(span->text), NULL);
            }
            if (((QBTTMLEmbeddableContainer) span)->descendants) {
                SvIterator anonSpanIt = SvArrayIterator(((QBTTMLEmbeddableContainer) span)->descendants);
                QBTTMLSpan anonSpan;
                while ((anonSpan = (QBTTMLSpan) SvIteratorGetNext(&anonSpanIt))) {
                    if (anonSpan->text) {
                        SvStringBufferAppendCString(buf, SvStringCString(anonSpan->text), NULL);
                    }
                }
            }
        }
    }

    text = SvStringBufferCreateContentsString(buf, NULL);
    SVRELEASE(buf);
    return text;
}
