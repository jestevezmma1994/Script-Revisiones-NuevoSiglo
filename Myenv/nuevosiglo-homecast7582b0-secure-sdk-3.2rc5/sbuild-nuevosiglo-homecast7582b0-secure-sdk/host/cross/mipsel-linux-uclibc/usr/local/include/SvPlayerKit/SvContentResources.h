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

#ifndef SV_CONTENT_RESOURCES_H_
#define SV_CONTENT_RESOURCES_H_

/** Tuner ID to read the data from. */
#define SV_CONTENT_RESOURCE__TUNER      "qb.res.tuner"

#define SV_CONTENT_RESOURCE__UDP_ID      "qb.res.udpip"

#define SV_CONTENT_RESOURCE__PR_AUTH_TOKEN  "qb.res.pr_auth_token"

#define SV_CONTENT_RESOURCE__PR_DEVID    "qb.res.pr_devid"

#define SV_CONTENT_RESOURCE__PR_CONAX_ID_FUTURE     "qb.res.pr_conaxid"
#define SV_CONTENT_RESOURCE__PR_CONAX_TOKEN_FUTURE  "qb.res.pr_conaxtoken"

/**
 * Additional GET parameter(s) that will be appended to PlayReady licence request url.
 * String in format key1=value1[&key2=value2][&key3=value3]...
 **/
#define SV_CONTENT_RESOURCE__PR_LICENCE_REQUEST_PARAMS "qb.res.pr_licence_request_params"

/**
 * Custom data replacement during PlayReady licence acquisition.
 **/
#define SV_CONTENT_RESOURCE__PR_LICENCE_REQUEST_CUSTOM_DATA "qb.res.pr_licence_request_custom_data"

/**
 * SSL parameters used during PlayReady licence acquisition.
 **/
#define SV_CONTENT_RESOURCE__PR_LICENCE_REQUEST_SSL_PARAMS "qb.res.pr_licence_request_ssl_params"

/**
 * Implementation type of QBPlayReady used for playback.
 * @see QBPlayReadyImplementation enum from DRM/QBPlayReady/QBPlayReadyManager
 **/
#define SV_CONTENT_RESOURCE__PLAYREADY_TYPE    "qb.res.playready_type"

#endif
