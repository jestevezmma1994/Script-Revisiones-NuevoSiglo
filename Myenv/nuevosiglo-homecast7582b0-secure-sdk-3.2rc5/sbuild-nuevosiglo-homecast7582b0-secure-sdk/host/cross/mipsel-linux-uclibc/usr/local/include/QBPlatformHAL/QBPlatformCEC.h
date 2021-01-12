/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PLATFORM_CEC_H_
#define QB_PLATFORM_CEC_H_

/**
 * @file QBPlatformCEC.h HDMI CEC interface control API
 **/

#include <stdint.h>
#include <stdbool.h>
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvCoreTypes.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformCEC HDMI CEC interface control
 * @ingroup QBPlatformHAL
 * @{
 **/


/**
 * CEC device type addresses.
 **/
typedef enum {
    QBPlatformCECDeviceAddress__first__         = 0,
    QBPlatformCECDeviceAddress_TV               = 0,
    QBPlatformCECDeviceAddress_RecordingDevice1 = 1,
    QBPlatformCECDeviceAddress_RecordingDevice2 = 2,
    QBPlatformCECDeviceAddress_Tuner1           = 3,
    QBPlatformCECDeviceAddress_PlaybackDevice1  = 4,
    QBPlatformCECDeviceAddress_AudioSystem      = 5,
    QBPlatformCECDeviceAddress_Tuner2           = 6,
    QBPlatformCECDeviceAddress_Tuner3           = 7,
    QBPlatformCECDeviceAddress_PlaybackDevice2  = 8,
    QBPlatformCECDeviceAddress_RecordingDevice3 = 9,
    QBPlatformCECDeviceAddress_Tuner4           = 10,
    QBPlatformCECDeviceAddress_PlaybackDevice3  = 11,
    QBPlatformCECDeviceAddress_ReservedC        = 12,
    QBPlatformCECDeviceAddress_ReservedD        = 13,
    QBPlatformCECDeviceAddress_ReservedE        = 14,
    QBPlatformCECDeviceAddress_Broadcast        = 15,
    QBPlatformCECDeviceAddress__last__          = 15
} QBPlatformCECDeviceAddress;

/**
 * CEC opcodes.
 **/
typedef enum {
    QBPlatformCECOpcode__first__                  = 0x00,
    QBPlatformCECOpcode_FeatureAbort              = 0x00,
    /* Reserved 0x01-0x03 */
    QBPlatformCECOpcode_ImageViewOn               = 0x04,
    QBPlatformCECOpcode_TunerStepIncrement        = 0x05,
    QBPlatformCECOpcode_TunerStepDecrement        = 0x06,
    QBPlatformCECOpcode_TunerDeviceStatus         = 0x07,
    QBPlatformCECOpcode_GiveTunerDeviceStatus     = 0x08,
    QBPlatformCECOpcode_RecordOn                  = 0x09,
    QBPlatformCECOpcode_RecordStatus              = 0x0A,
    QBPlatformCECOpcode_RecordOff                 = 0x0B,
    /* Reserved 0x0C */
    QBPlatformCECOpcode_TextViewOn                = 0x0D,
    /* Reserved 0x0E */
    QBPlatformCECOpcode_RecordTVScreen            = 0x0F,
    /* Reserved 0x10-0x19 */
    QBPlatformCECOpcode_GiveDeckStatus            = 0x1A,
    QBPlatformCECOpcode_DeckStatus                = 0x1B,
    /* Reserved 0x1C-0x31 */
    QBPlatformCECOpcode_SetMenuLanguage           = 0x32,
    QBPlatformCECOpcode_ClearAnalogueTimer        = 0x33,
    QBPlatformCECOpcode_SetAnalogueTimer          = 0x34,
    QBPlatformCECOpcode_TimerStatus               = 0x35,
    QBPlatformCECOpcode_Standby                   = 0x36,
    /* Reserved 0x37-0x40 */
    QBPlatformCECOpcode_Play                      = 0x41,
    QBPlatformCECOpcode_DeckControl               = 0x42,
    QBPlatformCECOpcode_TimerClearedStatus        = 0x43,
    QBPlatformCECOpcode_UserControlPressed        = 0x44,
    QBPlatformCECOpcode_UserControlReleased       = 0x45,
    QBPlatformCECOpcode_GiveOSDName               = 0x46,
    QBPlatformCECOpcode_SetOSDName                = 0x47,
    /* Reserved 0x48-0x63 */
    QBPlatformCECOpcode_SetOSDString              = 0x64,
    /* Reserved 0x65-0x66 */
    QBPlatformCECOpcode_SetTimerProgramTitle      = 0x67,
    /* Reserved 0x68-0x69 */
    QBPlatformCECOpcode_SystemAudioModeRequest    = 0x70,
    QBPlatformCECOpcode_GiveAudioStatus           = 0x71,
    QBPlatformCECOpcode_SetSystemAudioMode        = 0x72,
    /* Reserved 0x73-0x79 */
    QBPlatformCECOpcode_ReportAudioStatus         = 0x7A,
    /* Reserved 0x7B-0x7C */
    QBPlatformCECOpcode_GiveSystemAudioModeStatus = 0x7D,
    QBPlatformCECOpcode_SystemAudioModeStatus     = 0x7E,
    /* Reserved 0x7F */
    QBPlatformCECOpcode_RoutingChange             = 0x80,
    QBPlatformCECOpcode_RoutingInformation        = 0x81,
    QBPlatformCECOpcode_ActiveSource              = 0x82,
    QBPlatformCECOpcode_GivePhysicalAddress       = 0x83,
    QBPlatformCECOpcode_ReportPhysicalAddress     = 0x84,
    QBPlatformCECOpcode_RequestActiveSource       = 0x85,
    QBPlatformCECOpcode_SetStreamPath             = 0x86,
    QBPlatformCECOpcode_DeviceVendorID            = 0x87,
    QBPlatformCECOpcode_VendorCommand             = 0x89,
    QBPlatformCECOpcode_VendorRemoteButtonDown    = 0x8A,
    QBPlatformCECOpcode_VendorRemoteButtonUp      = 0x8B,
    QBPlatformCECOpcode_GiveDeviceVendorID        = 0x8C,
    QBPlatformCECOpcode_MenuRequest               = 0x8D,
    QBPlatformCECOpcode_MenuStatus                = 0x8E,
    QBPlatformCECOpcode_GiveDevicePowerStatus     = 0x8F,
    QBPlatformCECOpcode_ReportPowerStatus         = 0x90,
    QBPlatformCECOpcode_GetMenuLanguage           = 0x91,
    QBPlatformCECOpcode_SelectAnalogueService     = 0x92,
    QBPlatformCECOpcode_SelectDigitalService      = 0x93,
    /* Reserved 0x94-0x96 */
    QBPlatformCECOpcode_SetDigitalTimer           = 0x97,
    /* Reserved 0x98 */
    QBPlatformCECOpcode_ClearDigitalTimer         = 0x99,
    QBPlatformCECOpcode_SetAudioRate              = 0x9A,
    /* Reserved 0x9B-0x9C */
    QBPlatformCECOpcode_InactiveSource            = 0x9D,
    QBPlatformCECOpcode_CECVersion                = 0x9E,
    QBPlatformCECOpcode_GetCECVersion             = 0x9F,
    QBPlatformCECOpcode_VendorCommandWithID       = 0xA0,
    QBPlatformCECOpcode_ClearExternalTimer        = 0xA1,
    QBPlatformCECOpcode_SetExternalTimer          = 0xA2,
    /* Reserved 0xA3-0xFE */
    QBPlatformCECOpcode_Abort                     = 0xFF,
    QBPlatformCECOpcode__last__                   = 0xFF
} QBPlatformCECOpcode;

/**
 * CEC User Input Command (UI Command) codes.
 **/
typedef enum {
    QBPlatformCECUICommand__first__                 = 0x00,
    QBPlatformCECUICommand_Select                   = 0x00,
    QBPlatformCECUICommand_Up                       = 0x01,
    QBPlatformCECUICommand_Down                     = 0x02,
    QBPlatformCECUICommand_Left                     = 0x03,
    QBPlatformCECUICommand_Right                    = 0x04,
    QBPlatformCECUICommand_RightUp                  = 0x05,
    QBPlatformCECUICommand_RightDown                = 0x06,
    QBPlatformCECUICommand_LeftUp                   = 0x07,
    QBPlatformCECUICommand_LeftDown                 = 0x08,
    QBPlatformCECUICommand_RootMenu                 = 0x09,
    QBPlatformCECUICommand_SetupMenu                = 0x0a,
    QBPlatformCECUICommand_ContentsMenu             = 0x0b,
    QBPlatformCECUICommand_FavoriteMenu             = 0x0c,
    QBPlatformCECUICommand_Exit                     = 0x0d,
    /* Reserved 0x0E-0x1F */
    QBPlatformCECUICommand_Numbers0                 = 0x20,
    QBPlatformCECUICommand_Numbers1                 = 0x21,
    QBPlatformCECUICommand_Numbers2                 = 0x22,
    QBPlatformCECUICommand_Numbers3                 = 0x23,
    QBPlatformCECUICommand_Numbers4                 = 0x24,
    QBPlatformCECUICommand_Numbers5                 = 0x25,
    QBPlatformCECUICommand_Numbers6                 = 0x26,
    QBPlatformCECUICommand_Numbers7                 = 0x27,
    QBPlatformCECUICommand_Numbers8                 = 0x28,
    QBPlatformCECUICommand_Numbers9                 = 0x29,
    QBPlatformCECUICommand_Dot                      = 0x2a,
    QBPlatformCECUICommand_Enter                    = 0x2b,
    QBPlatformCECUICommand_Clear                    = 0x2c,
    /* Reserved 0x2d-0x2e */
    QBPlatformCECUICommand_NextFavorite             = 0x2f,
    QBPlatformCECUICommand_ChannelUp                = 0x30,
    QBPlatformCECUICommand_ChannelDown              = 0x31,
    QBPlatformCECUICommand_PreviousChannel          = 0x32,
    QBPlatformCECUICommand_SoundSelect              = 0x33,
    QBPlatformCECUICommand_InputSelect              = 0x34,
    QBPlatformCECUICommand_DisplayInformation       = 0x35,
    QBPlatformCECUICommand_Help                     = 0x36,
    QBPlatformCECUICommand_PageUp                   = 0x37,
    QBPlatformCECUICommand_PageDown                 = 0x38,
    /* Reserved 0x39-0x3F */
    QBPlatformCECUICommand_Power                    = 0x40,
    QBPlatformCECUICommand_VolumeUp                 = 0x41,
    QBPlatformCECUICommand_VolumeDown               = 0x42,
    QBPlatformCECUICommand_Mute                     = 0x43,
    QBPlatformCECUICommand_Play                     = 0x44,
    QBPlatformCECUICommand_Stop                     = 0x45,
    QBPlatformCECUICommand_Pause                    = 0x46,
    QBPlatformCECUICommand_Record                   = 0x47,
    QBPlatformCECUICommand_Rewind                   = 0x48,
    QBPlatformCECUICommand_FastForward              = 0x49,
    QBPlatformCECUICommand_Eject                    = 0x4a,
    QBPlatformCECUICommand_Forward                  = 0x4b,
    QBPlatformCECUICommand_Backward                 = 0x4c,
    QBPlatformCECUICommand_StopRecord               = 0x4d,
    QBPlatformCECUICommand_PauseRecord              = 0x4e,
    /* Reserved 0x4f */
    QBPlatformCECUICommand_Angle                    = 0x50,
    QBPlatformCECUICommand_SubPicture               = 0x51,
    QBPlatformCECUICommand_VideoOnDemand            = 0x52,
    QBPlatformCECUICommand_ElectronicProgramGuide   = 0x53,
    QBPlatformCECUICommand_TimerProgramming         = 0x54,
    QBPlatformCECUICommand_InitialConfiguration     = 0x55,
    /* Reserved 0x56-0x5f */
    QBPlatformCECUICommand_PlayFunction             = 0x60,
    QBPlatformCECUICommand_PausePlayFunction        = 0x61,
    QBPlatformCECUICommand_RecordFunction           = 0x62,
    QBPlatformCECUICommand_PauseRecordFunction      = 0x63,
    QBPlatformCECUICommand_StopFunction             = 0x64,
    QBPlatformCECUICommand_MuteFunction             = 0x65,
    QBPlatformCECUICommand_RestoreVolumeFunction    = 0x66,
    QBPlatformCECUICommand_TuneFunction             = 0x67,
    QBPlatformCECUICommand_SelectMediaFunction      = 0x68,
    QBPlatformCECUICommand_SelectAVInputFunction    = 0x69,
    QBPlatformCECUICommand_SelectAudioInputFunction = 0x6a,
    QBPlatformCECUICommand_PowerToggleFunction      = 0x6b,
    QBPlatformCECUICommand_PowerOffFunction         = 0x6c,
    QBPlatformCECUICommand_PowerOnFunction          = 0x6d,
    /* Reserved 0x6e-0x70 */
    QBPlatformCECUICommand_F1 /* Blue */            = 0x71,
    QBPlatformCECUICommand_F2 /* Red */             = 0x72,
    QBPlatformCECUICommand_F3 /* Green */           = 0x73,
    QBPlatformCECUICommand_F4 /* Yellow */          = 0x74,
    QBPlatformCECUICommand_F5                       = 0x75,
    QBPlatformCECUICommand_Data                     = 0x76,
    /* Reserved 0x77-0xff */
    QBPlatformCECUICommand__last__                  = 0xFF
} QBPlatformCECUICommand;

/**
 * CEC Version codes.
 **/
typedef enum {
    QBPlatformCECCECVersion__first__        = 0x00,
    QBPlatformCECCECVersion_Version1_1      = 0x00,
    QBPlatformCECCECVersion_Version1_2      = 0x01,
    QBPlatformCECCECVersion_Version1_2a     = 0x02,
    QBPlatformCECCECVersion_Version1_3      = 0x03,
    QBPlatformCECCECVersion_Version1_3a     = 0x04,
    QBPlatformCECCECVersion__last__         = 0x04
} QBPlatformCECCECVersion;

/**
 * CEC Power Status
 **/
typedef enum {
    QBPlatformCECPowerStatus__first__                 = 0x00,
    QBPlatformCECPowerStatus_On                       = 0x00,
    QBPlatformCECPowerStatus_Standby                  = 0x01,
    QBPlatformCECPowerStatus_InTransitionStandbyToOn  = 0x02,
    QBPlatformCECPowerStatus_InTransitionOnToStandby  = 0x03,
    QBPlatformCECPowerStatus__last__                  = 0x03
} QBPlatformCECPowerStatus;

/**
 * CEC Device Type
 **/
typedef enum {
    QBPlatformCECDeviceType__first__                 = 0x00,
    QBPlatformCECDeviceType_TV                       = 0x00,
    QBPlatformCECDeviceType_RecordingDevice          = 0x01,
    QBPlatformCECDeviceType_Reserved                 = 0x02,
    QBPlatformCECDeviceType_Tuner                    = 0x03,
    QBPlatformCECDeviceType_PlaybackDevice           = 0x04,
    QBPlatformCECDeviceType_AudioSystem              = 0x05,
    QBPlatformCECDeviceType__last__                  = 0x05
} QBPlatformCECDeviceType;

/**
 * CEC Abort Reason
 **/
typedef enum {
    QBPlatformCECAbortReason__first__                   = 0x00,
    QBPlatformCECAbortReason_UnrecognizedOpcode         = 0x00,
    QBPlatformCECAbortReason_NotInCorrectModeToRespond  = 0x01,
    QBPlatformCECAbortReason_CannotProvideSource        = 0x02,
    QBPlatformCECAbortReason_InvalidOperand             = 0x03,
    QBPlatformCECAbortReason_Refused                    = 0x04,
    QBPlatformCECAbortReason__last__                    = 0x04
} QBPlatformCECAbortReason;

/**
 * CEC device status.
 **/
typedef struct QBCECStatus_ {
    bool enabled;
    //FIXME: There can be many logical addresses in case of many supported features
    uint8_t logicalAddr;
    uint16_t physicalAddr;

} *QBCECStatus;

/**
 * CEC message type.
 **/
typedef struct QBCECMessage_ {
    uint8_t src;
    uint8_t dest;
    unsigned short len;
    uint8_t msg[16];
} *QBCECMessage;

/**
 * CEC OSD (On-Screen-Display) device name, the shortest ASCII (not NULL terminated) text within range 0x20-0x7E
 **/
#define QBPlatformCECOSDName__shortest__ 1
/**
 * CEC OSD (On-Screen-Display) device name, the longest ASCII (not NULL terminated) text within range 0x20-0x7E
 **/
#define QBPlatformCECOSDName__longest__  14

/**
 * CEC OSD (On-Screen-Display) on-screen text, the shortest ASCII (not NULL terminated) text within range 0x20-0x7E
 **/
#define QBPlatformCECOSDString__shortest__ 1
/**
 * CEC OSD (On-Screen-Display) on-screen text, the longest ASCII (not NULL terminated) text within range 0x20-0x7E
 **/
#define QBPlatformCECOSDString__longest__  13

/**
 * CEC The minimal valid VendorID value
 **/
#define QBPlatformCECVendorID__minimal__  0
/**
 * CEC The maximal valid VendorID value
 **/
#define QBPlatformCECVendorID__maximal__  0xFFFFFF

/**
 * CEC The minimal valid physical address
 **/
#define QBPlatformCECPhysicalAddress__minimal__  0x0000
/**
 * CEC The maximal valid VendorID value
 **/
#define QBPlatformCECPhysicalAddress__maximal__  0xFFFF

/**
 * Start the QBPlatformCEC fiber.
 *
 * @return              @c 0 on success, negative value in case of error
 **/
extern int QBPlatformCECStart(void);

/**
 * Stop the QBPlatformCEC fiber.
 *
 * @return              @c 0 on success, negative value in case of error
 **/
extern int QBPlatformCECStop(void);

/**
 * Send CEC Active Source message over HDMI port.
 *
 * Message will contain caller physical address
 * as an active source address.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @return              @c 0 on success, negative value in case of error
 **/
extern int
QBPlatformCECSendActiveSource(unsigned int outputID);

/**
 * Send CEC System Standby message over HDMI port.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @return              @c 0 on success, negative value in case of error
 **/
extern int
QBPlatformCECSendSystemStandby(unsigned int outputID);

/**
 * Send CEC One Touch Play messages over HDMI port.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @return              @c 0 on success, negative value in case of error
 **/
extern int
QBPlatformCECSendOneTouchPlay(unsigned int outputID);

/**
 * @class QBPlatformCECAction
 */
typedef struct QBPlatformCECAction_ *QBPlatformCECAction;
/**
 * Get runtime type specification of QBPlatformCECAction type
 *
 * @return QBPlatformCECAction runtime type specification
 */
SvType QBPlatformCECAction_getType(void);

/**
 * @class QBPlatformCECAction
 *
 * extends QBPlatformCECAction
 */
typedef struct QBPlatformCECActionMessageReceived_ *QBPlatformCECActionMessageReceived;
/**
 * Get runtime type specification of QBPlatformCECActionMessageRecevied type
 *
 * @return QBPlatformCECActionMessageRecevied runtime type specification
 */
SvType QBPlatformCECActionMessageReceived_getType(void);
/**
 * Create new message received CEC action
 *
 * @param[in] msg received CEC message
 * @return created message received CEC action object
 *
 */
QBPlatformCECActionMessageReceived QBPlatformCECActionMessageReceivedCreate(QBCECMessage msg);
/**
 * Get message bytes from message received CEC action
 *
 * @param[in] self message received CEC action handle
 * @return message bytes
 */
QBCECMessage QBPlatformCECActionMessageReceivedGetMessage(QBPlatformCECActionMessageReceived self);

/**
 * @class QBPlatformCECActionReady
 *
 * extends QBPlatformCECAction
 */
typedef struct QBPlatformCECActionReady_ *QBPlatformCECActionReady;
/**
 * Get runtime type specification of QBPlatformCECActionReady type
 *
 * @return QBPlatformCECActionReady runtime type specification
 */
SvType QBPlatformCECActionReady_getType(void);
/**
 * Create new ready CEC action
 *
 * @return created message ready CEC action
 */
QBPlatformCECActionReady QBPlatformCECActionReadyCreate(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
