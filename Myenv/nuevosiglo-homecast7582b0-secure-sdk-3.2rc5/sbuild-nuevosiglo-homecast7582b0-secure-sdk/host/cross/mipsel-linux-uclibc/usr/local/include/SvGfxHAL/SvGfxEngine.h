/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_GFX_ENGINE_H_
#define SV_GFX_ENGINE_H_

/**
 * @file SvGfxEngine.h GFX engine API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <CAGE/Core/Sv2DRect.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <SvGfxHAL/SvGfxConfig.h>
#include <SvGfxHAL/SvGfxQueue.h>
#include <SvGfxHAL/SvGfxOnVideoQueue.h>
#include <SvGfxHAL/SvHWSurface.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvGfxEngine Accelerated 2D graphics engine
 * @ingroup SvGfxHAL
 * @{
 *
 * An interface for controlling accelerated 2D graphics engine.
 **/

/**
 * Initialize GFX engine.
 *
 * @param[in] width     desired GFX canvas width
 * @param[in] height    desired GFX canvas height
 * @param[in] aspectRatio aspect ratio of GFX canvas
 * @return              @c 0 on success, @c -1 otherwise
 **/
extern int
SvGfxEngineInit(unsigned int width,
                unsigned int height,
                QBAspectRatio aspectRatio);

/**
 * Close GFX engine.
 *
 * This method deinitializes GFX engine, freeing all occupied resources:
 * GFX canvas, layers, viewports, framebuffers and temporary GFX surfaces.
 **/
extern void
SvGfxEngineDeinit(void);

/**
 * Check whether GFX engine is initialized.
 *
 * @return              @c true if engine is initialized, @c false otherwise
 **/
extern bool
SvGfxEngineIsInitialized(void);

/**
 * Setup GFX viewport.
 *
 * @param[in] outputID     ID of the output to configure viewport for
 * @param[in] mode         desired viewport mode
 * @param[in] outputRect   output rectangle, used only
 *                         if @a mode == QBViewportMode_windowed
 * @param[in] inputRect    input rectangle, pass @c NULL to make input
 *                         rectangle the size of the GFX frame buffer
 * @return                 @c 0 on success, @c -1 otherwise
 **/
extern int
SvGfxEngineSetupViewport(unsigned int outputID,
                         QBViewportMode mode,
                         const Sv2DRect *outputRect,
                         const Sv2DRect *inputRect);

/**
 * Get GFX viewport.
 *
 * @param[in] outputID     ID of the output to get viewport for
 * @param[out] mode        viewport mode
 *                         (can be @c NULL if not needed)
 * @param[out] outputRect  output rectangle, relevant only
 *                         if @a mode == QBViewportMode_windowed
 *                         (can be @c NULL if not needed)
 * @param[out] inputRect   input rectangle
 *                         (can be @c NULL if not needed)
 * @return                 @c 0 on success, @c -1 otherwise
 **/
extern int
SvGfxEngineGetViewport(unsigned int outputID,
                       QBViewportMode *mode,
                       Sv2DRect *outputRect,
                       Sv2DRect *inputRect);

/**
 * Get GFX engine configuration.
 *
 * @return              current GFX engine configuration,
 *                      @c NULL if engine is not initialized
 **/
extern const SvGfxConfig *
SvGfxEngineGetConfig(void);


/**
 * GFX engine extended configuration options.
 **/
typedef enum {
    /**
     * reserved option to prevent invalid usage - when enum is not set,
     * nor initialized
     **/
    SvGfxEngineOption_reserved = 0,
    /**
     * boolean value: @c true to allocate canvas in YCbCr color space,
     * @c false for platform default value
     * (it may also affect default color space for other surfaces)
     **/
    SvGfxEngineOption_useYCbCrFrameBuffer = 1u,
    /**
     * boolean value: @c true when we want use YUYV (4:2:2) instead of
     * YUV (4:4:4), @c false default value
     **/
    SvGFxEngineOption_useYUYV = 4u,
    /**
     * number of Gfx engine options
     **/
    SvGfxEngineOptions__count__
} SvGfxEngineOption;

/**
 * Get value of one of GFX engine extended configuration options.
 *
 * @param[in] option    extended configuration option
 * @return              option value, @c -1 if not defined for this platform
 **/
extern long int
SvGfxEngineGetOption(SvGfxEngineOption option);

/**
 * Change value of one of GFX engine extended configuration options.
 *
 * This function changes extended configuration options of the GFX engine.
 *
 * @note Some options (it is platform-specific which one) can only be
 * changed before GFX engine is initialized.
 *
 * @param[in] option    extended configuration option
 * @param[in] value     new value for @a option
 * @return              @c 0 on success, @c -1 otherwise
 **/
extern int
SvGfxEngineSetOption(SvGfxEngineOption option,
                     long int value);

