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

#ifndef CUIT_APPLICATION_H_
#define CUIT_APPLICATION_H_

/**
 * @file SMP/CUIT/app.h CUIT application
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvAutoreleasePool.h>
#include <SvFoundation/SvWeakList.h>
#include <CAGE/Core/SvColor.h>
#include <SvGfxHAL/dsplink_engine.h>
#include <QBInput/QBInputEvent.h>
#include <QBInput/QBInputQueue.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/effect.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvApplication CUIT application
 * @ingroup CUIT
 * @{
 **/


/**
 * CUIT application class.
 * @internal
 **/
struct SvApplication_t {
  unsigned int width;          ///< width of application window (INT)
  unsigned int height;         ///< height of application window (INT)
  SvWindow wnd_first;          ///< first element of active windows list (INT)
  SvWindow wnd_last;           ///< last element of active windows list (INT)
  SvEffect eff_first;          ///< first element of running effects list (INT)
  SvEffect eff_last;           ///< last element of running effects list (INT)
  SvWindow wnd_focus;          ///< focused window
  struct SvEventQueue_ *eq;    ///< events queue (INT)
  struct SvEventQueue_ *ueq;   ///< user events queue (INT)
  struct SvTimerQueue_ *tq;    ///< timer queue (INT)
  struct SvHash_ *widgets;     ///< widgets lookup table (INT)
  double timer;                ///< timer (INT)
  double frame_time;

  int render_frames_cnt;       ///< still needs to render this amount of frames, even if scene is unchanged
  int full_redraw_frames_cnt;  ///< this amount of frames won't use partial updates
  int render_frames_min;       ///< when the scene is changed, we need to render for this amount of frames to reach the display
  uint64_t rendered_frames;    ///< counter of rendered frames

  unsigned int widgets_cnt;    ///< counter of all widgets that are already in memory

  int in_events_us;            ///< time spent in event handlers since last frame
  int render_time_us;          ///< expected rendering time of current frame (averaged from several last frames).

  bool partial_update_enabled; ///< global setting
  void *partial_update_trace;  ///< state kept between frames to compute partial updates

  bool input_service_initialized;
  /** @internal source of input events */
  QBInputQueue input_queue;

  /** @internal fiber used by CUIT */
  SvScheduler scheduler;
  SvFiber mainFiber;
  SvFiber userEventFiber;
  SvFiberTimer mainFiberTimer;

  /** @internal CUIT main fiber state */
  enum {
    s_input, s_systemEvents, s_render
  } state;

  SvAutoreleasePool pool;

  unsigned int frame_rate;

  SvTime next_frame;  // at this time next frame should be rendered

  bool gfx_initialized;
  bool outputs_initialized;

  SvWeakList render_listeners;

  struct {
      SvWidget *new_focused_parents;    ///< parents of widget which will get focus,
                                        ///< able to handle @ref SvFocusEventKind_IN or @ref SvFocusEventKind_OUT events
      size_t new_focused_parents_cnt;   ///< current count of widgets inside new_focused_parents array

      SvWidget *old_focused_parents;    ///< parents of widget which will lose focus,
                                        ///< able to handle @ref SvFocusEventKind_IN or @ref SvFocusEventKind_OUT events
      size_t old_focused_parents_cnt;   ///< current count of widgets inside new_focused_parents array

      size_t max_parents_array_size;    ///< size of new_focused_parents and old_focused_parents arrays

  } focus_monitor; ///< helper for focus monitor feature (see @ref svWidgetSetFocusMonitor)
                   ///< @warning Do not access this field directly! It's used internally to create SvFocusEvent instance.
};


/**
 * Create CUIT application.
 *
 * @param[in] width       width of the application's drawing area in pixels
 * @param[in] height      height of the application's drawing area in pixels
 * @param[in] frameRate   application frame rate
 * @param[out] errorOut   error info
 * @return                new application handle, @c NULL in case of error
 **/
extern SvApplication
svAppCreate(unsigned int width, unsigned int height, unsigned int frameRate,
            SvErrorInfo *errorOut);

