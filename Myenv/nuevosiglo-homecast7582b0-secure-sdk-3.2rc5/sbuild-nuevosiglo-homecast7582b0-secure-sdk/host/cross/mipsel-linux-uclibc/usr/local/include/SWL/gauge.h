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

#ifndef SWL_GAUGE_H_
#define SWL_GAUGE_H_

/**
 * @file SMP/SWL/swl/gauge.h Gauge widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup SvGauge Gauge widget
 * @ingroup SWL_widgets
 * @{
 *
 * Gauge widget displays the progress of some activity in a form of progress bar.
 **/

/**
 * Gauge orientation types.
 **/
typedef enum {
    SV_GAUGE_HORIZONTAL = 0,    ///< horizontal gauge
    SV_GAUGE_VERTICAL,          ///< vertical gauge
} SvGaugeOrientation;

/**
 * Gauge styles.
 **/
typedef enum {
    SV_GAUGE_STYLE_BASIC = 0,  ///< basic gauge style
    SV_GAUGE_STYLE_COMPLEX,    ///< complex style
} SvGaugeStyle;

/**
 * Create gauge widget using settings from the Settings Manager.
 *
 * This function creates gauge widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 *  - width, height : integer, required,
 *  - bg: bitmap, optional.
 *  - bar: bitmap, required.
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name
 * @param[in] orientation  gauge orientation, see SvGaugeOrientation
 * @param[in] style        gauge style, see SvGaugeStyle
 * @param[in] minValue     minimum value accessible in the gauge
 * @param[in] maxValue     maximum value accessible in the gauge
 * @param[in] initialValue initial value of the gauge
 * @return                 created widget, @c NULL in case of error
 **/
extern SvWidget
svGaugeNewFromSM(SvApplication app, const char *widgetName,
                 SvGaugeOrientation orientation, SvGaugeStyle style,
                 int minValue, int maxValue, int initialValue);

/**
 * Create gauge widget using ratio settings from the Settings Manager.
 *
 * This function creates gauge widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 *  - widthRatio, heightRatio : double, required,
 *  - bg: bitmap, optional.
 *  - bar: bitmap, required.
 *
 * @param[in] app          CUIT application handle
 * @param[in] parentWidth  parent width
 * @param[in] parentHeight parent height
 * @param[in] widgetName   widget name
 * @param[in] orientation  gauge orientation, see SvGaugeOrientation
 * @param[in] style        gauge style, see SvGaugeStyle
 * @param[in] minValue     minimum value accessible in the gauge
 * @param[in] maxValue     maximum value accessible in the gauge
 * @param[in] initialValue initial value of the gauge
 * @return                 created widget, @c NULL in case of error
 **/
extern SvWidget
svGaugeNewFromRatio(SvApplication app,
                    unsigned int parentWidth,
                    unsigned int parentHeight,
                    const char *widgetName,
                    SvGaugeOrientation orientation,
                    SvGaugeStyle style,
                    int minValue,
                    int maxValue,
                    int initialValue);

/**
 * Set the gauge value.
 *
 * @param[in] w             gauge widget handle
 * @param[in] v             new value
 **/
extern void
svGaugeSetValue(SvWidget w, double v);

/**
 * Set gauge interval, in contrast to svGaugeSetValue(), it allows to
 * display values not starting at 0, but at any chosen value.
 * @param[in] w             guage widget handle
 * @param[in] v1            interval range start
 * @param[in] v2            interval range end
 **/
extern void
svGaugeSetInterval(SvWidget w, double v1, double v2);

/**
 * Set the shift bar start value, where the end value is assumed
 * to be equal to the main gauge value set by SetValue method.
 *
 * @param[in] w             gauge widget handle
 * @param[in] v             new value
 **/
extern void
svGaugeSetShiftStartValue(SvWidget w, double v);

/**
 * Set the current shift bar end value.
 *
 * @param[in] w             gauge widget handle
 * @param[in] v             new value
 **/
extern void
svGaugeSetShiftValue(SvWidget w, double v);

/**
 * Set shift bar interval - start and end value.
 * @param[in] w             guage widget handle
 * @param[in] v1            interval range start
 * @param[in] v2            interval range end
 **/
extern void
svGaugeSetShiftInterval(SvWidget w, double v1, double v2);

/**
 * Set the indicator position value.
 *
 * @param[in] w             gauge widget handle
 * @param[in] v             new value
 **/
extern void
svGaugeSetIndicatorValue(SvWidget w, double v);

/**
 * Hides or shows indicator depending on the hidden flag
 *
 * @param[in] w             gauge widget handle
 * @param[in] hidden        flag is indicator hidden
 **/
extern void
svGaugeIndicatorSetHidden(SvWidget w, bool hidden);

/**
 * Set the gauge bounds.
 *
 * @param[in] w             gauge widget handle
 * @param[in] min_value     new minimum value
 * @param[in] max_value     new maximum value
 **/
extern void
svGaugeSetBounds(SvWidget w, double min_value, double max_value);

/**
 * Set gauge indicator widget.
 *
 * @param[in] w             gauge widget handle
 * @param[in] indicator     new indicator widget handle
 */
extern void
svGaugeSetIndicator(SvWidget w, SvWidget indicator);

/**
 * Get gauge indicator widget.
 *
 * @param[in] w            gauge widget handle
 * @return                 currently used indicator widget handle
 */
extern SvWidget
svGaugeGetIndicator(SvWidget w);


/**
 * @}
 **/

#endif
