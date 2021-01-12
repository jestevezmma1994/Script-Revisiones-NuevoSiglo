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
#ifndef QBGFXAPP_H
#define QBGFXAPP_H

#include <QBApp.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvHashTable.h>
#include <stdint.h>

/**
 * @file QBGfxApp.h
 * @brief QBGfxApp class provides interface for managing graphical aspects of external app.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBGfxApp class.
 * @ingroup QBAppsManager
 * @{
 **/

/**
 * @brief QBGfxApp is a class derived from QBApp, enables user to
 * manage gfx settings, eg. size, window position, aspect ratio.
 */
typedef struct QBGfxApp_* QBGfxApp;

/**
  * Enumeration describing display's aspect ratio.
  */
typedef enum {
    QBGfxAppAspectRatio_Unknown = 0, /**< unknown display ratio */
    QBGfxAppAspectRatio_16x9 = 1, /**< 16 : 9 */
    QBGfxAppAspectRatio_4x3 = 2, /**< 4 : 3 */
} QBGfxAppAspectRatio;

/**
  * Enumeration describing display's mode,
  */
typedef enum {
    QBGfxAppMode_Disabled = 0, /**< displaying is disabled */
    QBGfxAppMode_FullScreen = 1, /**< displaying in full screen mode */
    QBGfxAppMode_Windowed = 2, /**< displaying in windowed mode */
} QBGfxAppMode;

/**
 * @brief Structure containing app's gfx params.
 */
typedef struct {
    uint16_t x; /**< x coordinate */
    uint16_t y; /**< y coordinate */
    uint16_t width; /**< width */
    uint16_t height; /**< height */
    QBGfxAppAspectRatio ratio; /**< picture's aspect ratio */
    QBGfxAppMode mode; /**< display's mode */
} QBGfxAppParams;

/**
 * @brief This function is used to obtain current gfx params from QBGfxApp.
 * Caller must implement QBGfxAppListener iterface to read current params.
 *
 * @param[in] self      QBGfxApp handle
 */
void QBGfxAppGetParams(QBGfxApp self);

/**
 * @brief This function is used to set gfx params in QBGfxApp.
 *
 * @param[in] self      QBGfxApp handle
 * @param[in] params    QBGfxAppParams handle
 */
void
QBGfxAppSetParams(QBGfxApp self, QBGfxAppParams params);

/**
 * Method returns hash table with names of services (keys, SvString) and
 * arrays of strings listing interfaces (SvArray of SvStrings)implemented
 * by QBGfxApp.
 *
 * @param [in] self     QBGfxApp handle
 *
 * @return              hash table with services and implemented interfaces
 */
SvHashTable QBGfxAppGetImplementedInterfaces(QBGfxApp self);

/**
 * @brief QBGfxAppFactory is a class used to create QBGfxApp objects.
 */
typedef struct QBGfxAppFactory_* QBGfxAppFactory;

/**
 * @brief This function is used to create QBGfxAppFactory instances.
 *
 * @return          created QBGfxAppFactory handle
 */
QBGfxAppFactory
QBGfxAppFactoryCreate(void);

/**
 * @brief Function returns handle to QBGfxAppListener interface.
 *
 * @return          QBGfxAppListener interface handle
 */
SvInterface
QBGfxAppListener_getInterface(void);

struct QBGfxAppListener_ {
    /**
     * Notification about gfx parameters being set.
     *
     * @param[in] self               self (listener) handle
     * @param[in] application        QBApp handler
     *
     * @return                       @a 0 in case of unhandled method, @c positive value in case of handled method
     *
     **/
    unsigned int (*setParamsNotification)(SvObject self_, QBApp application);
    /**
     * Notification about current gfx parameters.
     *
     * @param[in] self               self (listener) handle
     * @param[in] application        QBApp handler
     * @param[in] params             currentParams
     *
     * @return                       @a 0 in case of unhandled method, @c positive value in case of handled method
     *
     **/
    unsigned int (*getParamsNotification)(SvObject self_, QBApp application, QBGfxAppParams params);
};
typedef struct QBGfxAppListener_* QBGfxAppListener;

/**
 * @}
 **/
#endif // QBGFXAPP_H