/**
 * Initialize the application.
 *
 * This function creates the CUIT application instance.
 * Because there is no way to express complete video outputs configuration
 * using this function's parameters, it should be used only in simpliest
 * cases like test applications; in full-featured GUI projects with complex
 * configuration of video outputs use QBPlatformHAL API to configure outputs
 * and svAppCreate() to create CUIT application.
 *
 * \param[in] w           width of application window
 * \param[in] h           height of application window
 * \param[in] background  color used for background (unused)
 * \param[in] gui_ar      application window aspect ratio (unused)
 * \param[in] sd_mode     video mode of SD output
 * \param[in] sd_ar       aspect ratio of SD output
 * \param[in] hd_mode     video mode of HD output
 * \param[in] hd_ar       aspect ratio of HD output (ignored, always 16:9)
 * \return                new application handle, @c NULL in case of error
 **/
extern SvApplication
svAppInitExt(unsigned int w, unsigned int h,
             SvColor background,
             SvAspectRatioType gui_ar,
             SvOutputStandardType sd_mode,
             SvAspectRatioType sd_ar,
             SvOutputStandardType hd_mode,
             SvAspectRatioType hd_ar);

/**
 * Initialize the application.
 *
 * This function creates the CUIT application instance.
 * It's wrapper for svAppInitExt().
 *
 * \param[in] w           width of application window
 * \param[in] h           height of application window
 * \param[in] background  color used for background (unused)
 * \return                new application handle, @c NULL in case of error
 **/
extern SvApplication
svAppInit(unsigned int w, unsigned int h, SvColor background);

/**
 * Destroy CUIT application.
 *
 * This method frees all resources used by the application.
 * If application have been created using svAppInit() or svAppInitExt()
 * and initialized graphics engine by itself, this method will also
 * deinitialize graphics engine.
 *
 * \param[in] app    application handle (will be invalid after this call)
 **/
extern void
svAppDestroy(SvApplication app);

/**
 * Connect CUIT application's input to one of QBInput's event queues.
 *
 * This method sets up given @a inputQueue as a source of input
 * events for the CUIT application. By default, if this method has
 * not been called, svAppSetupMainLoop() will bind application
 * to the default output queue of the @ref QBInputService.
 *
 * \param[in] app   application handle
 * \param[in] inputQueue input events queue
 **/
extern void
svAppSetupInput(SvApplication app, QBInputQueue inputQueue);

/**
 * Setup CUIT application fiber before entering main loop.
 *
 * This method prepares CUIT application for entering main loop
 * and registers application's fiber in the @a scheduler. After
 * svAppSetupMainLoop() is called, application can be run either
 * by directly calling SvSchedulerLoop() or by svAppStart().
 *
 * \param[in] app  application handle
 * \param[in] scheduler  scheduler handle, @c NULL to use default
 */
extern void
svAppSetupMainLoop(SvApplication app, SvScheduler scheduler);

/**
 * Start the main CUIT application loop.
 *
 * This method calls svAppSetupMainLoop() (if necessary) and
 * starts main application loop. In main loop application delivers all events
 * (input events, user events and others) and redraws the screen.
 *
 * \note This method does not return until the application ends.
 *
 * \param[in] app  application handle
 **/
extern void
svAppStart(SvApplication app);

/**
 * Stop the main application loop.
 *
 * This method breaks main application loop. Before that,
 * application will deliver as many queued user events as possible.
 *
 * \param[in] app application handle
 */
extern void
svAppStop(SvApplication app);

/**
 * Set screen refresh ratio.
 *
 * @param[in] app application handle
 * @param[in] frameRate number of frames per second to render
 **/
extern void
svAppSetFrameRate(SvApplication app, unsigned int frameRate);

/**
 * Get size of the application's drawing area.
 *
 * @param[in] app application handle
 * @param[out] width width of the drawing area in pixels
 * @param[out] height height of the drawing area in pixels
 * @return @c 0 in case of error, any other value otherwise
 **/
extern int
svAppGetDimensions(SvApplication app, unsigned int *width, unsigned int *height);

