/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_OSK_KEY_H_
#define QB_OSK_KEY_H_

/**
 * @file QBOSKKey.h
 * @brief On Screen Keyboard key class
 **/

/**
 * @defgroup QBOSKKey On Screen Keyboard key class
 * @ingroup QBOSKCore
 * @{
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <CAGE/Core/SvBitmap.h>
#include <QBOSK/QBOSKTypes.h>


/// OSK key type.
typedef enum {
    QBOSKKeyType_default = 0,   ///< ordinary key for entering character(s)
    QBOSKKeyType_backspace,     ///< BACKSPACE key, removes a character
    QBOSKKeyType_enter,         ///< ENTER key, submits entered text
    QBOSKKeyType_layout,        ///< a key to change keyboard layout
    QBOSKKeyType_defaultWithLayout, ///< ordinary key for entering character from event-specified layout (see @ref QBOSKKeyPressedEvent)
} QBOSKKeyType;

typedef struct QBOSKKeyVariantPosition_ {
    unsigned short int x;
    unsigned short int y;
} QBOSKKeyVariantPosition;

struct QBOSKKeyVariant_ {
    bool disabled;
    bool hidden;
    struct {
        unsigned short int width, height;
    } size;

    QBOSKKeyVariantPosition position;
    SvString caption;
    SvString bitmap;
    SvString value;
    unsigned short int fontSize;      ///< custom font size, 0 if global font size is used
};

typedef enum QBOSKKeyShape_ {
    QBOSKKeyShape_standalone = 1,
    QBOSKKeyShape_leftConnected = 2,
    QBOSKKeyShape_rightConnected = 4,
    QBOSKKeyShape_topConnected = 8,
    QBOSKKeyShape_bottomConnected = 16
} QBOSKKeyShape;

/**
 * OSK key class.
 **/
struct QBOSKKey_ {
    struct SvObject_ super_;

    QBOSKKeyType type;
    SvString ID;
    SvBitmap keyBg;
    SvBitmap disabledKeyBg;
    unsigned int variantsCount;
    struct QBOSKKeyVariant_ *variants;
};


/**
 * Get runtime type identification object representing OSK key class.
 *
 * @return QBOSKKey type identification object
 **/
extern SvType
QBOSKKey_getType(void);

/**
 * Create new OSK key.
 *
 * @param[in] ID            key identifier
 * @param[in] variantsCount number of keyboard variants supported
 * @param[out] errorOut     error info
 * @return                  created proxy, @c NULL in case of error
 **/
extern QBOSKKey
QBOSKKeyCreate(SvString ID,
               unsigned int variantsCount,
               SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif
