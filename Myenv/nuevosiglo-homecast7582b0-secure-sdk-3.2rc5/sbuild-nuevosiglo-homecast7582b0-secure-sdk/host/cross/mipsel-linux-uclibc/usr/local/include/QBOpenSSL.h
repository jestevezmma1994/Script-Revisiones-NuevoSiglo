/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_OPEN_SSL_H_
#define QB_OPEN_SSL_H_

/**
 * @file QBOpenSSL.h Cubiware's openssl initialization component
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {

}
#endif
/**
 * @defgroup QBOpenSSL QBOpenSSL
 * @ingroup Utils
 * @{
 */

/**
 * @brief It performs initialization of openssl library and set up it for thread safe working.
 * OpenSSL can safely be used in multi-threaded applications provided that at least two callback functions are set,
 * locking_function and threadid_func.
 *
 * Locking function is needed to perform locking on shared data structures. (Note that OpenSSL uses a number of global data structures that will be implicitly shared whenever multiple threads use OpenSSL.)
 * Multi-threaded applications will crash at random if it is not set.
 *
 * Threadid function is needed to record the currently-executing thread's identifier into id.
 *
 * QBOpenSSLInit methods provides locking and threadid functions for openssl library.
 *
 */
void QBOpenSSLInit(void);

/**
 * @brief Clean up resources.
 */
void QBOpenSSLDeinit(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
