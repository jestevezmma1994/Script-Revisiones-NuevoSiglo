/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBPLATFORMVBI_H
#define QBPLATFORMVBI_H

/**
 * @file QBPlatformVBI.h Data structure and functions for passing data through VBI
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup QBPlatformVBI VBI handling
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Single teletext line, to be shown via VBI
 **/
struct QBPlatformVBITeletextLine_ {
    /// pointer to data representing teletext line, exactly 43 bytes long.
    const uint8_t *data;
    /// should the data be sent in odd/even field
    bool oddField;
    /// on which VBI line should the data be sent. May be -1, if don't care.
    int vbiLineNumber;
};
typedef struct QBPlatformVBITeletextLine_ QBPlatformVBITeletextLine;


/**
 * Enable or disable VBI Teletext
 *
 * @param[in] outputID  video output ID (see QBPlatformGetOutputConfig())
 * @param[in] active    @c true if VBI teletext should be enabled, @c false otherwise
 * @return              @c 0 on success, @c -1 in case of error
 **/
int QBPlatformSetVBITeletext(unsigned int outputID, bool active);

/**
 * Request showing an array of teletext lines described by QBPlatformVBITeletextLine. It may happen that not
 * all lines are shown, the count of shown lines is returned via processedLinesCnt. To show them, the function
 * must be called again later on.
 *
 * If lines will belong to multiple fields, or there will be more lines than a single field can carry
 * then the lines will be automatically split into multiple fields.
 *
 * @param[in] outputID  video output ID
 * @param[in] lines     array of teletext lines
 * @param[in] linesCnt  size of lines array
 * @param[out] processedLinesCnt amount of shown lines from lines array.
 * @return              @c 0 on success, @-1 otherwise.
 **/
int QBPlatformVBIShowTeletextLines(unsigned outputID, QBPlatformVBITeletextLine* lines, int linesCnt, int *processedLinesCnt);

/**
 * @}
 **/

#endif
