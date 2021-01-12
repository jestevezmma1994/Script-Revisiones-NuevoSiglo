/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef CUIT_EFFECT_H_
#define CUIT_EFFECT_H_

/**
 * @file SMP/CUIT/effect.h CUIT effect
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <CUIT/Core/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SvGfxQueue_s;

/**
 * @defgroup SvEffect CUIT effect
 * @ingroup CUIT
 * @{
 **/

/**
 * Effect completion type.
 *
 * @see svAppCancelEffect()
 **/
typedef enum {
  /// effect finished completely
  SV_EFFECT_NORMAL,
  /// effect asked to revert widget to the original state
  SV_EFFECT_ROLLBACK,
  /// effect asked to stop changing widget state and leave it as is
  SV_EFFECT_ABANDON,
  /// effect asked to immediately bring widget to the final state
  SV_EFFECT_FINISH
} SvEffectCompletionType;


/**
 * CUIT effect step function type.
 *
 * This function is called once for each frame rendered by CUIT,
 * until effect is finished.
 *
 * @param[in] effect    CUIT effect handle
 * @param[in] time      time when next frame, containing the changes made
 *                      by the @a effect, will appear on screen
 * @return              @c true to finish the effect
 **/
typedef bool (*SvEffectStep)(SvEffect effect, double time);

/**
 * CUIT effect done function type.
 *
 * @param[in] e CUIT effect handle
 * @param[in] type completion type
 **/
typedef void (*SvEffectDone)(SvEffect e, SvEffectCompletionType type);

/**
 * CUIT effect pre serialize function type.
 *
 * This function is called before serialization of widget in order to
 * add to graphics queue operations to perform on widget
 * that contains this effect and its children.
 *
 * @param[in] effect    CUIT effect handle
 * @param[in] q         GFX queue handle
 * @param[in] offsetX   global horizontal position of widget
 * @param[in] offsetY   global vertical position of widget
 **/
typedef void (*SvEffectPreSerialize)(SvEffect effect, struct SvGfxQueue_s* q, int offsetX, int offsetY);

/**
 * CUIT effect post self serialize function type.
 *
 * This function is called after serialization of widget containing this effect
 * to revert GFX engine transformation stack to the state as before calling the
 * pre serialization function.
 *
 * @param[in] effect    CUIT effect handle
 * @param[in] q         GFX queue handle
 * @param[in] offsetX   global horizontal position of widget
 * @param[in] offsetY   global vertical position of widget
 **/
typedef void (*SvEffectPostSelfSerialize)(SvEffect effect, struct SvGfxQueue_s* q, int offsetX, int offsetY);

/**
 * CUIT effect post serialize function type.
 *
 * This function is called after serialization of widget containing this effect
 * and its children to revert GFX engine transformation stack to the state
 * as before calling the pre serialize function.
 *
 * @param[in] effect    CUIT effect handle
 * @param[in] q         GFX queue handle
 * @param[in] offsetX   global horizontal position of widget
 * @param[in] offsetY   global vertical position of widget
 **/
typedef void (*SvEffectPostSerialize)(SvEffect effect, struct SvGfxQueue_s* q, int offsetX, int offsetY);

/**
 * CUIT effect type.
 * @internal
 **/
struct SvEffect_t {
  SvEffectId id;     ///< effect id
  /// pointer to next effect in the global list of all effects in the application
  SvEffect prev;
  /// pointer to previous effect in the global list of all effects in the application
  SvEffect next;
  /// target widget for the effect
  SvWidget wgt;
  SvEffectId notificationTarget; ///< notification target of effect state


  double start;                         ///< start time
  double duration;                      ///< effect duration
  void *prv;                            ///< private structure (can be used for effect-specific data)
  SvEffectStep step;                    ///< step function
  SvEffectDone done;                    ///< done function
  SvEffectPreSerialize pre;             ///< pre serialize function
  SvEffectPostSelfSerialize postSelf;   ///< post self serialize function
  SvEffectPostSerialize post;           ///< post serialize function
  unsigned int cnt;                     ///< how many times step function was called
  bool isPermanent;                     ///< specifies if after animation completion we should destroy effect or only unregister it from app and keep it's effect
  bool finished;                        ///< specifies if permanent effect finshed all steps

  /// pointer to the next effect in the list of effects operating on the same widget
  SvEffect wgt_prev;
  /// pointer to the previous effect in the list of effects operating on the same widget
  SvEffect wgt_next;
};

/**
 * Create new effect.
 *
 * \param[in] wgt       target widget for the effect
 * \param[in] step      step function
 * \param[in] done      done function
 * \param[in] duration  duration of the effect
 * \return new effect handle or NULL in case of error
 **/
extern SvEffect
svEffectNew(SvWidget wgt, SvEffectStep step, SvEffectDone done, double duration);

/**
 * Create new extended effect.
 *
 * \param[in] wgt               target widget for the effect
 * \param[in] step              step function
 * \param[in] done              done function
 * \param[in] pre               pre serialize function
 * \param[in] postSelf          post self serialize function
 * \param[in] post              post serialize function
 * \param[in] duration          duration of the effect
 * \param[in] isPermanent       should we keep this effect in widget's effect list
 *            so that animations effect is preserved
 * \return new effect handle or NULL in case of error
 **/
extern SvEffect
svEffectExtendedNew(SvWidget wgt, SvEffectStep step, SvEffectDone done, SvEffectPreSerialize pre, SvEffectPostSelfSerialize postSelf, SvEffectPostSerialize post, double duration, bool isPermanent);

/**
 * Normally effects notify the widget that they are working on,
 * this function allows to overwrite this behaviour and
 * provide an another widget that will be notified about the event.
 * @param[in] e effect whose notification target will be set
 * @param[in] w widget wich will receive notifications
 **/
extern void
svEffectSetNotificationTarget(SvEffect e, SvWidget w);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
