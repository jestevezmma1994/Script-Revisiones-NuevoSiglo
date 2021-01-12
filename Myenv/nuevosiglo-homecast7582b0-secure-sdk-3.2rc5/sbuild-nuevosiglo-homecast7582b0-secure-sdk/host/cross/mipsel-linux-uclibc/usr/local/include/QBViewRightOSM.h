/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QBVIEWRIGHTOSM_H_
#define QBVIEWRIGHTOSM_H_

/**
 * @file QBViewRightOSM.h QBViewRight DVB On screen message API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include "QBViewRightTriggers.h"
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <CAGE/Core/SvColor.h>

/**
 * @defgroup QBViewRightOSM QBViewRightOSM ViewRight On Screen Messages.
 * @ingroup QBViewRight
 * @{
 *
 **/

/**
 * @brief Position of OSM's anchor
 */
typedef enum QBViewRightOSMAnchor_e {
    QBViewRightOSMAnchor_LeftTop,     /**< left-top at the screen */
    QBViewRightOSMAnchor_LeftCenter,  /**< left-center at the screen */
    QBViewRightOSMAnchor_LeftBottom,  /**< left-bottom at the screen */
    QBViewRightOSMAnchor_CenterTop,   /**< center-top at the screen */
    QBViewRightOSMAnchor_Center,      /**< center at the screen */
    QBViewRightOSMAnchor_CenterBottom, /**< center-bottom at the screen */
    QBViewRightOSMAnchor_RightTop,    /**< right-top at the screen */
    QBViewRightOSMAnchor_RightCenter, /**< right-center at the screen */
    QBViewRightOSMAnchor_RightBottom  /**< right-bottom at the screen */
} QBViewRightOSMAnchor;

/**
 * @brief Align of the text at the On Screen Message.
 */
typedef enum QBViewRightOSMTextAlign_e {
    QBViewRightOSMTextAlign_Left,     /**< left text align */
    QBViewRightOSMTextAlign_Center,   /**< center text align */
    QBViewRightOSMTextAlign_Right     /**< right text align */
} QBViewRightOSMTextAlign;

/**
 * @brief Type of the pin.
 */
typedef enum QBViewRightPinType_e {
    QBViewRightPinType_ParentalControlPin2 = 0, /**< Parental control PIN */
    QBViewRightPinType_IPPV_PC_1 = 1,          /**< Pay per view PIN and parentel control pin 1 */
    QBViewRightPinType_STBLockPin = 2,         /**< STB lock PIN */
    QBViewRightPinType_HomeShopping = 3,       /**< Home shopping PIN */
    QBViewRightPinType_NonScPin = 0x80,        /**< Non smart card PIN  */
} QBViewRightPinType;

/**
 * @brief Source of PIN check.
 */
typedef enum QBViewRightCheckPinSource_e {
    QBViewRightCheckPinSource_livePlayback, /**< PIN checked issued by live playback */
    QBViewRightCheckPinSource_pvrPlayback,  /**< PIN checked issued by pvr playback */
    QBViewRightCheckPinSource_pvrRecord,    /**< PIN checked issued by pvr record */
} QBViewRightCheckPinSource;

/**
 * @brief Indicates which text should be shown at the PIN pop-up
 */
typedef enum QBViewRightPinTextSelector_e {
    QBViewRightPinTextSelector_ParentalControl = 0,            /**< Parental control text */
    QBViewRightPinTextSelector_IPPV = 1,                       /**< Pay per  view text */
    QBViewRightPinTextSelector_ParentalControlNonSmartCard = 3, /**< Non smartcard parental control text */
    QBViewRightPinTextSelector_ResumingEvent = 4,              /**< Resuming event text */
    QBViewRightPinTextSelector_SelectEvent = 5,                /**< Select event text text */
    QBViewRightPinTextSelector_STBInternalNonSC = 0xFE,        /**< Check NonSC PIN for internal purpose */
    QBViewRightPinTextSelector_STBInternalSC = 0xFF,           /**< Check SC PIN for internal purpose */
} QBViewRightPinTextSelector;

/**
 * @brief Type of Smartcard MMI notification.
 */
typedef enum QBViewRightSmartcardMMINotificationType_e {
    QBViewRightSmartcardMMINotificationType_OK,     /**< OK status notification. It should terminate warning and error notifications */
    QBViewRightSmartcardMMINotificationType_SmartcardNotPresent,   /**< Error notification */
    QBViewRightSmartcardMMINotificationType_HardwareError,   /**< Error notification */
    QBViewRightSmartcardMMINotificationType_SmartcardRejected,   /**< Error notification */
    QBViewRightSmartcardMMINotificationType_NonSmartcardMode,   /**< Info notification */
    QBViewRightSmartcardMMINotificationType_NonSmartcardModeWithCard, /**< Info notification */
    QBViewRightSmartcardMMINotificationType_UpdateRequired,   /**< Info notification */
} QBViewRightSmartcardMMINotificationType;

