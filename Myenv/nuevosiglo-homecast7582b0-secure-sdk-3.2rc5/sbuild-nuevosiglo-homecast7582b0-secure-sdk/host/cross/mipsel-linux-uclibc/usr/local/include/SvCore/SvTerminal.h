/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_TERMINAL_H_
#define SV_TERMINAL_H_

/**
 * @file SvTerminal.h
 * @brief Various terminal-related macros and global variables
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvTerminal Terminal support
 * @ingroup SvCore
 * @{
 *
 * Various terminal-related macros and global variables.
 **/

/**
 * ANSI escape code switching text color to black.
 **/
#define SvTerminalColorBlack       "\033[30m"
/**
 * ANSI escape code switching text color to red.
 **/
#define SvTerminalColorRed         "\033[31m"
/**
 * ANSI escape code switching text color to green.
 **/
#define SvTerminalColorGreen       "\033[32m"
/**
 * ANSI escape code switching text color to yellow.
 **/
#define SvTerminalColorYellow      "\033[33m"
/**
 * ANSI escape code switching text color to blue.
 **/
#define SvTerminalColorBlue        "\033[34m"
/**
 * ANSI escape code switching text color to magenta.
 **/
#define SvTerminalColorMagenta     "\033[35m"
/**
 * ANSI escape code switching text color to cyan.
 **/
#define SvTerminalColorCyan        "\033[36m"
/**
 * ANSI escape code switching text color to white.
 **/
#define SvTerminalColorWhite       "\033[37m"
/**
 * ANSI escape code switching to default text color.
 **/
#define SvTerminalColorNormal      "\033[m"

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