/**
 * Get handle to a scheduler driving application's main loop.
 *
 * @param[in] app application handle
 * @return scheduler
 **/
extern SvScheduler
svAppGetScheduler(SvApplication app);


/**
 * @defgroup SvApplicationWindows Working with windows
 * @ingroup SvApplication
 * @{
 **/

/**
 * Add a window to the display stack.
 *
 * Window will be inserted before window \a before.
 *
 * \param[in] app   application handle
 * \param[in] wnd   window to be inserted
 * \param[in] before window before which \a wnd will be inserted (or @c NULL to insert \a wnd on top of the display stack)
 * \param[in] off_x  x coordinate of top-left window \a wnd corner in application space
 * \param[in] off_y  y coordinate of top-left window \a wnd corner in application space
 *
 * \note This function does not set focus to inserted window. Use svAppSetFocusedWindow() to do this.
 */
extern void
svAppPutWindow(SvApplication app, SvWindow wnd, SvWindow before, int off_x, int off_y);

/**
 * Push a window on top of the display stack.
 *
 * Window will be automatically displayed on top of other windows and will gain focus.
 *
 * \param[in] app    application handle
 * \param[in] wnd    window to be pushed
 * \param[in] off_x  x coordinate of top-left window \a wnd corner in application space
 * \param[in] off_y  y coordinate of top-left window \a wnd corner in application space
 */
extern void
svAppPushWindow(SvApplication app, SvWindow wnd, int off_x, int off_y);

/**
 * Add a window to the display queue.
 *
 * Window will be inserted after window \a after.
 *
 * \param[in] app   application handle
 * \param[in] after window after which \a wnd will be insrted (or @c NULL to insert \a wnd on bottom of the display queue)
 * \param[in] wnd   window to be inserted
 * \param[in] off_x  x coordinate of top-left window \a wnd corner in application space
 * \param[in] off_y  y coordinate of top-left window \a wnd corner in application space
 *
 * \note This function does not set focus to inserted window. Use svAppSetFocusedWindow() to do this.
 **/
extern void
svAppAddWindow(SvApplication app, SvWindow after, SvWindow wnd, int off_x, int off_y);

/**
 * Add a window to the display queue.
 *
 * Window will be inserted before window \a before.
 *
 * \param[in] app   application handle
 * \param[in] before window before which \a wnd will be insrted (or @c NULL to insert \a wnd on top of the display queue)
 * \param[in] wnd   window to be inserted
 * \param[in] off_x  x coordinate of top-left window \a wnd corner in application space
 * \param[in] off_y  y coordinate of top-left window \a wnd corner in application space
 *
 * \note This function does not set focus to inserted window. Use svAppSetFocusedWindow() to do this.
 **/
extern void
svAppAddBeforeWindow(SvApplication app, SvWindow before, SvWindow wnd, int off_x, int off_y);

/**
 * Remove a window from the application
 *
 * \param[in] app application handle
 * \param[in] wnd window to be removed
 * \return removed window: @a wnd or @c NULL if not found
*/
extern SvWindow
svAppRemoveWindow(SvApplication app, SvWindow wnd);

/**
 * Remove last window (top one) from application.
 *
 * Focus is granted to window which becomes top one.
 * \param[in] app application handle
 * \return removed window
 */
extern SvWindow
svAppPopWindow(SvApplication app);

/**
 * Give focus to selected window.
 *
 * \param[in] app application handle
 * \param[in] wnd window to focus
 */
extern void
svAppSetFocusedWindow(SvApplication app, SvWindow wnd);

/**
 * Retrieve the currently focused window.
 *
 * \param[in] app application handle
 * \return focused window
 */
extern SvWindow
svAppGetFocusedWindow(SvApplication app);

/**
 * Dump information about all application's windows to the standard output.
 *
 * \param[in] app application handle
 **/
extern void
svAppDumpWindows(SvApplication app);

/**
 * @}
 **/


/**
 * @defgroup SvApplicationEvents Sending and receiving events
 * @{
 **/

/**
 * Deliver input event to the specified window.
 *
 * \param[in] app application handle
 * \param[in] wnd window to deliver input event to
 * \param[in] event input event
 * \return event delivery status: @c true if handled, @c false otherwise
 **/
