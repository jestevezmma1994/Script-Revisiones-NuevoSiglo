/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau,
 ** application service provider, or similar business, or make any other use of
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#include "overscanMargins.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <QBConf.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>
#include <SvJSON/SvJSONParse.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <safeFiles.h>
#include <QBPlatformHAL/QBPlatformUtil.h>

/* Parsing configuration file with overscan margins. Format (JSON) like in example below:
{
"overscan-margins": [
        {
                "system": "PAL",
                "margin-left": "2.5%",
                "margin-right": "2.5%",
                "margin-top": "2.5%",
                "margin-bottom": "2.5%"
        },
        {
                "system": "NTSC",
                "margin-left": "3%",
                "margin-right": "3%",
                "margin-top": "5px",
                "margin-bottom": "5px"
        }
]
}
 */

struct OverscanMargins {
    int left;
    int right;
    int top;
    int bottom;
};

enum {
    EC_NONE = 0,
    EC_ELEMENT_NOT_FOUND,
    EC_ELEMENT_IS_NOT_STRING,
    EC_NOT_A_NUMBER,
    EC_TOO_BIG,
    EC_NUMBER_AFTER_DOT_TOO_LONG,
    EC_NO_SUFFIX,
    EC_INVALID_SUFFIX,
    EC_FRACTION_PIXELS
};

/* Some information used mainly to produce usefull error messages if configuration is invalid. */
struct parser_context {
    SvHashTable htable;
    int arrayElementNumber;
    const char * elementName;
    const char * elementValue;

    int errorCode;
    /* Unfortunately I have not line number from config file, because JSON parser do not hold
     * this information. */
    int errorPositionInValue;
};

static void print_error(const struct parser_context * ctx)
{
    switch (ctx->errorCode) {
    case EC_ELEMENT_NOT_FOUND:
        SvLogError("overscan-margins parser: element %s not found (array element %d; first is 1)",
                   ctx->elementName, ctx->arrayElementNumber + 1);
        break;
    case EC_ELEMENT_IS_NOT_STRING:
        SvLogError("overscan-margins parser: element %s is not string (array element %d; first is 1)",
                   ctx->elementName, ctx->arrayElementNumber + 1);
        break;
    case EC_NOT_A_NUMBER:
        SvLogError("overscan-margins parser: value of %s is not a number (array element %d; first is 1)",
                   ctx->elementName, ctx->arrayElementNumber + 1);
        break;
    case EC_TOO_BIG:
        SvLogError("overscan-margins parser: value of %s is too big (array elament %d; first is 1)",
                   ctx->elementName, ctx->arrayElementNumber + 1);
        break;
    case EC_NUMBER_AFTER_DOT_TOO_LONG:
        SvLogError("overscan-margins parser: value of %s has more than two fractional digits (array element %d; first is 1)",
                   ctx->elementName, ctx->arrayElementNumber + 1);
        break;
    case EC_NO_SUFFIX:
        SvLogError("overscan-margins parser: value of %s has no suffix. Allowed are \"%%\" (percentage) and \"px\" (pixels) [array element %d; first is 1]",
                   ctx->elementName, ctx->arrayElementNumber + 1);
        break;
    case EC_INVALID_SUFFIX:
        SvLogError("overscan-margins parser: value of %s has invalid suffix \"%s\". Allowed are \"%%\" (percentage) and \"px\" (pixels) [array element %d; first is 1]",
                   ctx->elementName, ctx->elementValue + ctx->errorPositionInValue, ctx->arrayElementNumber + 1);
        break;
    case EC_FRACTION_PIXELS:
        SvLogError("overscan-margins parser: value of %s specified in pixels has fractional part (array element %d; first is 1)",
                   ctx->elementName, ctx->arrayElementNumber + 1);
        break;
    }
}

static bool parseMargin(struct parser_context * ctx, int * pixels,
                        const char * element_name, int base_size)
{
    SVAUTOSTRING(element_name_string, element_name);
    SvValue value = (SvValue) SvHashTableFind(ctx->htable, (SvGenericObject) element_name_string);
    ctx->elementName = element_name;

    if (! value) {
        ctx->errorCode = EC_ELEMENT_NOT_FOUND;
        return false;
    }

    if (!SvObjectIsInstanceOf((SvObject) value, SvValue_getType()) || !SvValueIsString(value)) {
        ctx->errorCode = EC_ELEMENT_IS_NOT_STRING;
        return false;
    }

    const char * text = SvStringCString(SvValueGetString(value));
    ctx->elementValue = text;

    char * endptr;
    unsigned long v = strtoul(text, &endptr, 10);
    unsigned long frac100 = 0;
    if (*endptr == '.') {
        endptr++;
        if (isdigit(*endptr)) {
            frac100 += 10 * (*endptr - '0');
            endptr++;
        } else {
            ctx->errorCode = EC_NOT_A_NUMBER;
            ctx->errorPositionInValue = endptr - text;
            return false;
        }

        if (isdigit(*endptr)) {
            frac100 += *endptr - '0';
            endptr++;
        }

        if (isdigit(*endptr)) {
            ctx->errorCode = EC_NUMBER_AFTER_DOT_TOO_LONG;
            ctx->errorPositionInValue = endptr - text;
            return false;
        }
    }

    if (endptr == text) {
        ctx->errorCode = EC_NOT_A_NUMBER;
        ctx->errorPositionInValue = 0;
        return false;
    }

    /* strip whitespace after number and before suffix */
    while (*endptr && (isspace(*endptr))) {
        ++endptr;
    }

    if ((endptr != text) && (strcmp(endptr, "%") == 0)) {
        *pixels = base_size * (v*100 + frac100) / 10000;
        return true;
    } else if ((endptr != text) && (strcmp(endptr, "px") == 0)) {
        if (frac100 != 0) {
            ctx->errorCode = EC_FRACTION_PIXELS;
            ctx->errorPositionInValue = endptr - text;
            return false;
        }
        *pixels = v;
        return true;
    } else if ((endptr != text) && (*endptr != '\0')) {
        ctx->errorCode = EC_INVALID_SUFFIX;
        ctx->errorPositionInValue = endptr - text;
        return false;
    } else {
        ctx->errorCode = EC_NO_SUFFIX;
        ctx->errorPositionInValue = endptr - text;
        return false;
    }
}

