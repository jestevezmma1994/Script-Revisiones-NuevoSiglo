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

#ifndef SV_COMMAND_LINE_H_
#define SV_COMMAND_LINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file SvCommandLine.h
 * @brief Utility for retrieving command line arguments
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvCommandLine Retrieving command line arguments
 * @ingroup SvQuirks
 * @{
 **/

/**
 * Get an array of command line arguments.
 *
 * @param[out] argc      number of arguments == number of items in @a argv
 * @param[out] argv      array of command line arguments, @c NULL on error
 **/
extern void
SvGetCommandLine(int *argc, const char ***argv);

/**
 * Set command line arguments.
 *
 * This function will change command line arguments returned by
 * SvGetCommandLine() to the new ones passed as arguments. It will not
 * modify the command line as seen from outside the process, for example
 * the one shown by ps command.
 *
 * @param[in] argv      array of command line arguments terminated by @c NULL
 **/
extern void
SvSetCommandLine(const char **argv);


/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
