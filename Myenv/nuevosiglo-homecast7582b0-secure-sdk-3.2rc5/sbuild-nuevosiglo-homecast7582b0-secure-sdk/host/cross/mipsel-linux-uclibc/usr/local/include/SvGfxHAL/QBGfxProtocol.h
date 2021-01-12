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

#ifndef QB_GFX_PROTOCOL_H_
# define QB_GFX_PROTOCOL_H_

/**
 * @file QBGfxProtocol.h Inter-process graphics protocol
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBGfxProtocol Inter-process graphics protocol
 * @ingroup SvGfxHAL
 * @{
 *
 * This protocol is used to describe graphics operations requested
 * by a client process to be performed by a renderer process that has
 * exclusive access to some 2D graphics acceleration hardware.
 *
 * Binary format of this protocol makes it easy to submit multiple
 * operations as a batch using just a single message from client
 * to renderer. The actual transport for those messages is not specified
 * here (it is defined by the renderer implementation), but it can work
 * using various kinds of sockets, message queues or shared memory.
 *
 * This protocol deliberately does not include any operations used for
 * managing resources. In private graphics memory mode (see @ref QBGfxProtocolSurface)
 * renderer must provide some way for the client to allocate surfaces
 * in the graphics memory and to free them when not needed anymore.
 * The method of establishing access to segments of memory shared between
 * client and renderer are also defined by the renderer implementation.
 **/

/**
 * @defgroup QBGfxProtocolCommonTypes Common data types
 * @{
 **/

/**
 * @enum QBGfxProtocolPixelFormat
 * @brief Pixel formats.
 **/
enum QBGfxProtocolPixelFormat {
    /// 8 bit pixels, only alpha channel
    QBGfxProtocolPixelFormat_A8 = 0x11u,
    /// 32 bit pixels, order (in memory): alpha, red, green, blue
    QBGfxProtocolPixelFormat_B8G8R8A8 = 0x14u
};

/**
 * @enum QBGfxProtocolSurfaceFlag
 * @brief Surface flags.
 **/
enum QBGfxProtocolSurfaceFlag {
    /**
     * flag set if color components in every pixel are stored
     * as premultiplied by alpha component
     **/
    QBGfxProtocolSurfaceFlag_preMultiplied = 0x1u
};

/**
 * Color value in RGB color space with alpha channel.
 **/
struct QBGfxProtocolColor {
    /// red component
    uint8_t red;
    /// green component
    uint8_t green;
    /// blue component
    uint8_t blue;
    /// alpha component
    uint8_t alpha;
};

/**
 * Description of a surface (bitmap) in memory.
 *
 * This type describes the layout of a surface in memory. It can be used
 * in two different memory access modes.
 *
 * In shared graphics memory mode client takes care of surface allocation
 * on its own using segment(s) of graphics memory shared between client
 * and renderer. Information about the layout of each individual surface
 * in memory is sent to the renderer with every operation that uses
 * a surface as its source or destination.
 *
 * In private graphics memory mode renderer manages surface allocation.
 * Client uses an identifier of a surface received from the renderer
 * to describe source and destination of requested operations.
 * This type is used only to describe a surface in shared memory
 * segment used for #QBGfxProtocolOperation_uploadSurface and
 * #QBGfxProtocolOperation_downloadSurface operations.
 **/
struct QBGfxProtocolSurface {
    /// format in which all pixels are stored (see ::QBGfxProtocolPixelFormat)
    uint8_t pixelFormat;
    /// additional flags (see ::QBGfxProtocolSurfaceFlag)
    uint8_t flags;
    /// identifier of the memory segment this surface resides in
    uint16_t memorySegment;
    /// offset (in bytes) where the first row of pixels starts in the memory segment
    uint32_t offset;
    /// width (number of columns) in pixels
    uint16_t width;
    /// height (number of rows) in pixels
    uint16_t height;
    /// distance in bytes between starts of two consecutive rows
    uint32_t stride;
};

/**
 * Description of a rectangular area of a surface.
 **/
struct QBGfxProtocolRect {
    /// X coordinate of the top-left corner in the surface
    uint16_t x;
    /// Y coordinate of the top-left corner in the surface
    uint16_t y;
    /// width (number of columns) in pixels
    uint16_t width;
    /// height (number of rows) in pixels
    uint16_t height;
};

/**
 * Description of a rectangular area of a surface described
 * with sub-pixel precision: all values are in 16.16 fixed
 * point format.
 **/
struct QBGfxProtocolFPRect {
    /// X coordinate of the top-left corner in the surface
    uint32_t x;
    /// Y coordinate of the top-left corner in the surface
    uint32_t y;
    /// width (number of columns) in pixels
    uint32_t width;
    /// height (number of rows) in pixels
    uint32_t height;
};

/**
 * Max length of a single operation, in bytes.
 **/
#define QB_GFX_PROTOCOL_MAX_OPERATION_LENGTH 120u

/**
 * Common header used in all operations.
 **/
struct QBGfxProtocolOperationHeader {
    /// operation code, see ::QBGfxProtocolOperation
    uint8_t code;
    /**
     * length of the operation structure in bytes,
     * will always be between @c 8 and
     * #QB_GFX_PROTOCOL_MAX_OPERATION_LENGTH
     **/
    uint8_t length;
    /// reserved, must be set to @c 0
    uint16_t reserved;
};

/**
 * @}
 **/


/**
 * @defgroup QBGfxProtocolOperations Operations
 * @{
 **/

/**
 * Operation codes.
 **/
