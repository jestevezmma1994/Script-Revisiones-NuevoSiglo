/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_ECM_H_
#define SV_ECM_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ECM Section
 *  Represents raw, non-decrypted ecm
 */
typedef struct SvECMSection_s* SvECMSection;
struct SvECMSection_s {
  struct SvObject_ super_;

  int pid;

  int len;
  unsigned char data[256]; // max section len is 253
  unsigned char packet[188];
};

extern SvType SvECMSection_getType(void);

/**
 * Allocate new ecm section object.
 */
static inline SvECMSection SvECMSectionCreate(void)
{
  SvECMSection ecmsec = (SvECMSection) SvTypeAllocateInstance(SvECMSection_getType(), NULL);
  return ecmsec;
}

//-----------------------------------------

/** Supported encryption algorithms.
 */
enum SvECMAlgo_e {
  SvECMAlgo_null,
  SvECMAlgo_RC4,
  SvECMAlgo_AESECBT,
  SvECMAlgo_AESECBL,
  SvECMAlgo_DVBCSA,
};
typedef enum SvECMAlgo_e  SvECMAlgo;

/** Get name of given encryption algorithm.
 */
const char* SvECMAlgoToString(SvECMAlgo algo);

//-----------------------------------------

/** ECM Control Word
 *  Represents preprocessed, decrypted ecm.
 */
typedef struct SvECMControlWord_s* SvECMControlWord;
struct SvECMControlWord_s {
  struct SvObject_ super_;

  SvECMAlgo algo;

  int version;
  bool isEven;
  unsigned char key[32];
  unsigned char iv[32];
};

extern SvType SvECMControlWord_getType(void);

/**
 * Allocate new ecm section object.
 */
static inline SvECMControlWord SvECMControlWordCreate(void)
{
  SvECMControlWord ecmcw = (SvECMControlWord) SvTypeAllocateInstance(SvECMControlWord_getType(), NULL);
  return ecmcw;
};

/** Parse ecm section and look for ecm control word.
 *  For testing only.
 */
SvECMControlWord  SvECMControlWordCreateFromSection(SvECMSection ecmsec);

/** Sprintf \a cw into \a buf, using not more than \a maxbytes
 *  For debugging only/
 */
int SvECMControlWordSprintf(SvECMControlWord cw, char* buf, int maxbytes);

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_ECM_H
