/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include "magicCode.h"
#include <string.h>
#include <stdlib.h>

struct QBMagicCode_ {
    int len;
    uint16_t* code;
    uint16_t* typed;
};

void QBMagicCodeAdd(QBMagicCode* self, uint16_t code)
{
    for (uint16_t i = 1; i < self->len; i++)
        self->typed[i - 1] = self->typed[i];
    self->typed[self->len - 1] = code;
}

bool QBMagicCodeMatches(QBMagicCode* self)
{
    return !memcmp(self->code, self->typed, self->len * sizeof(uint16_t));
}

QBMagicCode* QBMagicCodeCreate(uint16_t* magicCode, int len)
{
    QBMagicCode* self = calloc(1, sizeof(struct QBMagicCode_));
    self->len = len;
    self->code = calloc(1, self->len * sizeof(uint16_t));
    self->typed = calloc(1, self->len * sizeof(uint16_t));

    memcpy(self->code, magicCode, self->len * sizeof(uint16_t));

    return self;
}

void QBMagicCodeDestroy(QBMagicCode* self)
{
    free(self->code);
    free(self->typed);
    free(self);
}