typedef enum {
    /// synchronization barrier operation
    QBGfxProtocolOperation_barrier = 1u,
    /// upload operation: copying surface data from shared memory to private graphics memory
    QBGfxProtocolOperation_uploadSurface = 2u,
    /// download operation: copying surface data from private graphics memory to shared memory
    QBGfxProtocolOperation_downloadSurface = 3u,
    /// solid fill operation
    QBGfxProtocolOperation_fillColor = 8u,
    /// draw solid rectangle operation
    QBGfxProtocolOperation_blendColor = 9u,
    /// image copying operation
    QBGfxProtocolOperation_copyImage = 10u,
    /// image alpha-blending operation
    QBGfxProtocolOperation_blendImage = 11u,
    /// operation code used for protocol extensions
    QBGfxProtocolOperation_extension = 255u
} QBGfxProtocolOperation;

/**
 * @defgroup QBGfxProtocolBarrierOperation Barrier operation
 * @{
 *
 * Barrier operation enforces correct ordering of other operations. All operations
 * preceding the barrier must be finished before any of the operations succeeding
 * the barrier can be executed.
 *
 * Renderer that implements this protocol can notify the client when barrier
 * is reached. This notification should include the QBGfxProtocolBarrierOperation::cookie
 * value. Client can make use of such notification to wait for some operations
 * to be finished.
 **/

/**
 * Description of the barrier operation.
 **/
struct QBGfxProtocolBarrierOperation {
    /**
     * header containing code #QBGfxProtocolOperation_barrier
     * and length of this structure in bytes
     **/
    struct QBGfxProtocolOperationHeader header;
    /**
     * cookie value assigned to the barrier, renderer can sent it back
     * to the client when barrier is reached
     **/
    uint32_t cookie;
} __attribute__ ((aligned(8)));

/**
 * @}
 **/

/**
 * @defgroup QBGfxProtocolSurfaceOperations Surface operations
 * @{
 *
 * Surface operations are used in private graphics memory mode to copy surface
 * data between shared memory and graphics memory.
 **/

/**
 * Description of the surface upload operation.
 **/
struct QBGfxProtocolUploadSurfaceOperation {
    /**
     * header containing code #QBGfxProtocolOperation_uploadSurface
     * and length of this structure in bytes
     **/
    struct QBGfxProtocolOperationHeader header;
    /// source surface in shared memory
    struct QBGfxProtocolSurface srcSurface;
    /// region of the source surface to be uploaded
    struct QBGfxProtocolRect srcRect;
    /// identifier of the destination surface
    uint32_t destSurfaceID;
    /// X coordinate of the target region
    uint16_t destX;
    /// Y coordinate of the target region
    uint16_t destY;
} __attribute__ ((aligned(8)));

/**
 * Description of the surface download operation.
 **/
struct QBGfxProtocolDownloadSurfaceOperation {
    /**
     * header containing code #QBGfxProtocolOperation_downloadSurface
     * and length of this structure in bytes
     **/
    struct QBGfxProtocolOperationHeader header;
    /// identifier of the source surface
    uint32_t srcSurfaceID;
    /// region of the source surface to be uploaded
    struct QBGfxProtocolRect srcRect;
    /// target surface in shared memory
    struct QBGfxProtocolSurface destSurface;
    /// X coordinate of the target region
    uint16_t destX;
    /// Y coordinate of the target region
    uint16_t destY;
} __attribute__ ((aligned(8)));

/**
 * @}
 **/

/**
 * @defgroup QBGfxProtocol2DDrawingOperations 2D drawing operations
 * @{
 *
 * Accelerated 2D graphics operations.
 **/

/**
 * Description of the color drawing operation.
 **/
struct QBGfxProtocol2DColorOperation {
    /**
     * header containing code #QBGfxProtocolOperation_fillColor or
     * #QBGfxProtocolOperation_blendColor and length of this structure in bytes
     **/
    struct QBGfxProtocolOperationHeader header;
    /**
     * in color fill operation: color value to set target pixels to,
     * in color blend operation: color of the rectangle to draw over
     * the target area
     **/
    struct QBGfxProtocolColor srcColor;
    /**
     * in private graphics memory mode: identifier of the target surface,
     * otherwise must be set to @c 0
     **/
    uint32_t destSurfaceID;
    /**
     * in shared graphics memory mode: description of the target surface,
     * must be filled with @c 0 otherwise
     **/
    struct QBGfxProtocolSurface destSurface;
    /// area of the target surface to be drawn on
    struct QBGfxProtocolRect destRect;
} __attribute__ ((aligned(8)));

/**
 * Description of the image drawing operation.
 **/
struct QBGfxProtocol2DImageOperation {
    /**
     * header containing code #QBGfxProtocolOperation_copyImage or
     * #QBGfxProtocolOperation_blendImage and length of this structure in bytes
     **/
    struct QBGfxProtocolOperationHeader header;
    /**
     * for alpha-only source surfaces: color value of the entire surface,
     * for RGB source surfaces: global alpha value for image blend operation
     **/
    struct QBGfxProtocolColor srcColor;
    /**
     * in private graphics memory mode: identifier of the source surface,
     * otherwise must be set to @c 0
     **/
    uint32_t srcSurfaceID;
    /**
     * in shared graphics memory mode: description of the source surface,
     * must be filled with @c 0 otherwise
     **/
    struct QBGfxProtocolSurface srcSurface;
    /// area of the source surface to be drawn on destination surface
    struct QBGfxProtocolFPRect srcRect;
    /**
     * in private graphics memory mode: identifier of the target surface,
     * otherwise must be set to @c 0
     **/
    uint32_t destSurfaceID;
    /**
     * in shared graphics memory mode: description of the target surface,
     * must be filled with @c 0 otherwise
     **/
    struct QBGfxProtocolSurface destSurface;
    /// area of the target surface to be drawn on
    struct QBGfxProtocolRect destRect;
} __attribute__ ((aligned(8)));

/**
 * @}
 **/


/**
 * @}
 **/

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