static bool parseJSONFragment(struct parser_context * ctx, struct OverscanMargins * margins,
                              const char * tv_system, int width, int height)
{
    SvValue systemVal = (SvValue) SvHashTableFind(ctx->htable, (SvGenericObject) SVSTRING("system"));
    ctx->elementName = "system";

    if (! systemVal) {
        ctx->errorCode = EC_ELEMENT_NOT_FOUND;
        return false;
    }

    if (!SvObjectIsInstanceOf((SvObject) systemVal, SvValue_getType()) || !SvValueIsString(systemVal)) {
        ctx->errorCode = EC_ELEMENT_IS_NOT_STRING;
        return false;
    }

    if (strcmp(SvStringCString(SvValueGetString(systemVal)), tv_system) != 0) {
        return false;
    }

    if (parseMargin(ctx, &margins->left, "margin-left", width) &&
        parseMargin(ctx, &margins->right, "margin-right", width) &&
        parseMargin(ctx, &margins->top, "margin-top", height) &&
        parseMargin(ctx, &margins->bottom, "margin-bottom", height))
    {
        return true;
    } else {
        return false;
    }
}

static bool loadOverscanMargins(const char *tv_system, struct OverscanMargins *margins, int width, int height, SvString fileName)
{
    bool configurationFound = false;
    if (! tv_system) {
        SvLogWarning("%s(): tv_system == NULL", __FUNCTION__);
        return false;
    }

    char *buffer = NULL;
    QBFileToBuffer(SvStringCString(fileName), &buffer);
    if (! buffer) {
        SvLogNotice("Overscan margins configuration: file not present.");
        return false;
    }

    SvGenericObject root = SvJSONParseString(buffer, false, NULL);
    free(buffer);
    if (! root) {
        SvLogError("Overscan margins configuration: invalid format (JSON is expected).");
        return false;
    }

    if (!SvObjectIsInstanceOf(root, SvHashTable_getType())) {
        SvLogError("Overscan margins configuration: JSON element \"overscan-margins\" not found.");
        goto end_of_parsing;
    }

    SvGenericObject obj = SvHashTableFind((SvHashTable) root, (SvGenericObject) SVSTRING("overscan-margins"));

    if (!obj || !SvObjectIsInstanceOf(obj, SvArray_getType())) {
        SvLogError("Overscan margins configuration: JSON element \"overscan-margins\" is not array type.");
        goto end_of_parsing;
    }

    SvArray conf = (SvArray) obj;
    SvIterator iter = SvArrayIterator(conf);
    SvGenericObject elem;

    struct parser_context ctx = {
        .errorCode = 0
    };

    while ((elem = SvIteratorGetNext(&iter))) {
        if (SvObjectIsInstanceOf(elem, SvHashTable_getType())) {
            ctx.htable = (SvHashTable) elem;
            if (parseJSONFragment(&ctx, margins, tv_system, width, height)) {
                configurationFound = true;
                goto end_of_parsing;
            }
            if (ctx.errorCode) {
                print_error(&ctx);
                goto end_of_parsing;
            }
        }
        ctx.arrayElementNumber++;
    }

end_of_parsing:
    SVRELEASE(root);
    if (configurationFound) {
        return true;
    } else {
        SvLogError("Overscan margins configuration: cannot find configuration for TV system \"%s\"", tv_system);
        return false;
    }
}

bool QBOverscanMarginsGetRect(Sv2DRect * outputRect, QBVideoOutputConfig cfg, SvString fileName)
{
    struct OverscanMargins margins = {.left = 0};
    const char *tvSystemName = QBPlatformGetTVSystemName(cfg.mode == QBOutputStandard_480i59 ? QBTVSystem_NTSC : QBTVSystem_PAL);
    if (loadOverscanMargins(tvSystemName, &margins, cfg.width, cfg.height, fileName)) {
        SvLogNotice("Loaded overscan margins, left: %dpx, right: %dpx, top: %dpx, bottom: %dpx",
                    margins.left, margins.right, margins.top, margins.bottom);
        outputRect->x = margins.left;
        outputRect->width = cfg.width - margins.left - margins.right;
        outputRect->y = margins.top;
        outputRect->height = cfg.height - margins.top - margins.bottom;
        return true;
    } else {
        SvLogError("Cannot load overscan margins from configuration. Using defaults.");
        // don't use 3% of output space on left and right
        outputRect->x = cfg.width * 3 / 100;
        outputRect->width = cfg.width - 2 * outputRect->x;
        // don't use 2% of output space on top and bottom
        outputRect->y = cfg.height * 2 / 100;
        outputRect->height = cfg.height - 2 * outputRect->y;
        return false;
    }
}
