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

#ifndef CUIT_WIDGET_H_
#define CUIT_WIDGET_H_

/**
 * @file SMP/CUIT/widget.h CUIT widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>

#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>

#include <CUIT/Core/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvWidget CUIT widget
 * @ingroup CUIT
 * @{
 **/

/** cleanup function type */

typedef void (*SvWidgetCleanup)(SvApplication app, void *prv);

/** input handler function type */
typedef bool (*SvInputEventHandler)(SvWidget w, SvInputEvent e);

/** focus handler function type */
typedef void (*SvFocusEventHandler)(SvWidget w, SvFocusEvent e);

/** effect handler function type */
typedef void (*SvEffectEventHandler)(SvWidget w, SvEffectEvent e);

/** timer handler function type */
typedef void (*SvTimerEventHandler)(SvWidget w, SvTimerEvent e);

/** user handler function type */
typedef void (*SvUserEventHandler)(SvWidget w, SvWidgetId sender, SvUserEvent e);

/**
 * Widget types.
 **/
typedef enum {
  /// standard 2D widget
  SvWidgetType_2D = 0,
  /// video viewport widget
  SvWidgetType_viewport
} SvWidgetType;

/**
 * Widget class.
 * @internal
 **/
struct SvWidget_t {
  // this short attributes have to be together
  // TODO: we can use 'unsigned char' for 'type__' in the future when we remove 'input_events_mask'
  unsigned short int  type__;         ///< widget type (INT/2)
  /// types of input events widget wants to receive (INT/2)
  unsigned short int input_events_mask;

  SvWidgetId id__;                    ///< widget identifier (INT)
  SvApplication app;                  ///< parent application handle (INT)
  struct SvWidget_t *prev;            ///< pointer to other widgets in widget tree (INT)
  struct SvWidget_t *next;            ///< pointer to other widgets in widget tree (INT)
  struct SvWidget_t *parent__;        ///< parent widget (NULL for Window) (INT)
  SvWindow window__;                  ///< window the widget belongs to (INT)
  struct SvWidget_t *first;           ///< child widgets sorted by level (INT)
  struct SvWidget_t *last;            ///< child widgets sorted by level (INT)

  // this short attributes have to be together
  unsigned short int num_children;    ///< number of children (INT/2)
  unsigned short int level__;         ///< z-value of the widget (INT/2)

  // this bool attributes have to be together (4 bools make 'one INT')
  bool hidden__;                      ///< is widget hidden (sub-tree operation)
  bool cutFromBitmap;                 ///< should we cut this widget from bitmap relative absolute position
  /// internal, is set to true when first timer on this widget is run
  bool uses_timers;
  bool acceptFocus__;                 ///< is widget accept focus

  struct {
    SvFocusEventHandler focusEventHandler;   ///< focus event handler
    SvEffectEventHandler effectEventHandler; ///< effect event handler
    SvInputEventHandler inputEventHandler;   ///< input event handler
    SvTimerEventHandler timerEventHandler;   ///< timer event handler
    SvUserEventHandler userEventHandler;     ///< user event handler
  } eventHandlers;                    ///< event handlers

  struct SvWidget_t *focused;         ///< focused widget (in a window) (INT)

  void* prv;                          ///< widget's private data
  SvWidgetCleanup clean;              ///< cleanup function (if not set and \a prv is set, \a free() function will be used to free \a prv)

  int off_x;                          ///< x offset in the parent widget (or in the screen in case of windows, or zero for objects)
  int off_y;                          ///< y offset in the parent widget (or in the screen in case of windows, or zero for objects)
  int width;                          ///< width of the widget (@c 0 for object)
  int height;                         ///< height of the widget (@c 0 for object)
  SvPixel alpha;                      ///< global alpha scaler
  SvPixel tree_alpha;                 ///< sub-tree alpha or widget alpha
  SvPixel dim;                        ///< global dim
  SvPixel lighten;                    ///< global lighten
  SvPixel tree_dim;                   ///< sub-tree dim
  SvPixel tree_lighten;               ///< sub-tree lighten
  SvColor fill_color;                 ///< fill the widget with a color instead of using background bitmap.

  // Underlying bitmap
  SvBitmap bmp;                       ///< bitmap handle
  int bm_x;                           ///< x offset in the bitmap
  int bm_y;                           ///< y offset in the bitmap
  int bm_w;                           ///< horizontal fragment of the bitmap (defines scale factor). Can be negative for 3D widgets to generate reflection of the bitmap.
  int bm_h;                           ///< vertical fragment of the bitmap (defines scale factor). Can be negative for 3D widgets to generate reflection of the bitmap.