/**
 * @brief Type of descrambling MMI notification.
 */
typedef enum QBViewRightDescramblingMMINotificationType_e {
    QBViewRightDescramblingMMINotificationType_OK,     /**< OK status notification. It should terminate warning and error notifications */
    QBViewRightDescramblingMMINotificationType_AuthorisationRequired,  /**< Terminate warning and error notifications except PIN requests */
    QBViewRightDescramblingMMINotificationType_NoPairing,  /**< There is no pairing */
    QBViewRightDescramblingMMINotificationType_Error,   /**< Error notification */
} QBViewRightDescramblingMMINotificationType;

/**
 * @interface QBViewRightOSMListener
 * Object which implements that interface will be notified about:
 * On Screen Messages, PIN pop-ups and MMI notifications.
 */
typedef struct QBViewRightOSMListener_s* QBViewRightOSMListener;

/**
 * @class QBViewRightOSM
 * @brief ViewRight On Screen Message. OSM contains a user text message or an application trigger.
 */
typedef struct QBViewRightOSM_s* QBViewRightOSM;

/**
 * @struct QBViewRightOSMListener_s
 * @brief See @ref QBViewRightOSMListener
 */
struct QBViewRightOSMListener_s {
    /**
     * @brief Notifies about the need for showing dialog based on OSM.
     * @param[in] self_ instance of listener
     * @param[in] osm On Screen Message from QBViewRight library
     */
    void (*show)(SvObject self_, QBViewRightOSM osm);

    /**
     * @brief Notifies about the need for checking PIN.
     * @param[in] self_ instance of listener
     * @param[in] sessionId cas session id
     * @param[in] pinSource source of PIN request
     * @param[in] pinType type of the pin listed at @ref QBViewRightPinType
     * @param[in] textSelector defines text which should be displayed at the pin pop-up.
     */
    void (*checkPin)(SvObject self_, int sessionId, QBViewRightCheckPinSource pinSource, QBViewRightPinType pinType, QBViewRightPinTextSelector textSelector);

    /**
     * @brief Notifies about the need for checking Pay Per View PIN.
     * @param[in] self_ instance of listener
     * @param[in] pinSource source of PIN request
     * @param[in] pinType type of the pin listed at @ref QBViewRightPinType
     * @param[in] availableCredit string with available credit
     * @param[in] costOfEvent string with cost of the event
     */
    void (*orderPin)(SvObject self_, QBViewRightCheckPinSource pinSource, QBViewRightPinType pinType, SvString availableCredit, SvString costOfEvent);

    /**
     * @brief Notifies about the need for showing Smartcard MMI notification on the screen.
     * @param[in] self_ instance of listener
     * @param[in] type type of notification defined by @ref QBViewRightSmartcardMMINotificationType
     */
    void (*showSmartcardMMINotification)(SvObject self_, QBViewRightSmartcardMMINotificationType type);

    /**
     * @brief Notifies about the need for showing descrambling MMI notification on the screen.
     * @param[in] self_ instance of listener
     * @param[in] type type of notification defined by @ref QBViewRightDescramblingMMINotificationType
     */
    void (*showDescramblingMMINotification)(SvObject self_, QBViewRightDescramblingMMINotificationType type);
};

/**
 * @brief Get instance of @ref SvInterface related to @ref QBViewRightOSMListener.
 * @return Instance of @ref SvInterface.
 */
SvInterface QBViewRightOSMListener_getInterface(void);
/**
 *  @brief Create On Screen Message object.
 * @param[in] pabMsg text with special formating options
 * @param[in] wMode  if (wMode&0x100)=0x100 take width and height given in wW and wH, else
 *               ignore wW and wH and calculate width and height of the box according
 *               to the message.
 * @param[in] wX horizontal position of the anchor in pixels
 * @param[in] wY vertical position of the anchor in pixels
 * @param[in] wW width of the OSM in pixels
 * @param[in] wH height of the OSM in pixels
 * @param[in] bBackground selects one of 16 colors for the backgrounds
 * @param[in] bAlpha give the alpha-blending value for the dialog
 * @param[in] bForeground selects one of 16 colors for the text
 * @param[out] errorOut returns error if it occurred.
 * @return QBViewRightOSM object if success, NULL in the other case.
 */
