/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_GFX_HAL_H
#define QB_GFX_HAL_H

#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvSurface.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#include <stdint.h>

struct Sv3DOperation_s;
struct SvColorOperation_s;

//------------------------------------------
// Common api.

typedef struct QBGfxHALCapabilities_s  QBGfxHALCapabilities;
struct QBGfxHALCapabilities_s
{
    /// How many framebuffers we are to allocate per output.
    /// This is the major factor determining the gfx algorithm.
    /// Possible values are:
    /// 0 : framebuffers are not used, we're drawing on canvas, which is then sent down with commit_hw_canvas()
    /// 2 : double-buffering, we cannot copy to any framebuffer while a flip is ongoing
    /// 3 : triple-buffering, we can always copy to one free framebuffer, even when two other are flipping
    uint32_t framebuffers_per_output;

    /// How much we can upscale with a single operation.
    /// Used only for determining if we can upscale: 1 -> N pixels.
    /// Iff 0, then there's no limit.
    uint32_t max_bitmap_upscale_factor_x;
    uint32_t max_bitmap_upscale_factor_y;

    /// How much we can downscale with a single operation.
    /// Iff 0, then there's no limit.
    uint32_t max_bitmap_downscale_factor_x;
    uint32_t max_bitmap_downscale_factor_y;

    /// Can convert scp while doing upload_bitmap_data().
    /// Else: app has to upload the data passing the exact same csp as the target surface.
    bool can_upload_bitmap_with_csp_conversion;

    /// If set to true it means that this HAL does not support partial rendering and
    /// the whole scene needs to be redrawn on each frame.
    bool forceFullRedraw;

    /// If set to true it means that this HAL supports 3d operations
    bool has3DSupport;

    /// Add margin to calculated dirty areas for partial redraw
    int dirtyRectMargin;
};

//------------------------------------------
// For asynchronous handling.

/// Render finnished callback
typedef void (QBGfxHALRenderFinishedCallback)(void* arg);

/// Flip finnished callback
typedef void (QBGfxHALFlipFinishedCallback)(void* arg, int outputID);

/**
 * Descriptor of the GFX engine implementation.
 **/
