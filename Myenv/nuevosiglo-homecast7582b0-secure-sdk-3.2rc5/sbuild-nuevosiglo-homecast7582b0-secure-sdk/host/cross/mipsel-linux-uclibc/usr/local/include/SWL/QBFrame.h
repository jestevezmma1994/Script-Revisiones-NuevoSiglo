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

#ifndef SWL_QB_FRAME_H_
#define SWL_QB_FRAME_H_

/**
 * @file SMP/SWL/swl/QBFrame.h Frame widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>


/**
 * @defgroup QBFrame Frame widget
 * @ingroup SWL_widgets
 * @{
 *
 * Frame widget displays non-uniformly scaled bitmap, typically used as
 * a background for other widgets.
 **/

/**
 * Frame parameters
 **/
typedef struct QBFrameParameters_ {
    SvBitmap bitmap;                    ///< bitmap handle
    unsigned int width;                 ///< widget width
    unsigned int height;                ///< widget height

    unsigned int minWidth;              ///< minimal width
    unsigned int maxWidth;              ///< maximal width
    unsigned int minHeight;             ///< minimal height
    unsigned int maxHeight;             ///< maximal height

    unsigned int cutXPosition;          ///< x offset of 1px rect cut for the resized middle frame
    unsigned int cutYPosition;          ///< y offset of 1px rect cut for the resized middle frame
} QBFrameParameters;

/**
 * Frame extended parameters
 *
 * @brief Extended parameters are used to create frame with middle part created from scaled
 * part of background bitmap determined by 2 vertical and 2 horizontal cut lines
 **/
typedef struct QBFrameExtendedParameters_ {
    SvBitmap bitmap;                    ///< bitmap handle
    unsigned int width;                 ///< widget width
    unsigned int height;                ///< widget height

    unsigned int minWidth;              ///< minimal width
    unsigned int maxWidth;              ///< maximal width
    unsigned int minHeight;             ///< minimal height
    unsigned int maxHeight;             ///< maximal height

    unsigned int cutXMinPosition;       ///< x offset of first rect cut for the resized middle frame
    unsigned int cutXMaxPosition;       ///< x offset of second rect cut for the resized middle frame
    unsigned int cutYMinPosition;       ///< y offset of first rect cut for the resized middle frame
    unsigned int cutYMaxPosition;       ///< y offset of second rect cut for the resized middle frame
} QBFrameExtendedParameters;

/**
 * @class QBFrameDescription
 * @extends SvObject
 **/
typedef struct QBFrameDescription_ {
    /// super class
    struct SvObject_ super_;
    /// frame parameters
    QBFrameExtendedParameters data;
    /// frame name
    char* name;
} QBFrameDescription;

/**
 * Create frame description object with parameters
 * from the Settings Manager.
 *
 * @see See QBFrameCreateFromSM() for the list of parameters.
 *
 * @memberof QBFrameDescription
 *
 * @param[in] name      widget name
 * @return              created description, @c NULL in case of error
 **/
extern QBFrameDescription *
QBFrameDescriptionCreateFromSM(const char *name);

/**
 * Create frame description object with parameters
 * from the ratio settings.
 *
 * @see See QBFrameCreateFromSM() for the list of parameters.
 *
 * @memberof QBFrameDescription
 *
 * @param[in] name          widget name
 * @param[in] parentWidth   parent width
 * @param[in] parentHeight  parent height
 * @return              created description, @c NULL in case of error
 **/
extern QBFrameDescription *
QBFrameDescriptionCreateFromRatio(const char *name, unsigned int parentWidth, unsigned int parentHeight);

/**
 * Create frame description object with parameters
 *
 * @memberof QBFrameDescription
 *
 * @param[in] name      widget name
 * @param[in] params    frame parameters
 * @return              created description, @c NULL in case of error
 **/
extern QBFrameDescription *
QBFrameDescriptionCreateWithParameters(const char *name, const QBFrameParameters *params);

/**
 * Create frame widget using given parameters.
 *
 * @param[in] app           CUIT application handle
 * @param[in] params        set of parameters describing widget
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBFrameCreate(SvApplication app, const QBFrameParameters *params);

/**
 * Create frame widget using given extended parameters.
 *
 * @param[in] app           CUIT application handle
 * @param[in] params        set of parameters describing widget
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBFrameCreateFromExtendedParams(SvApplication app, const QBFrameExtendedParameters *params);

/**
 * Create frame widget using settings from the Settings Manager.
 *
 * This function creates frame widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 *  - bg: bitmap, optional,
 *  - width, height: integer (required if bg parameter is not present),
 *  - minWidth, minHeight: integer, optional (default: 0),
 *  - maxWidth, maxHeight: integer, optional,
 *  - cutX: integer, optional (default: width / 2),
 *  - cutY: integer, optional (default: height / 2),
 *  - cutXMargin, cutYMargin: integer, optional, define the X/Y cut margin from both sides.
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    widget name
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBFrameCreateFromSM(SvApplication app, const char *widgetName);

/**
 * Create frame widget using parameters from description object.
 *
 * @param[in] app           CUIT application handle
 * @param[in] desc          frame description handle
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBFrameCreateWithDescription(SvApplication, const QBFrameDescription *desc);

/**
 * Set bitmap of the frame.
 *
 * @param[in] w             frame widget handle
 * @param[in] params        new widget params
 **/
extern void
QBFrameSetBitmap(SvWidget w, const QBFrameParameters *params);

/**
 * Set bitmap of the frame.
 *
 * @param[in] w             frame widget handle
 * @param[in] params        new widget params
 **/
extern void
QBFrameSetBitmapFromExtendedParams(SvWidget w, const QBFrameExtendedParameters *params);

/**
 * Set width of the frame.
 *
 * @param[in] w             frame widget handle
 * @param[in] width         new width in pixels
 **/
extern void
QBFrameSetWidth(SvWidget w, unsigned int width);

/**
 * Set height of the frame.
 *
 * @param[in] w             frame widget handle
 * @param[in] height        new height in pixels
 **/
extern void
QBFrameSetHeight(SvWidget w, unsigned int height);

/**
 * Set dimensions (both width and height) of the frame.
 *
 * @param[in] w             frame widget handle
 * @param[in] width         new width in pixels
 * @param[in] height        new height in pixels
 **/
extern void
QBFrameSetSize(SvWidget w, unsigned int width, unsigned int height);


/**
 * @cond
 **/
// compatibility macros, remove when code that uses them is fixed
#define QBFrameConstructData QBFrameParameters
#define QBFrameConstructor QBFrameDescription
#define QBFrameConstructorFromSM QBFrameDescriptionCreateFromSM
#define QBFrameCreateFromConstructor QBFrameCreateWithDescription
/**
 * @endcond
 **/

/**
 * @}
 **/


#endif /* SWL_QB_FRAME_H_ */
