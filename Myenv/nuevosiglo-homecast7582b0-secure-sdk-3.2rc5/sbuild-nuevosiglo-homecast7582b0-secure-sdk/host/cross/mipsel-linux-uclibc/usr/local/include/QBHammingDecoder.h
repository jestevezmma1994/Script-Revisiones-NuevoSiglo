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
****************************************************************************/
#ifndef QB_HAMMING_DECODER_H
#define QB_HAMMING_DECODER_H

/**
 * @file QBHammingDecoder.h
 * @brief Teletext manager class.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <stdint.h>

/**
 * Function decodes one byte encoded within hamming8/4 code.
 * Byte must have reversed bit order.
 * Function automatically reverses bits in byte.
 *
 * @param[in] hammByte encoded byte
 * @return    decoded byte with reversed bits.
 * If the 9th == 1 it means that an error occurred.
 **/
uint16_t QBHamming84(uint8_t hammByte);

/**
 * Function checks bit parity of given data buffer.
 * Byte must have reversed bit order.
 * Function automatically reverses bits in byte.
 *
 * @param[in/out] data  a data buffer
 * @param[in]     dataLen lenght of data buffer
 **/
void QBChkParity(uint8_t *data, int dataLen);

void QBDecodeParity(const uint8_t *in, int16_t *out, int dataLen);

/**
 * Function decodes magazine and line number from buffer.
 * Byte must have reversed bit order.
 * Function automatically reverses bits in byte.
 *
 * @param[in]  data     a data buffer
 * @param[out] mag      magazine number
 * @param[out] lineNum  line number
 * @param[out] error    If > 0 it means that an error occurred.
 **/
void QBHammingGetMagAndLineNum(const uint8_t *data, int *mag, int *lineNum, int *error);

uint32_t QBHamming2418(uint32_t x);

#endif