typedef struct QBGfxEngineImplementation_ {
    /// Get the Gfx engine capability specification structure.
    ///
    /// \return     structure describing Gfx engine capabilities
    /// \sa         QBGfxHALCapabilities, QBGfxHALCapabilities_s
    const QBGfxHALCapabilities* (*getCapabilities)(void);

    /// Initialize the Gfx engine.
    ///
    /// \param  w   graphics width
    /// \param  h   graphics height
    /// \return     \c 0 on success, \c -1 on error
    int (*init)(unsigned int w, unsigned int h);

    /// Clean up the Gfx engine
    ///
    /// \return     \c 0 on success, \c -1 on error
    int (*deinit)(void);

    SvHWSurface (*allocSurface)(unsigned int w, unsigned int h, SvColorSpace csp, const char* debug_name);

    int (*uploadBitmapData)(SvHWSurface dest, Sv2DRect dstRect, SvSurfaceIterator srcDesc);

    /// Return color space of the GFX surface created for the source bitmap in color space \a csp.
    /// \param csp  color space of the source bitmap
    /// \return \c color space of the GFX surface
    SvColorSpace (*getHwColorSpace)(const SvColorSpace csp);

    /// Set target of every future QBGfxHAL_draw_*() operation.
    int (*setTarget)(SvHWSurface dest);

    /// Draw solid colored rectangle.
    /// \param color  source color
    /// \param blend  true iff the color is to be blended with target, else it will overwrite the target
    /// \param isPremultiplied  true iff \a color is given in premultiplied form
    int (*drawColor)(Sv2DRect dstRect, SvColor color,
                      bool blend, bool isPremultiplied);

    /// Draw rectagle, with bitmap multiplied with given color and with dim effect.
    /// \param src  source bitmap
    /// \param modulateColor  color to modulate with each bitmap pixel before blending with target (must be non-premultiplied)
    /// \param dimFactor  dim effect to be applied after bitmap*color modulation
    /// \param blend  true iff the source is to be blended with target, else it will overwrite the target
    /// \param isPremultiplied  true iff \a bitmap pixels are in premultiplied form
    int (*drawBitmap)(Sv2DRect dstRect,
                       SvHWSurface src, Sv2DRect srcRect,
                       SvColor modulateColor, int dimFactor,
                       bool blend, bool isPremultiplied);

    /// Draw some recognizable color pattern, used for debugging only.
    int (*drawPattern)(Sv2DRect dstRect);

    /// Handle 3D operation.
    int (*handle3DOperation)(const struct Sv3DOperation_s* operation);

    /// Handle Color operation.
    int (*handleColorOperation)(const struct SvColorOperation_s *operation);

    //------------------------------------------
    // For asynchronous handling.

    int (*setRenderFinishedCallback)(QBGfxHALRenderFinishedCallback* cb, void* arg);

    /// Request for renderFinishedCallback() to be called when all issued drawing requests are done.
    /// \returns 1   if all ops are alreay done (render_finished_callback will NOT be called)
    /// \returns 0   if drawing is still in progress (render_finished_callback will be called later, possibly from another thread)
    /// \returns <0  on error
    int (*requestRenderFinishedCallback)(void);

    int (*setFlipFinishedCallback)(QBGfxHALFlipFinishedCallback* cb, void* arg);

    //---------------------------------------------------
    // For simple platforms, that do all the
    // framebuffer copy/doublebuffering/flip internally.

    /// Get canvas to draw the next frame on.
    /// The canvas must contain the results of previous renderings.
    SvHWSurface (*getHwCanvas)(void);

    /// Push hardware canvas to be displayed on all outputs.
    int (*commitHwCanvas)(void);

    /// Setup Gfx viewport params
    int (*setupViewport)(unsigned int output, QBViewportMode mode, const Sv2DRect *outputRect, const Sv2DRect *inputRect);

    /// Get Gfx viewport params
    int (*getViewport)(unsigned int output, QBViewportMode *mode, Sv2DRect *outputRect, Sv2DRect *inputRect);

    /// Get next frame size
    bool (*getFBSize)(int outputID, SvHWSurface surf_, unsigned int* width, unsigned int* height);

    //-----------------------------------------------------
    // For platforms where framebuffers are normal bitmaps
    // and we need to manage them manually.

    /// Allocate a single surface fitted to be used as a framebuffer on output \a outputID.
    /// This should only be done when we're certain that the output exists, is a master output,
    /// and is properly configured.
    /// After any re-configurations of the output, this surface is assumed to be invalid.
    /// But if it's not destroyed explicitly, the driver MAY still keep its resources,
    /// to be reused when allocating new framebuffer.
    /// \param old  framebuffer allocated before re-configuration, may be re-used, if the driver allows it
    ///             (if not, the driver will just destroy it now).
    SvHWSurface (*allocFramebuffer)(int outputID, SvHWSurface old);

    /// Request \a fb to be shown on output \a outputID.
    /// Also request the flip_finished_callback() to come after this operation is done.
    /// \returns 1   if the flip has been done immediately (flip_finished_callback will NOT be called)
    /// \returns 0   if flip has been scheduled (flip_finished_callback will be called later, possibly from another thread)
    /// \returns <0  on error
    int (*switchFramebuffer)(int outputID, SvHWSurface fb);

    /// force use YUYV (4.2.2) instead of YUV (4.4.4)
    void (*setUseYUYV)(bool use);

    /// return true if YUYV(4.2.2) is used instead of YUV (4.4.4)
    bool (*getUseYUYV)(void);
} QBGfxEngineImplementation;

#endif // #ifndef QB_GFX_HAL_H
