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

// This file was generated on 2015-11-23 using omniidl
// (beeing part of omniORB_4_1) with Cubiware C backend.
/*****************************************************************************
******************************************************************************
******************************** DO NOT EDIT *********************************
******************************************************************************
******************************************************************************/

#ifndef IDL__IDIALOG__
#define IDL__IDIALOG__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


typedef enum {
    QBRPCIDialogButton_OK = 0,
    QBRPCIDialogButton_Cancel = 1,
    QBRPCIDialogButton_Open = 2,
    QBRPCIDialogButton_Save = 3,
    QBRPCIDialogButton_Yes = 4,
    QBRPCIDialogButton_No = 5,
    QBRPCIDialogButton_Install = 6,
    QBRPCIDialogButton_Delete = 7,
} QBRPCIDialogButton;

typedef struct {
    int16_t id;
    SvString title;
    SvString message;
    int16_t buttonsMask;
} QBRPCIDialogData;

typedef struct {
    int16_t id;
    QBRPCIDialogButton selectedButton;
} QBRPCIDialogActionData;

struct QBRPCIDialog_ {
    uint16_t (*dialogRequest)(SvObject self_, QBRPCIDialogData const * data, int16_t* __returnValue__);
    uint16_t (*dialogRequestAsync)(SvObject self_, SvObject caller, uint16_t* callId, QBRPCIDialogData const * data);

    uint16_t (*dialogAction)(SvObject self_, QBRPCIDialogActionData const * actionData, int16_t* __returnValue__);
    uint16_t (*dialogActionAsync)(SvObject self_, SvObject caller, uint16_t* callId, QBRPCIDialogActionData const * actionData);
};
typedef const struct QBRPCIDialog_ *QBRPCIDialog;
#define QBRPC_IDIALOG_NAME "QBRPC::IDialog"

SvInterface QBRPCIDialog_getInterface(void);


struct QBRPCIDialogAsyncCallListener_ {
    void (*dialogRequestAsyncCallback)(SvObject self_, uint16_t callId, int16_t __returnValue__);
    void (*dialogActionAsyncCallback)(SvObject self_, uint16_t callId, int16_t __returnValue__);
};
typedef const struct QBRPCIDialogAsyncCallListener_ *QBRPCIDialogAsyncCallListener;

SvInterface QBRPCIDialogAsyncCallListener_getInterface(void);


#endif // IDL__IDIALOG__
