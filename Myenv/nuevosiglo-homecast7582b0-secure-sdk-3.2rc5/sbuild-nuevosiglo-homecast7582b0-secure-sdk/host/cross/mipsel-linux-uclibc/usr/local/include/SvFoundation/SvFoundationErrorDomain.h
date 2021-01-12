/*******************************************************************************
 ** Sentivision K.K. Software License Version 1.1
 **
 ** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau, application
 ** service provider, or similar business, or make any other use of this Software
 ** without express written permission from Sentivision K.K.

 ** Any User wishing to make use of this Software must contact Sentivision K.K.
 ** to arrange an appropriate license. Use of the Software includes, but is not
 ** limited to:
 ** (1) integrating or incorporating all or part of the code into a product
 **     for sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 *******************************************************************************/

#ifndef SvFoundationErrorDomain_h
#define SvFoundationErrorDomain_h

/**
 * @file SvFoundationErrorDomain.h SvFoundation framework error domain
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


/**
 * @defgroup SvFoundationErrorDomain Error domain
 * @ingroup SvFoundation
 * @{
 *
 * Error codes returned by modules of this framework.
 **/

/**
 * SvFoundation error domain.
 **/
#define SvFoundationErrorDomain "com.sentivision.SvFoundation"

/**
 * SvFoundation error domain error codes.
 **/
enum SvFoundationError {
    /** @cond */
    SvFoundationError_reserved = 0,
    /** @endcond */
    /// used interface is not implemented by class
    SvFoundationError_interfaceNotImplemented,
    /// C string contains invalid characters for specified encoding
    SvFoundationError_invalidStringEncoding,
    /// URI format is invalid
    SvFoundationError_malformedURI,
    /// file already exists
    SvFoundationError_fileExists,
    /** @cond */
    SvFoundationError_last
    /** @endcond */
};

/**
 * @}
 **/


#endif