  char *name__;                       ///< name of widget

  /// head of the list of effects operating on this widget
  SvEffect eff_first;
  /// tail of the list of effects operating on this widget
  SvEffect eff_last;

  /// if set to @c true, focus event handler will be called every time
  /// any child widget will get or lose focus.
  /// @note Do not access this field directly, use @ref svWidgetSetFocusMonitor
  /// or @ref svWidgetIsFocusMonitor instead.
  bool focus_monitor__;

  /// if set to @c true, widget is used as underlying widget for QBCUITWidget,
  /// in such case, widget private data points to QBCUITWidget instance which
  /// is retained when widget is attached and released when widget is released
  bool is_cuit_widget__;
};

/** @cond */
#define input_eh eventHandlers.inputEventHandler
/** @endcond */

/**
 * Create empty widget.
 *
 * \param[in] app application handle
 * \param[in] w widget width
 * \param[in] h widget height
 * \returns new widget
 */
extern SvWidget svWidgetCreate(SvApplication app, unsigned int w, unsigned int h);

/** Create widget with a provided bitmap as its background.
 *
 * \param[in] app application handle
 * \param[in] w widget width
 * \param[in] h widget height
 * \param[in] bmp bitmap
 * \returns new widget
 * \note If \a bmp is NULL then color fill is used as a background. Full bitmap will be used as a background, if bitmap size and the widget size differ, the bitmap will be scaled.
 */
SvWidget svWidgetCreateBitmap(SvApplication app, unsigned int w, unsigned int h, SvBitmap bmp);

/** Create widget that cannot handle events with a provided bitmap as its background.
 *
 * \param[in] app application handle
 * \param[in] w widget width
 * \param[in] h widget height
 * \param[in] bmp bitmap
 * \returns new widget
 * \note Widgets created with this function cannot handle events. svWidgetCreateBitmap() notes apply.
 */
SvWidget svWidgetCreateSimple(SvApplication app, unsigned int w, unsigned int h, SvBitmap bmp);

/** Set widget's background bitmap.
 *
 * \param[in] widget  widget handle
 * \param[in] bmp new bitmap
*/
void svWidgetSetBitmap(SvWidget widget, SvBitmap bmp);

/**
 * Create a 'colored rectangle' widget.
 *
 * \param[in] app application handle
 * \param[in] w widget width
 * \param[in] h widget height
 * \param[in] color widget color
 * \return new widget
 */
extern SvWidget svWidgetCreateWithColor(SvApplication app, unsigned int w, unsigned int h, SvColor color);

/**
 * Create a 'colored rectangle' widget.
 *
 * This function is a wrapper for svWidgetCreateWithColor().
 *
 * \param[in] app application handle
 * \param[in] w widget width
 * \param[in] h widget height
 * \param[in] color widget color
 * \param[in] gradient deprecated, pass @c false
 * \param[in] gradientColor deprecated, pass @c 0
 * \param[in] gradientAngle deprecated, pass @c 0
 * \return new widget
 */
static inline SvWidget svWidgetCreateColor(SvApplication app, int w, int h, SvColor color,
                                           bool gradient, SvColor gradientColor, int gradientAngle)
{
  return svWidgetCreateWithColor(app, w, h, color);
}

/**
 * Create 2D video viewport widget.
 *
 * @param[in] app application handle
 * @param[in] w widget width
 * @param[in] h widget height
 * @return 2D video viewport widget
 **/
extern SvWidget svWidgetCreateViewport(SvApplication app, unsigned int w, unsigned int h);

/**
 * Create copy of a widget.
 *
 * Bitmaps will be shared with the original one.
 * Private data of source @a widget will not be copied.
 *
 * \param[in] widget widget handle
 * \return copy of @a widget
 */
extern SvWidget svWidgetCreateCopy(SvWidget widget);

/**
 * Destroy widget and all its subwidgets.
 *
 * \param[in] w widget handle
 */
extern void svWidgetDestroy(SvWidget w);

/**
 * Destroy all widgets subwidgets.
 *
 * \param[in] w widget handle
 */
extern void svWidgetDestroySubwidgets(SvWidget w);

/**
 * Attach widget to parent.
 *
 * \see svWidgetAttachQBWidget
 *
 * \param[in] parent parent widget
 * \param[in] child widget that will be attached
 * \param[in] off_x x coordinate of top-left corner of widget (parent relative)
 * \param[in] off_y y coordinate of top-left corner of widget (parent relative)
 * \param[in] level Z-order of the widget (higher number = closer to viewer)
 */
extern void svWidgetAttach(SvWidget parent, SvWidget child,
                           int off_x, int off_y, unsigned short int level);