/**
 * Create GFX surface.
 *
 * @since 1.3.3
 *
 * @param[in] width     surface width
 * @param[in] height    surface height
 * @param[in] csp       color space of the source bitmap
 * @return              a handle to an allocated GFX surface,
 *                      @c NULL in case of error
 **/
extern SvHWSurface
SvGfxEngineCreateSurface(unsigned int width,
                         unsigned int height,
                         SvColorSpace csp);

/**
 * Perform GFX operations.
 *
 * @param[in] dest      destination GFX surface handle
 *                      (@c NULL to render to the GFX canvas and send rendered frame
 *                      to video outputs)
 * @param[in] queue     handle to a queue of GFX operations
 * @return              @c 1 for ok, @c 0 if busy right now,
 *                      @c -1 in case of error
 **/
extern int
SvGfxEngineBlitQueue(SvHWSurface dest,
                     const SvGfxQueue *queue);

/**
 * Send previously rendered frame again to video outputs.
 *
 * This function signals that next displayed frame doesn't need to be rendered.
 * It is used when there are no changes on the screen between consecutive frames.
 * For most HAL implementations, this is a no-op.
 *
 * @return              @c 0 on success, @c -1 otherwise
 **/
extern int
SvGfxEngineBlitEmpty(void);

/**
 * Enable/disable graphics on given @a outputID.
 *
 * This function disconnects graphics layer from given video output.
 * On some platforms it must be used before output is reconfigured.
 *
 * @param[in] outputID   ID of output to be changed
 * @param[in] enable     @c true to enable displaying graphics on given output
 * @param[in] cleanup    @c true to permanently disable graphics on given output and free all internally
 *                       used resources (ignored when @a enable == @c false)
 **/
extern void
SvGfxEngineEnableGfxOnOutput(unsigned int outputID,
                             bool enable,
                             bool cleanup);

/**
 * Set graphics on given output to passive/active mode.
 *
 * @param[in] outputID   ID of output to be set
 * @param[in] active     @c true to set output to active mode, @c false for passive mode
 **/
extern void
SvGfxEngineSetActiveMode(unsigned int outputID,
                         bool active);

/**
 * Set surface to be shown (instead of graphics canvas) on outputs
 * being in passive graphics mode.
 *
 * @param[in] surface   GFX surface to be shown in passive graphics mode
 * @return              @c 0 on success, @c -1 otherwise
 **/
extern int
SvGfxEngineSetPassiveSurface(SvHWSurface surface);

/**
 * Return color space of the GFX surface created for the source bitmap in color space @a csp.
 *
 * @param[in] csp       color space of the source bitmap
 * @return              @c color space of the GFX surface
 **/
extern SvColorSpace
SvGfxEngineGetHWColorSpace(const SvColorSpace csp);

/**
 * Add image to specific output, relative to video display
 *
 * Object destination rect (@c object->dest_rect) will be scaled
 * relative to 720x576 (PAL) rectangle
 *
 * @param[in] outputID    ID of output
 * @param[in] object      image to use on selected output, dest_rect is
 *                        relative to 720x576 (PAL) rectangle
 * @param[in] minimalSize minimal size of object for display in canvas coordinates,
 *                        x and y params are ignored, NULL if not used,
 *                        if width or height are 0, then aspect ratio is kept
 * @param[in] type        object type
 * @param[in] handle      handle to image
 * @return                @c 0 on success, @c -1 otherwise
 **/
extern int
SvGfxEngineAddImageOnVideo(unsigned int outputID,
                           SvGfxObject *object,
                           const Sv2DRect *minimalSize,
                           SvGfxOnVideoQueueObjectType type,
                           unsigned int *handle);

/** @cond */
// compatibility alias, to be removed ASAP
static inline int
SvGfxAddImageOnVideo(unsigned int outputID,
                     SvGfxObject *object,
                     const Sv2DRect *minimalSize,
                     SvGfxOnVideoQueueObjectType type,
                     unsigned int *handle)
{
    return SvGfxEngineAddImageOnVideo(outputID, object, minimalSize, type, handle);
}
/** @endcond */

/**
 * Remove image from specific output
 *
 * @param[in] outputID  ID of output
 * @param[in] handle    handle to image
 * @return              @c 0 on success, @c -1 otherwise
**/
extern int
SvGfxEngineRemoveImageFromVideo(unsigned int outputID,
                                unsigned int handle);

/** @cond */
// compatibility alias, to be removed ASAP
static inline int
SvGfxRemoveImageFromVideo(unsigned int outputID,
                          unsigned int handle)
{
    return SvGfxEngineRemoveImageFromVideo(outputID, handle);
}
/** @endcond */

/**
 * Check if gfx engine has 3D operations support.
 *
 * @return                  @c true if GFX engine is capable of 3D operations,
 *                          @c false otherwise
 **/
extern bool
SvGfxEngineHas3DSupport(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
