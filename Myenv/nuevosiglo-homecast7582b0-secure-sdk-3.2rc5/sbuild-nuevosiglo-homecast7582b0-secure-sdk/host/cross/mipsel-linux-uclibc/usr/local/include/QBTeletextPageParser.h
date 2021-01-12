/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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
****************************************************************************/

#ifndef QBTELETEXTPAGEPARSER_H_
#define QBTELETEXTPAGEPARSER_H_

#include <stdint.h>
#include <QBTeletextReceiver.h>

typedef struct QBTeletextParsedPage_t *QBTeletextParsedPage;

typedef struct QBTeletextParser_t *QBTeletextParser;
QBTeletextParser QBTeletextParserNew(void);
QBTeletextParsedPage QBTeletextParsePage(QBTeletextParser self, QBTeletextRawPage data);

#define TELETEXT_LINES   25
#define TELETEXT_COLUMNS 40
#define TELETEXT_HEADER_LENGTH 32

#define QB_TTEXT_FLAG_DWIDTH      0x001
#define QB_TTEXT_FLAG_DHEIGHT     0x002
#define QB_TTEXT_FLAG_MOSAIC      0x004
#define QB_TTEXT_FLAG_SEPARATED   0x008
#define QB_TTEXT_FLAG_HOLD_MOS    0x010
#define QB_TTEXT_FLAG_BOX         0x020
#define QB_TTEXT_FLAG_FAKE_SPACE  0x100
#define QB_TTEXT_FLAG_CONCEAL     0x200
#define QB_TTEXT_FLAG_FLASH       0x400

typedef enum
{
  QB_TELETEXT_COLOR_TRANSPARENT,
  QB_TELETEXT_COLOR_RED,
  QB_TELETEXT_COLOR_GREEN,
  QB_TELETEXT_COLOR_YELLOW,
  QB_TELETEXT_COLOR_BLUE,
  QB_TELETEXT_COLOR_MAGENTA,
  QB_TELETEXT_COLOR_CYAN,
  QB_TELETEXT_COLOR_GRAY,

  QB_TELETEXT_COLOR_BLACK
}QBTeletextColor; ///teletext colors, order is importanat !

struct QBTeletextCell_s
{
  uint32_t teletextChar;
  QBTeletextColor textColor;
  QBTeletextColor bgColor;
  uint32_t flags;
};

/**
 * The QBTeletextCell type.
 *
 * This type should be used to store information about
 * one cell in teletext line.
 **/
typedef struct QBTeletextCell_s QBTeletextCell;


struct QBTeletextParsedPage_t {
    struct SvObject_ super_;
    QBTeletextCell cells[TELETEXT_LINES][TELETEXT_COLUMNS];
    int langCode;

    int pageNum, subCode;
    bool suppressHeaderFlag;
    bool subtitleFlag;
};

struct QBTeletextParserCallbacks_t {
    void (*printPage)   (void* target, QBTeletextParsedPage page);
    void (*printHeader) (void *target, QBTeletextCell *cells, int pageNum);
};

void QBTeletextParserSetCallbacks(QBTeletextParser self, const struct QBTeletextParserCallbacks_t* callbacks, void *target);
void QBTeletextParserSetHints(QBTeletextParser self, const char* lang);



#endif /* QBTELETEXTPAGEPARSER_H_ */
