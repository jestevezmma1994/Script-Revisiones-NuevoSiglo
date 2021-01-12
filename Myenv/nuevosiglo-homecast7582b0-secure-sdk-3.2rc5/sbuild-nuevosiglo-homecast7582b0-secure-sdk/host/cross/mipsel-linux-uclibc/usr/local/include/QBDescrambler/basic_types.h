/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DESCRAMBLER_BASIC_TYPES_H
#define QB_DESCRAMBLER_BASIC_TYPES_H

/**
 * @file SMP/HAL/QBDescrambler/proto/basic_types.h
 * @brief Basic data types used by descrambler API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef MODULE
# include <linux/kernel.h>
#else
# include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBDescramblerTypes Data types used in descrambler API
 * @ingroup QBDescrambler
 * @{
 **/

/// Control-word protection mode (how control-word is scrambled.)
enum QBDescramblerCWProtectionMode_e {
    QBDescramblerCWProtectionMode_none, ///< CW is in clear
    QBDescramblerCWProtectionMode_cwpk, ///< CW is protected with given cwpk

    QBDescramblerCWProtectionMode_cnt_, ///< number of descrambler protection modes
};
typedef enum QBDescramblerCWProtectionMode_e QBDescramblerCWProtectionMode;

/// Scrambling mode (How content should be scrambled)
enum QBDescramblerScramblingMode_e {
    QBDescramblerScramblingMode_none, ///< Do not scramble content
    QBDescramblerScramblingMode_even, ///< Scramble content using even key
    QBDescramblerScramblingMode_odd,  ///< Scramble content using odd key
    QBDescramblerScramblingMode_provided_polarity,  ///< Scramble content using polarity provided by stream

    QBDescramblerScramblingMode_cnt_, ///< number of scrambling modes
};
typedef enum QBDescramblerScramblingMode_e QBDescramblerScramblingMode;

/// Algorithm of scrambling the content with given control-words.
enum QBDescramblerAlgo_e {
    QBDescramblerAlgo_dvbcsa,     ///< DVB CSA
    QBDescramblerAlgo_aes128_cbc, ///< AES CBC 128 bit
    QBDescramblerAlgo_aes128_ecb, ///< AES ECB 128 bit
    QBDescramblerAlgo_aes128_ecb_residual, ///< AES ECB 128 bit leading residual
    QBDescramblerAlgo_aes128_ecb_cp, ///< AES 128 bit ecb content protection
    QBDescramblerAlgo_3des_aba,   ///< 3DES
    QBDescramblerAlgo_conax_cp, ///< Conax content protection
    // TODO: more different types for termination block

    QBDescramblerAlgo_cnt_, ///< number of descrambler algorithms of scrambling
};
typedef enum QBDescramblerAlgo_e QBDescramblerAlgo;

/// Block termination scrambling mode (for AES).
enum QBDescramblerTermMode_e {
    QBDescramblerTermMode_clear = 0,  ///< leftover is left in clear
    QBDescramblerTermMode_cbc_atis,   ///< CBC-ATIS
};
typedef enum QBDescramblerTermMode_e QBDescramblerTermMode;

/// Control words, with all the info needed to use them.
struct QBDescramblerCW_s {
    uint8_t cwProtectionMode;   ///< @ref QBDescramblerCWProtectionMode
    uint8_t cwAlgo;             ///< @ref QBDescramblerAlgo

    struct {
        uint8_t data[16];       ///< control-word data
        uint8_t len;            ///< control-word length (can vary depending on algorithm)
    } odd,                 ///< odd control-word data
      even;                ///< even control-word data

    uint8_t cwpk[16];  ///< control-word protection key
};
typedef struct QBDescramblerCW_s QBDescramblerCW;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DESCRAMBLER_BASIC_TYPES_H