extern bool
svAppDeliverInputEvent(SvApplication app, SvWindow wnd, QBInputEvent event);

/**
 * Queue event for delivery.
 *
 * This function enqueues @a event for delivery by the main loop
 * of the application.
 *
 * \param[in] app application handle
 * \param[in] sender sender widget
 * \param[in] event event to be sent
 **/
extern void
svAppPostEvent(SvApplication app, SvWidget sender, SvEvent event);

/**
 * Deliver user event immediately.
 *
 * This function delivers user @a event immediately it recipient in found.
 *
 * \param[in] app application handle
 * \param[in] sender sender widget
 * \param[in] event event to send
 * \return @c true if at least one recipient was found, @c false otherwise
 **/
extern bool
svAppDeliverEvent(SvApplication app, SvWidget sender, SvEvent event);

/**
 * @}
 **/


/**
 * @defgroup SvApplicationTimers Managing timers
 * @{
 **/

/**
 * Request a timer event to be sent to a widget.
 *
 * \param[in] app   application handle
 * \param[in] w     widget that will receive timer events
 * \param[in] secs  time (in seconds) after which timer event will be generated (also, interval between subsequent timer events)
 * \param[in] oneshot if @c true, timer event will be generated once, otherwise timer events will be generated until svAppTimerStop() is called
 * \return unique timer identifier
 *
 **/
extern SvTimerId
svAppTimerStart(SvApplication app, SvWidget w,
                double secs, int oneshot);

/**
 * Stop a timer.
 *
 * \param[in] app application handle
 * \param[in] id  identifier of timer that should be stopped
 **/
extern void
svAppTimerStop(SvApplication app, SvTimerId id);

/**
 * @}
 **/


/**
 * @defgroup SvApplicationEffects Controlling effects
 * @{
 **/

/**
 * Register and start effect.
 *
 * \param[in] app   application handle
 * \param[in] effect effect that should be registered
 * \return identifier of registered effect
 **/
extern SvEffectId
svAppRegisterEffect(SvApplication app, SvEffect effect);

/**
 * Cancel running effect.
 *
 * \param[in] app  application handle
 * \param[in] effectId identifier of the effect that should be canceled
 * \param type specify way in which effect should be ended
 * \return @c 0 in case of error, any other value otherwise
 **/
extern int
svAppCancelEffect(SvApplication app, SvEffectId effectId, SvEffectCompletionType type);

/**
 * Get current value of the effect timer.
 *
 * @param[in] app application handle
 * @return effect timer value
 **/
extern double
svAppGetEffectTimer(SvApplication app);

/**
 * @}
 **/


/**
 * Enable of disable partial updates.
 *
 * @param[in] app application handle
 * @param[in] enable @c true to enable partial updates, @c false to disable
 **/
extern void
svAppEnablePartialUpdates(SvApplication app, bool enable);

/**
 * Force screen redraw in the next turn of the main CUIT loop.
 *
 * @param[in] app application handle
 **/
extern void
svAppForceRender(SvApplication app);

/**
 * Check if given widget is currently visible on the screen.
 *
 * @param[in] app application handle
 * @param[in] widget widget to check for visibility
 * @return @c true if @a widget is visible, @c false if not
 **/
extern bool
svAppWidgetIsVisible(SvApplication app, SvWidget widget);

/**
 * Find widget by its identifier.
 *
 * @param[in] app    application handle
 * @param[in] id     widget identifier
 * @return           widget handle or @c NULL if not found
 **/
extern SvWidget
svAppFindWidget(SvApplication app, SvWidgetId id);

/**
 * Add render listener.
 *
 * \param[in] app       application handle
 * \param[in] listener  handle to a listener implementing \ref SvAppRenderListener
 **/
extern void
svAppAddRenderListener(SvApplication app, SvObject listener);

/**
 * Get widgets' count.
 *
 * \param[in] app       application handle
 * \return count of widgets that are already in memory
 **/
extern unsigned int
svAppGetWidgetsCount(SvApplication app);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