/**
 * Attach QBCUITWidget to parent.
 *
 * \see svWidgetAttach
 *
 * \param[in] parent parent widget
 * \param[in] child widget that will be attached
 * \param[in] off_x x coordinate of top-left corner of widget (parent relative)
 * \param[in] off_y y coordinate of top-left corner of widget (parent relative)
 * \param[in] level Z-order of the widget (higher number = closer to viewer)
 */
extern void svWidgetAttachQBCUITWidget(SvWidget parent, QBCUITWidget child,
                                       int off_x, int off_y, unsigned short int level);

/**
 * Attach widget after given widget.
 *
 * \param[in] brother widget, after which \a widget will be attached
 * \param[in] widget widget that will be attached
 * \param[in] off_x x coordinate of top-left corner of widget (parent relative)
 * \param[in] off_y y coordinate of top-left corner of widget (parent relative)
 */
extern void svWidgetAttachAfter(SvWidget brother, SvWidget widget,
                                int off_x, int off_y);

/**
 * Attach widget before given widget.
 *
 * \param[in] brother widget before which \a widget will be attached
 * \param[in] widget widget that will be attached
 * \param[in] off_x x coordinate of top-left corner of widget (parent relative)
 * \param[in] off_y y coordinate of top-left corner of widget (parent relative)
 */
extern void svWidgetAttachBefore(SvWidget brother, SvWidget widget,
                                 int off_x, int off_y);

/**
 * Detach widget from its parent.
 *
 * \param[in] widget widget handle
 */
extern void svWidgetDetach(SvWidget widget);

/**
 * Check whether widget is a focused widget.
 *
 * \param[in] widget widget handle
 * \return @c true if widget is focused, @c false if not
 */
extern bool svWidgetIsFocused(SvWidget widget);

/**
 * Set focused widget.
 *
 * \param[in] widget widget handle
 */
extern void svWidgetSetFocus(SvWidget widget);

/**
 * Search for closest child of the widget.
 *
 * \param[in] widget widget handle
 */
extern void svWidgetSetFocusToClosestChild(SvWidget widget);

/**
 * Drop focus from widget.
 *
 * \param[in] widget widget handle
 */
extern void svWidgetDropFocus(SvWidget widget);

/**
 * Check whether widget is a window.
 *
 * \param[in] widget widget handle
 * \return @c true if widget is a window, @a false if not
 */
extern bool svWidgetIsWindow(SvWidget widget);

/**
 * Check whether window is a focused window.
 *
 * \param[in] window window handle
 * \return @c true if window is focused, @a false if not
 */
extern bool svWidgetIsFocusedWindow(SvWindow window);

/**
 * Signal that widget had changed and application redraw may be needed.
 *
 * \param[in] widget widget handle
 */
extern void svWidgetForceRender(SvWidget widget);

/**
 * Register new input event handler.
 *
 * \param[in] w widget handle
 * \param[in] handler input event handler function
 */
extern void svWidgetSetInputEventHandler(SvWidget w, SvInputEventHandler handler);

/**
 * Register new focus event handler.
 *
 * \param[in] w widget handle
 * \param[in] handler focus event handler function
 */
extern void svWidgetSetFocusEventHandler(SvWidget w, SvFocusEventHandler handler);

/**
 * Register new timer event handler.
 *
 * \param[in] w widget handle
 * \param[in] handler timer event handler function
 */
extern void svWidgetSetTimerEventHandler(SvWidget w, SvTimerEventHandler handler);

/**
 * Register new effect event handler.
 *
 * \param[in] w widget handle
 * \param[in] handler effect event handler function
 */
extern void svWidgetSetEffectEventHandler(SvWidget w, SvEffectEventHandler handler);

/**
 * Register new user event handler.
 *
 * \param[in] w widget handle
 * \param[in] handler user event handler function
 */
extern void svWidgetSetUserEventHandler(SvWidget w, SvUserEventHandler handler);

/**
 * Generic input event handler.
 *
 * This function is an input event handler installed by default
 * in every created widget.
 *
 * When @a widget is a window, this function will act as a focus manager
 * for this window. If you are installing custom input event handler
 * in a window, it is a good practice to call this function from it
 * to handle focus movement key.
 *
 * \param[in] widget widget handle
 * \param[in] event input event
 * \return @c true if @a event was consumed, otherwise @c false
 */
extern bool svWidgetInputEventHandler(SvWidget widget, SvInputEvent event);

/**
 * Input event handler for subwidget.
 *
 * It acts exactly like svWidgetInputEventHandler(),
 * but for given widget's subwidgets
 *
 * \param[in] w widget handle
 * \param[in] oldFocused last focused widget
 * \param[out] newFocused new focused widget
 * \param[in] event input event
 * \return @c true if @a event was consumed, otherwise @c false
 */
