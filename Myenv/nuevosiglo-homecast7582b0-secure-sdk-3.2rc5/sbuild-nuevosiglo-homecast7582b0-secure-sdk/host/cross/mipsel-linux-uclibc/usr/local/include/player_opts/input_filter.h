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

#ifndef PLAYER_OPTS_INPUT_FILTER_H
#define PLAYER_OPTS_INPUT_FILTER_H

/**
 * @file input_filter.h input filter listener declaration
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerKit/SvMemCounter.h>
#include <dataformat/sv_data_format.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>

/**
 * @defgroup InputFilter input filter player opt command and interfaces declarations
 * @ingroup autoplayer
 * @{
 **/
/**
 * @brief Add data listener opt command
 */
#define PLAYER_OPT_INPUT_FILTER_ADD_DATA_LISTENER "input_filter:add_data_listener"

/**
 * @brief Remove data listener opt command
 */
#define PLAYER_OPT_INPUT_FILTER_REMOVE_DATA_LISTENER "input_filter:remove_data_listener"

/**
 * @brief Add format listener opt command
 */
#define PLAYER_OPT_INPUT_FILTER_ADD_FORMAT_LISTENER "input_filter:add_format_listener"

/**
 * @brief Remove format listener opt command
 */
#define PLAYER_OPT_INPUT_FILTER_REMOVE_FORMAT_LISTENER "input_filter:remove_format_listener"

/**
 * @brief Object which implements that interface can receive data format each time it is updated
 */
typedef struct QBInputFilterFormatListener_ {
    /**
     * @brief It is called each time data format is updated
     * @param[in] self_ listener instance
     * @param[in] format updated data format
     */
    void (*formatChanged)(SvObject self_, const struct svdataformat *formatChanged);
} *QBInputFilterFormatListener;

/**
 * @brief Object which implements that interface will receive data, from selected pid.
 */
typedef struct QBInputFilterDataReceiver_ {
    /**
     * @brief Called when we data is ready
     * @param[in] self_ listener instance
     * @param[in] packets received data
     * @param[in] packetCnt number of ts packets
     * @param[in] memCounter memory counter
     */
    void (*pushData)(SvObject self_, const uint8_t *packets, uint32_t packetCnt, SvMemCounter memCounter);
    /**
     * @brief Called when data needs to be flushed
     * @param[in] self_ listener instance
     */
    void (*flush)(SvObject self_);
    /**
     * @brief Gets pid to be filtered
     * @param[in] self_ listener instance
     * @return pid to be filtered
     */
    int (*getPid)(SvObject self_);
} *QBInputFilterDataReceiver;

/**
 * @brief Gets SvInterface instance of QBInputFilterFormatListener
 */
SvExport SvInterface QBInputFilterFormatListener_getInterface(void);

/**
 * @brief Gets SvInterface instance of QBInputFilterDataListener
 */
SvExport SvInterface QBInputFilterDataReceiver_getInterface(void);

/**
* @}
*/

#endif // #ifndef PLAYER_OPTS_INPUT_FILTER_H
