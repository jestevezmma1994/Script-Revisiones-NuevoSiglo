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

#ifndef QB_KEYMAP_FILTER_H_
#define QB_KEYMAP_FILTER_H_

/**
 * @file QBKeyMapFilter.h
 * @brief Keyboard map input filter class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBInput/QBKeyMap.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBKeyMapFilter Keyboard map input filter class
 * @ingroup QBInputEventFilters
 * @{
 **/

/**
 * Keyboard map input filter class.
 * @class QBKeyMapFilter
 * @extends SvObject
 **/
typedef struct QBKeyMapFilter_ *QBKeyMapFilter;


/**
 * Get runtime type identification object representing
 * keyboard map input filter class.
 *
 * @return keyboard map input filter class
 **/
extern SvType QBKeyMapFilter_getType(void);

/**
 * Initialize keyboard map input filter object.
 *
 * @memberof QBKeyMapFilter
 *
 * @param[in] self         keyboard map input filter handle
 * @param[in] mapFilePath  path to the keyboard map file
 * @param[out] errorOut    error info
 * @return                 self, @c NULL in case of error
 **/
extern QBKeyMapFilter QBKeyMapFilterInit(QBKeyMapFilter self,
                                         const char *mapFilePath,
                                         SvErrorInfo *errorOut);

/**
 * Get keyboard map used by the filter.
 *
 * @memberof QBKeyMapFilter
 *
 * @param[in] self         keyboard map input filter handle
 * @return                 keyboard map, @c NULL in case of error
 **/
extern QBKeyMap QBKeyMapFilterGetKeyMap(QBKeyMapFilter self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
