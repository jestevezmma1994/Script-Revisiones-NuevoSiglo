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

#ifndef QB_TTML_SUBS_PARSER_H_
#define QB_TTML_SUBS_PARSER_H_

#include <stdint.h>

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvData.h>

#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvTextTypes.h>
#include <CAGE/Core/Sv2DRect.h>


/**
 * @defgroup QBTTMLSubsParser TTML subtitles parser
 * @ingroup QBTTMLSubsParser
 * @{
 **/

struct lengthDesc {
    double v;
    enum {
        unitType_pixel,
        unitType_percentage,
        unitType_em, // WTF
        unitType_c,
    } unitType;
};

typedef struct QBTTMLDocumentProperties_ QBTTMLDocumentProperties;

struct QBTTMLDocumentProperties_ {
    struct {
        unsigned x, y;
    } cellResolution;

    unsigned int frameRate;
};

typedef struct QBTTMLStyle_ *QBTTMLStyle;

struct QBTTMLStyle_ {
    struct SvObject_ super_;

    SvColor textColor;
    SvColor backgroundColor;
    SvTextAlignment align;
    int fontSize;

    struct {
        struct lengthDesc x, y;
    } origin;

    struct {
        struct lengthDesc w, h;
    } extent;

    SvHashTable rawAttrs;
};

typedef struct QBTTMLMetadata_ *QBTTMLMetadata;

struct QBTTMLMetadata_ {
    struct SvObject_ super_;

    SvArray descendants;
    SvHashTable rawAttrs;
};

typedef struct QBTTMLSMPTETTImage_ *QBTTMLSMPTETTImage;

struct QBTTMLSMPTETTImage_ {
    struct SvObject_ super_;

    SvData imageData;
};

typedef struct QBTTMLContainer_ *QBTTMLContainer;
typedef struct QBTTMLRegion_ *QBTTMLRegion;

struct QBTTMLContainer_ {
    struct SvObject_ super_;

    SvString id;

    double start, end;

    QBTTMLStyle style;
    QBTTMLRegion region;
};

struct QBTTMLRegion_ {
    struct QBTTMLContainer_ super_;
};

typedef struct QBTTMLEmbeddableContainer_ *QBTTMLEmbeddableContainer;

struct QBTTMLEmbeddableContainer_ {
    struct QBTTMLContainer_ super_;

    SvArray descendants;
};

typedef struct QBTTMLDiv_ *QBTTMLDiv;

struct QBTTMLDiv_ {
    struct QBTTMLEmbeddableContainer_ super_;

    SvString backgroundImageId;
    SvData backgroundImage;
};

typedef struct QBTTMLParagraph_ *QBTTMLParagraph;

struct QBTTMLParagraph_ {
    struct QBTTMLEmbeddableContainer_ super_;
};

typedef struct QBTTMLSpan_ *QBTTMLSpan;

struct QBTTMLSpan_ {
    struct QBTTMLEmbeddableContainer_ super_;

    SvString text;
};

typedef struct QBTTMLSubsParser_ *QBTTMLSubsParser;

/**
 * Create QBTTMLSubsParser class instance.
 *
 * @memberof QBTTMLSubsParser
 *
 * @param[out] errorOut        error information
 * @return                     QBTTMLSubsParser instance
 **/
QBTTMLSubsParser
QBTTMLSubsParserCreate(SvErrorInfo *errorOut);

/* Parse passed raw subtitles data.
 *
 * @param[in] self          QBTTMLSubsParser handle
 * @param[in] data          raw subtitles data pointer
 * @param[in] length        data length
 * @param[out] errorOut     error info
 **/
void
QBTTMLSubsParserParse(QBTTMLSubsParser self, unsigned char *data,
                      size_t length, SvErrorInfo *errorOut);

/**
 * Get array of QBTTMLParagraph instances.
 *
 * @param[in] self          QBTTMLSubsParser handle
 * @return                  array of QBTTMLParagraph instances
 **/
SvArray
QBTTMLSubsParserGetParagraphs(QBTTMLSubsParser self);

/**
 * Get array of QBTTMLDiv instances.
 *
 * @param[in] self          QBTTMLSubsParser handle
 * @return                  array of QBTTMLDiv instances
 **/
SvArray
QBTTMLSubsParserGetDivs(QBTTMLSubsParser self);

/**
 * Get the TTML document properties
 *
 * @param[in] self          QBTTMLSubsParser handle
 * @return                  TTML document properties
 **/
struct QBTTMLDocumentProperties_
QBTTMLSubsParserGetDocumentProperties(QBTTMLSubsParser self);

/**
 * Reset the TTML subtitle parser state and release all of its objects.
 *
 * @param[in] self          QBTTMLSubsParser handle
 **/
void
QBTTMLSubsParserReset(QBTTMLSubsParser self);

/**
 * Create SvString containing text extracted from QBTTMLParagraph object.
 *
 * @memberof QBTTMLParagraph
 *
 * @param[in] self  QBTTMLParagraph handle
 * @return          SvString object with text, NULL in case of error
 **/
SvString
QBTTMLParagraphCreateText(QBTTMLParagraph self);

/**
 * @}
 **/

#endif