extern bool
svWidgetSubwidgetsInputEventHandler(SvWidget w,
                                    const SvWidget oldFocused,
                                    SvWidget *newFocused,
                                    SvInputEvent event);

/**
 * Hide or unhide widget.
 *
 * \param[in] w widget handle
 * \param[in] hidden @c true to hide widget, @c false to unhide hidden widget
 **/
extern void svWidgetSetHidden(SvWidget w, bool hidden);

/**
 * Set widgets alpha value.
 *
 * Alpha value affects only widget on which it was set.
 * Final alpha of the widget is calculated by multiplication of
 * widgets alpha and product of multiplication of all
 * tree alpha values up to the windows widgets tree root.
 *
 * \param[in] w widget handle
 * \param[in] alpha alpha value
 */
extern void svWidgetSetAlpha(SvWidget w, SvPixel alpha);

/**
 * Set widgets tree alpha value.
 *
 * Tree alpha value affects whole widgets sub tree
 * that originates from this widget.
 * Final alpha of the widget is calculated by multiplication of
 * widgets alpha and product of multiplication of all
 * tree alpha values up to the widgets tree root.
 *
 * \param[in] w widget handle
 * \param[in] treeAlpha tree alpha value
 */
extern void svWidgetSetTreeAlpha(SvWidget w, SvPixel treeAlpha);

/**
 * Is widget hidden.
 *
 * @param[in]     w widget handle
 *
 * @return        @c true if widget is hidden otherwise @c false
 **/
extern bool svWidgetIsHidden(SvWidget w);

/**
 * Find effect operating on given widget by effect id
 *
 * \param[in] w widget handle
 * \param[in] id effect id
 * \return effect handle or NULL if not found
 **/
extern SvEffect svWidgetFindEffectById(SvWidget w, SvEffectId id);


/**
 * Set name of widget.
 *
 * @param[in]    w widget handle
 * @param[in]    name the name of the widget
 **/
extern void svWidgetSetName(SvWidget w, const char* name);


/**
 * Get name of widget.
 *
 * @param[in]   w widget handle
 *
 * @return      name of the widget or @c NULL in case when
 *              widget doens't have set name.
 **/
extern const char* svWidgetGetName(SvWidget w);

/**
 * Check if widget is attached to another widget.
 *
 * @param[in]   w widget handle
 *
 * @return      @c true if @a w is attached, @c false otherwise
 **/
extern bool svWidgetIsAttached(SvWidget w);

/**
 * Get parent of widget.
 *
 * @param[in]   w widget handle
 *
 * @return      handle to parent widget or @c NULL in case when
 *              widget doesn't have parent.
 **/
extern SvWidget svWidgetGetParent(SvWidget w);

/**
 * Get ID of widget.
 *
 * @param[in]    w widget handle
 *
 * @return       identifier of widget.
 **/
extern SvWidgetId svWidgetGetId(SvWidget w);

/**
 * Get window that widget belongs to.
 *
 * @param[in]     w widget handle
 *
 * @return        handle to window or @c NULL is case when
 *                widget doesn't belong to window.
 **/
extern SvWindow svWidgetGetWindow(SvWidget w);

/**
 * Is widget focusable.
 *
 * @param[in]    w widget handle
 *
 * @return       @c true if widget accept focus otherwise @c false
 **/
extern bool svWidgetIsFocusable(SvWidget w);

/**
 * Set widget focus policy.
 *
 * @param[in]    w widget handle
 * @param[in]    isFocusable @c true if widget should accept focus otherwise @c false
 **/
extern void svWidgetSetFocusable(SvWidget w, bool isFocusable);

/**
 * Set widget to be focus monitor.
 *
 * If widget is a focus monitor, it will be notified in focus event handler
 * every time any of child widgets will get or lost focus.
 *
 * @see SvFocusEventKind.
 *
 * @param[in]   w            widget handle
 * @param[in]   focusMonitor @c true to set widget to be a focus monitor,
 *                           @c false otherwise
 **/
extern void svWidgetSetFocusMonitor(SvWidget w, bool focusMonitor);

/**
 * Is widget a focus monitor.
 *
 * If widget is a focus monitor, it will be notified in focus event handler
 * every time any of child widgets will get or lost focus.
 *
 * @see SvFocusEventKind.
 *
 * @param[in]   w widget handle
 *
 * @return      @c true if widget is a focus monitor, @c false otherwise
 **/
extern bool svWidgetIsFocusMonitor(SvWidget w);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
