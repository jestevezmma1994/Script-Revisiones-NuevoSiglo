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

#ifndef QB_TUNER_FUNCTOR_H
#define QB_TUNER_FUNCTOR_H

#include "QBTunerTypes.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/** Open tuner instance.
 *  \param id  tuner id, from config
 */
typedef QBTuner* (QBTunerOpenFunc) (int id);

/** Check if requested tuner could be opened.
 *  \param num  0 on success, nagative on error
 */
typedef int (QBTunerConfigFunc) (const struct QBTunerConfig** configs, int* config_cnt,
                                 const struct QBTunerConfigSet** sets, int* set_cnt);
/** Set master tuner.
 * \param tunerNumber number of tuner
 * \return 0 on success, -1 on fail
 */
typedef int (QBTunerSetMasterFunc) (int tunerNumber);

/** Get master tuner.
 * \return tuner number on success, -1 when no master tuner is set
 */
typedef int (QBTunerGetMasterFunc) (void);

struct QBTunerFunctor
{
  /** Close tuner instance.
   */
  void (*close) (QBTuner* tuner);

  /** Start tuning to given frequency.
   *  Result can be checked with \a QBTunerGetStatus().
   *  \returns 0 on sucess, SV_ERR_* on error.
   */
  int (*tune) (QBTuner* tuner, const struct QBTunerParams* params);

  /** Go into "not-tuning" status, and prepare for tuning to another frequency.
   *  Doesn't have to actually do anything internally,
   *  but has to return the "not-tuning" status in \a QBTunerIsTuned().
   */
  void (*untune) (QBTuner* tuner);

  /** Check if tuner is in "tuning" status.
   */
  bool (*is_tuned) (const QBTuner* tuner);

  /** Check lock status.
   *  \returns 0 on success, SV_ERR_* on error.
   */
  int (*get_status) (const QBTuner* tuner, struct QBTunerStatus* status);

  /** Get tuner properties.
   */
  void (*get_properties) (const QBTuner* tuner, struct QBTunerProperties* prop);

  /** set lnb master tuner.
   */
  void (*set_lnb_master) (const QBTuner* tuner, const bool is_lnb_master);
};

void QBTunerRegisterFunctor(QBTunerOpenFunc* open_func,
                            QBTunerConfigFunc* config_func,
                            QBTunerSetMasterFunc* set_master_func,
                            QBTunerGetMasterFunc* get_master_func,
                            const struct QBTunerFunctor* functor,
                            const char* name);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_TUNER_FUNCTOR_H
