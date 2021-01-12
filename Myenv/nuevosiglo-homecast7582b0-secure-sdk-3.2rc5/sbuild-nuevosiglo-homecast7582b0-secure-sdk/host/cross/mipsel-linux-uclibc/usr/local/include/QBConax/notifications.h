/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CONAX_NOTIFICATIONS_H
#define QB_CONAX_NOTIFICATIONS_H

/**
 * @file QBConax/notifications.h
 * @brief Notifications returned from the module.
 **/

/**
 * @ingroup QBConax
 * @{
 */

/// Type of notification to return with @ref QBConaxGetNotification
enum QBConaxNotification
{
  QBConaxNotification_none = 0,               ///< no notification available

  QBConaxNotification_card_state_removed,         ///< the card has been removed
  QBConaxNotification_card_state_inserted,        ///< a card has been inserted
  QBConaxNotification_card_state_correct,         ///< card is detected as "correct", data : @ref QBConaxNotificationCorrectCard
  QBConaxNotification_card_state_incorrect,       ///< card is detected as incorrect (wrong interface)
  QBConaxNotification_card_state_incorrect_conax, ///< incorrect conax card detected
  QBConaxNotification_card_state_problem,         ///< card communication problem

  QBConaxNotification_ecm_finished,           ///< all the return parameters from processed ecm have been notified
  QBConaxNotification_ecm_invalid,            ///< the ecm being process a.t.m. caused the smartcard reset. it is not a no-access, but the ecm should not be pushed ever again

  QBConaxNotification_control_word,           ///< data : @ref QBDescramblerCW
  QBConaxNotification_no_access,              ///< data : uint32_t - no access code
  QBConaxNotification_maturity_rating,        ///< data : @ref QBConaxNotificationMaturityRating
  QBConaxNotification_token_debit,            ///< data : @ref QBConaxNotificationTokenDebit
  QBConaxNotification_accept_viewing,         ///< data : @ref QBConaxNotificationAcceptViewing
  QBConaxNotification_order,                  ///< data : @ref QBConaxNotificationOrder
  QBConaxNotification_no_access_to_network,   ///< see conax specs
  QBConaxNotification_geographical_blackout,  ///< see conax specs

  QBConaxNotification_user_text,              ///< new host-data is stored in the nvram : uint32_t - slot index
  QBConaxNotification_fingerprint,            ///< new fingerprint is stored in the nvram

  QBConaxNotification_card_reset,             ///< the card was reset, and is now "correct_card". cat should be refreshed. @deprecated use correct_card instead

  QBConaxNotification_pairing_changed,        ///< data: @ref QBConaxNotificationPairingChanged, should be used for debugging only
  QBConaxNotification_custom_data,            ///< data: @ref QBConaxCustomData

  QBConaxNotification_apdu_data,              ///< data: @ref QBConaxNotificationAPDUData
  QBConaxNotification_encrypt,                ///< data: @ref QBConaxEncrypt
  QBConaxNotification_uri_data,               ///< data: @ref QBConaxNotificationUriData

  // convenient aliases for checking if card's state has changed
  QBConaxNotification_card_state_first = QBConaxNotification_card_state_removed,
  QBConaxNotification_card_state_last  = QBConaxNotification_card_state_problem,
};

/**
 * @}
 */

#endif // #ifndef QB_CONAX_NOTIFICATIONS_H