QBViewRightOSM QBViewRightOSMCreate(uint8_t* pabMsg, int16_t wMode,
                                    int16_t wX, int16_t wY, int16_t wW, int16_t wH,
                                    int8_t bBackground, int8_t bAlpha, int8_t bForeground, SvErrorInfo *errorOut);

/**
 * @brief Set display parameters.
 * @param[in] self self instance of a OSM
 * @param[in] bDisplayMode display mode
 *        If (bDisplayMode&1)==0 the window can be removed by the customer pressing a selected button
 * @param[in] wDuration duration in seconds
 *        a duration of 0 means the window is not automatically removed.
 * @param[out] errorOut returns error if it occurred
 */
void QBViewRightOSMSetDisplayParams(QBViewRightOSM self, uint8_t bDisplayMode, uint16_t wDuration, SvErrorInfo *errorOut);
/**
 * @brief Indicate whether OSM can be removed by a user.
 * If not it has to shown by the fixed number of seconds.
 * @param[in] self instance of a OSM
 * @return true if OSM is removable, false if it cannot be removed by the user
 */
bool QBViewRightOSMIsRemovable(QBViewRightOSM self);

/**
 * @brief Indicate whether OSM has background.
 * @param[in] self instance of a OSM
 * @return true if OSM has background, false if not
 */
bool QBViewRightOSMHasBackground(QBViewRightOSM self);

/**
 * @brief Get Trigger from On screen message. OSM system can be used for delivering triggers
 * @param[in] self instance of on screen message
 * @return instance of a trigger or NULL when OSM doesn't include trigger.
 */
QBViewRightGenericTrigger QBViewRightOSMGetTrigger(QBViewRightOSM self);

/**
 * @brief Indicate if OSM is actually a trigger.
 * @param[in] self instance of a OSM
 * @return true if OSM is a trigger, false if it is just on screen message.
 */
bool QBViewRightOSMIsTrigger(QBViewRightOSM self);

/**
 * Get text with html markups.
 * @param[in] self instance of a OSM
 * @return text with html markups
 */
SvString QBViewRightOSMGetMarkupText(QBViewRightOSM self);
/**
 * Get number of seconds, how long OSM should be displayed.
 * A duration of 0 means the window is not automatically removed.
 * @param[in] self instance of a OSM
 * @return duration of OSM in seconds. If duration is 0, OSM pop-up shouldn't be removed automatically
 */
uint QBViewRightOSMGetDuration(QBViewRightOSM self);
/**
 * Get background color of 'On Screen Message'.
 * @param[in] self instance of a OSM
 * @return background color
 */
SvColor QBViewRightOSMGetBackgroundColor(QBViewRightOSM self);
/**
 * Get width of an OSM dialog in pixels
 * @param[in] self instance of a OSM
 * @return width of an OSM dialog in pixels
 */
uint QBViewRightOSMGetWidth(QBViewRightOSM self);
/**
 * Get height of an OSM dialog in pixels
 * @param[in] self instance of a OSM
 * @return height of an OSM dialog in pixels
 */
uint QBViewRightOSMGetHeight(QBViewRightOSM self);
/**
 * Get horizontal position of OSM anchor
 * @param[in] self instance of a OSM
 * @return horizontal position in pixels
 */
uint QBViewRightOSMGetXOffset(QBViewRightOSM self);
/**
 * Get vertical position of OSM anchor
 * @param[in] self instance of a OSM
 * @return vertical position in pixels
 */
uint QBViewRightOSMGetYOffset(QBViewRightOSM self);
/**
 * Get value of alpha channel of the OSM [0, 0xFF]
 * 0x00 - dialog is transparent
 * 0xFF - dialog is solid
 * @param[in] self instance of a OSM
 * @return value of alpha channel of the OSM [0, 0xFF]
 */
uint QBViewRightOSMGetAlpha(QBViewRightOSM self);
/**
 * Indicate if should we use width and height given in OSM.
 * If not, we should  calculate width and height of the dialog according
 * to the text message.
 * @param[in] self instance of a OSM
 * @return true if we should use given size, false if size has to be adjusted to the user text
 */
bool QBViewRightOSMUseGivenSize(QBViewRightOSM self);
/**
 * Get position of the anchor.
 * @param[in] self instance of a OSM
 * @return OSM anchor
 */
QBViewRightOSMAnchor QBViewRightOSMGetAnchor(QBViewRightOSM self);

/**
 * Get align of the text
 * @param[in] self instance of a OSM
 * @return align of the text
 */
QBViewRightOSMTextAlign QBViewRightOSMGetTextAlign(QBViewRightOSM self);

/**
 * @}
 **/

#endif /* QBVIEWRIGHTOSM_H_ */
