/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBVIEWRIGHTTIRGGER_H_
#define QBVIEWRIGHTTIRGGER_H_

#include <stdint.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <mpeg_descriptors/dvb/cable_delivery.h>
#include <mpeg_descriptors/dvb/satellite_delivery.h>
#include <SvPlayerKit/SvBuf.h>
/**
 * Base class for triggers. Contains trigger Id.
 */
typedef struct QBViewRightGenericTrigger_s* QBViewRightGenericTrigger;
/**
 * Trigger listener interface. Should be implemented by object which want to receive triggers from QBViewRigh.
 */
typedef struct QBViewRightTriggerListener_i *QBViewRightTriggerListener;
/**
 * Generic trigger interface. Used to obtain trigger id number, and to notify listener.
 */
typedef struct QBViewRightTrigger_i* QBViewRightTrigger;
/**
 * @brief Watermark trigger
 * This trigger indicates that watermark should be shown.
 * It contains its duration and text which should be displayed.
 */
typedef struct QBViewRightWatermark_s *QBViewRightWatermark;
/**
 * @brief Pin reset trigger
 * This trigger indicates that Pin should be reset.
 * It contains
 */
typedef struct QBViewRightPin_s *QBViewRightPin;
/**
 * @brief Upgrade trigger
 * This trigger indicates that upgrade should be performed.
 * It contains information when it should be done.
 * It may contains delivery descriptor with location from where upgrade should be downloaded.
 */
typedef struct QBViewRightUpgrade_s* QBViewRightUpgrade;
/**
 * @brief Rescan trigger
 * This trigger indicates that rescan should be performed.
 * It contains information when it should be done, and what to do with old and new channels.
 * It may contains delivery descriptor with location from where rescan should be started
 */
typedef struct QBViewRightRescan_s* QBViewRightRescan;

/**
 * @brief Customer binding trigger
 * This trigger indicates that new binding date should be written in non-volatile memory.
 * It contains customerID, and bindig date.
 */
typedef struct QBViewRightCustomerBinding_s* QBViewRightCustomerBinding;


typedef enum QBViewRightUpgradeType_e {
    QBViewRightUpgradeType_Forced,             //!< Upgrade forced - Perform upgrade as soon as possible, ignore version
    QBViewRightUpgradeType_Now,                //!< Upgrade now - Perform upgrade as soon as possible
} QBViewRightUpgradeType;


typedef enum QBViewRightRescanType_e {
    QBViewRightRescanType_Now,                 //!< Starts rescan immediately
    QBViewRightRescanType_OnPowerUp,           //!< Rescan on power up
} QBViewRightRescanType;

typedef enum QBViewRightRescanMode_e {
    QBViewRightRescanMode_EraseAllAddFTAandCA, //!< Erase all add FTA and CA
    QBViewRightRescanMode_EraseAllAddCA,       //!< Erase all add CA
    QBViewRightRescanMode_AddFTAandCA,         //!< Add FTA andCA
    QBViewRightRescanMode_AddCA                //!< Add CA
} QBViewRightRescanMode;

typedef enum QBViewRightWatermarkType_e {
    QBViewRightWatermarkType_solid,                     //!< show watermark through whole duration
    QBViewRightWatermarkType_blinking,                  //!< show watermark in blinking pattern through requested duration
    QBViewRightWatermarkType_solidWithPositionAndColor, //!< show watermark with given position and color
} QBViewRightWatermarkType;


struct QBViewRightTriggerListener_i {
    /**
     * Method displays watermark, when application is in proper context.
     * If new watermark is requested, the actual is removed immediately.
     * Each new watermark is shown in random position.
     *
     * @param self_ listener instance
     * @param watermark watermark trigger
     */
    void (*watermark)(SvObject self_, QBViewRightWatermark watermark);
    /**
     * Immediately and silently change PC_PIN value into given one.
     * @param self_
     * @param pinReset pin reset trigger
     */
    void (*pinReset)(SvObject self_, QBViewRightPin pinReset);
    /**
     * Notify about upgrade. if it's 'upgrade now' trigger it is done immediately
     * @param self_ listener instance
     * @param upgrade upgrade trigger
     */
    void (*upgrade)(SvObject self_, QBViewRightUpgrade upgrade);
    /**
     * Starts the scanning. First there are checked frequencies given by rescan trigger.
     * @param self_ listener instance
     * @param rescan rescan trigger
     */
    void (*rescan)(SvObject self_, QBViewRightRescan rescan);
    /**
     * Handle customer binding trigger.
     * If customer id is invalid, or binding date is the past, decoding of the content is stopped.
     * @param self_ listener instance
     * @param customerBinding customer binding trigger
     */
    void (*customerBinding)(SvObject self_, QBViewRightCustomerBinding customerBinding);
};

struct QBViewRightTrigger_i {
    /**
     * Notify listener, about the trigger.
     * It invokes proper method in triggerListner interface
     * @param self_ trigger instance
     * @param listener listener instance
     */
    void (*notify)(SvObject self_, SvObject listener);
    /**
     * Return trigger's id number
     * @param self_ trigger instance
     * @return trigger id number
     */
    uint32_t (*getId)(SvObject self_);
};

SvInterface QBViewRightTrigger_getInterface(void);

/**
 * Creates trigger from data buffer delivered by SYS_SetDialogue.
 * @param buf trigger message
 * @param[out] errorOut returns error if occurred
 * @return trigger object if success, null if error occurred
 */
QBViewRightGenericTrigger QBViewRightTriggerCreate(SvBuf buf, SvErrorInfo *errorOut);

