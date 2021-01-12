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

#ifndef SV_PROXY_SETTINGS_H_
#define SV_PROXY_SETTINGS_H_

#define SV_PROXY_ADDRESS          "NETWORK_PROXY_ADDRESS"
#define SV_PROXY_PORT             "NETWORK_PROXY_PORT"
#define SV_PROXY_STATUS           "NETWORK_PROXY_STATUS"
#define SV_PROXY_PASS             "NETWORK_PROXY_PASS"
#define SV_PROXY_USER             "NETWORK_PROXY_USER"

#define SV_DEFAULT_PROXY_PORT     8080


/**
 * Initialize proxy settings.
 *
 * This function initializes proxy settings using values
 * from environment variables.
 *
 * @return @c 0 on success, @c -1 otherwise
 **/
extern int svProxyInit(void);

/**
 * Check if proxy settings are initialized.
 * @return @c 1 if initialized, @c 0 otherwise
 **/
extern int svProxyIsInited(void);

/**
 * Deinitialize proxy settings.
 **/
extern void svProxyDeinit(void);

/**
 * Set proxy settings.
 * @param[in] proxyStatus  new status: @c 1 to enable proxy, @c 0 to disable
 * @param[in] proxyAddress new proxy server address
 * @param[in] proxyPort    new proxy server port
 * @param[in] proxyUser    new proxy user name, @c NULL for none
 * @param[in] proxyPass    new proxy password, @c NULL for none
 * @return @c 0 on success, @c -1 otherwise
 **/
extern int svProxySet(unsigned short proxyStatus,
                      const char *proxyAddress,
                      unsigned short proxyPort,
                      const char *proxyUser,
                      const char *proxyPass);

/**
 * Get proxy status.
 * @return current proxy status: @c 1 if enabled, @c 0 if disabled
 **/
extern unsigned short svProxyGetStatus(void);

/**
 * Get proxy server address.
 * @return a string containing current proxy server address
 **/
extern const char* svProxyGetAddress(void);

/**
 * Get proxy server port.
 * @return current proxy server port
 **/
extern unsigned short svProxyGetPort(void);

/**
 * Get proxy user name.
 * @return a string containing current proxy server user name
 **/
extern const char* svProxyGetUser(void);

/**
 * Get proxy password.
 * @return a string containing current proxy server password
 **/
extern const char* svProxyGetPass(void);

/**
 * Get proxy user name and password.
 * @return a string containing current proxy server user name and password
 *         in 'user:password' format
 **/
extern const char* svProxyGetUserAndPassword(void);

/**
 * Get proxy servr address and port.
 * @return a string containing current proxy server address and port
 *         in 'address:port' format
 **/
extern const char* svProxyGetAddressAndPort(void);


#endif