SvInterface QBViewRightTriggerListener_getInterface(void);
/**
 * Returns how long watermark should be displayed.
 * @param self watermark trigger instance
 * @return duration in seconds
 */
uint QBViewRightWatermarkGetDuration(QBViewRightWatermark self);

/**
 * Get type of watermark
 * @param self watermark trigger instance
 * @return watermark type
 */
QBViewRightWatermarkType QBViewRightWatermarkGetType(QBViewRightWatermark self);

/**
 * Get watermark visible frames count.
 * @param self watermark trigger instance
 * @return visible frames count
 */
uint32_t QBViewRightWatermarkGetVisibleFramesCount(QBViewRightWatermark self);

/**
 * Get watermark hidden frames count.
 * @param self watermark trigger instance
 * @return hidden frames count
 */
uint32_t QBViewRightWatermarkGetHiddenFramesCount(QBViewRightWatermark self);

/**
 * Return text which should be shown in watermark.
 * @param self watermark
 * @return text
 */
SvString QBViewRightWaremarkGetText(QBViewRightWatermark self);

/**
 * Maximum X position
 */
#define QBVIEW_RIGHT_WATERMARK_MAX_X_POSITION 0x450

/**
 * Get watermark x position.
 * @param self watermark trigger instance
 * @return x position
 */
uint16_t QBViewRightWatermarkGetXPosition(QBViewRightWatermark self);

/**
 * Maximum Y position
 */
#define QBVIEW_RIGHT_WATERMARK_MAX_Y_POSITION 0x400

/**
 * Get watermark y position.
 * @param self watermark trigger instance
 * @return y position
 */
uint16_t QBViewRightWatermarkGetYPosition(QBViewRightWatermark self);

/**
 * Get watermark font color index
 * @param self watermark trigger instance
 * @return font color index
 */
uint16_t QBViewRightWatermarkGetFontColorIndex(QBViewRightWatermark self);

/**
 * Get watermark background color index
 * @param self watermark trigger instance
 * @return background color index
 */
uint16_t QBViewRightWatermarkGetBackgroundColorIndex(QBViewRightWatermark self);

/**
 * Returns new PC_Pin value
 * @param self watermark trigger instance
 * @return pin value
 */
SvString QBViewRightPinGetString(QBViewRightPin self);

SvType QBViewRightUpgrade_getType(void);

/**
 * If it is Forced Upgrade
 */
bool QBViewRightUpgradeIsForced(QBViewRightUpgrade self);

/**
 * Method returns type of upgrade.
 * It defines when upgrade should be performed
 * @param self upgrade trigger instance
 * @return upgrade type
 */
QBViewRightUpgradeType QBViewRightUpgradeGetType(QBViewRightUpgrade self);
/**
 * Returns cable delivery descriptor. It can be more then one.
 * Those descriptors indicates from where upgrade should be downloaded.
 * @param self upgrade trigger instance
 * @return cable delivery descriptor
 */
mpeg_cable_delivery_desc * QBViewRightUpgradeGetCableDeliveryDescriptor(QBViewRightUpgrade self);
/**
 * Returns satellite delivery descriptor.
 * Those descriptor indicates from where upgrade should be downloaded.
 * @param self upgrade trigger instance
 * @return satellite delivery descriptor
 */
dvb_satellite_delivery_desc * QBViewRightUpgradeGetSatelliteDeliveryDescriptor(QBViewRightUpgrade self);

/**
 * This method gives information for what box type this upgrade is intended.
 * If stb's 'box type' is different upgrade should not be performed.
 *
 * The box type is defined by 8 hex-characters representing a 32 bit value,
 * where the first three characters define the set top box manufacturer
 * and the remaining 20 bit define the individual type for this manufacturer.
 *
 * If all bits are 0 of the manufacturer code, this means all set top boxes must react
 *
 * @param self upgrade trigger instance
 * @return box type as a SvString
 */
SvString QBViewRightUpgradeGetBoxType(QBViewRightUpgrade self);

SvType QBViewRightRescan_getType(void);
/**
 * Method returns type of rescan.
 * It defines when rescan should be performed
 * @param self rescan trigger instance
 * @return rescan type
 */
QBViewRightRescanType QBViewRightRescanGetType(QBViewRightRescan self);
/**
 * Method returns mode of rescan.
 * It defines define what should be done with channel after rescan.
 * @param self rescan trigger instance
 * @return rescan mode
 */

QBViewRightRescanMode QBViewRightRescanGetMode(QBViewRightRescan self);
/**
 * Returns cable delivery descriptor. It can be more then one.
 * Those descriptors indicates from where scan should be started.
 * @param self rescan trigger instance
 * @return cable delivery descriptor
 */
mpeg_cable_delivery_desc * QBViewRightRescanGetCableDeliveryDescriptor(QBViewRightRescan self);
/**
 * Returns customer id. It should be compared with the hard-code customer-id.
 * If the customer id is not equal, the box must immediately stop decoding any content,
 * independent whether it is scrambler or FTA content
 * @param self customer binding trigger
 * @return customer's id
 */
SvString QBViewRightCustomerBindingGetCustomerID(QBViewRightCustomerBinding self);
/**
 * Returns new customer binding date, whitch should be stored in non-volatile memory
 * If the stored date is less than the date
 * value, the box must immediately stop decoding any con-
 * tent, independent whether it is scrambler or FTA content
 *
 * @param self customer binding trigger
 * @return customer binding date
 */
SvTime QBViewRightCustomerBindingGetDate(QBViewRightCustomerBinding self);

#endif /* QBVIEWRIGHTTIRGGER_H_ */
