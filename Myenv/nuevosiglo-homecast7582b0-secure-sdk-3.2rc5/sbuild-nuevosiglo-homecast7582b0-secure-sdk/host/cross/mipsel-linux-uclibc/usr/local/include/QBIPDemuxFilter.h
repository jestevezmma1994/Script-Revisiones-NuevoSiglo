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

#ifndef QB_IP_DEMUX_FILTER_H_
#define QB_IP_DEMUX_FILTER_H_

/**
 * @file QBIPDemuxFilter.h QBIPDemuxFilter class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>
#include <SvPlayerKit/SvBuf.h>
#include <SvPlayerManager/SvPlayerTask.h>
#include <dataformat/sv_data_format.h>

/**
 * @defgroup QBIPDemuxFilter class
 * @ingroup HAL
 * @{
 **/

/**
 * @brief It is pid filtering tool, which can be used when content is delivered by multicast.
 * Generally it can be used instead of QBTunerFilter in case of multicast transmition
 */
typedef struct QBIPDemuxFilter_s* QBIPDemuxFilter;

/**
 * @brief This method is called each time stream format has changed.
 * @param[in] owner instance of owner
 * @param[in] filter instance of QBIPDemuxFilter
 * @param[in] format stream format
 * @return pid number to be filtered
 */
typedef int (QBIPDemuxFilterSelectPidFun) (SvObject owner, const QBIPDemuxFilter filter, const struct svdataformat* format);

/**
 * @brief Object which implements that interface can notified each time new data is filtered
 */
typedef struct QBIPDemuxFilterListener_s {
    /**
     * @brief Notifies listener that it has to flush internal data.
     * @param[in] self listener handler
     */
    void (*flush) (SvObject self_, QBIPDemuxFilter filter);
    /**
     * @brief It provides filtered data chunk
     * @param self[in] listener handler
     * @param filter instance of QBIPDemuxFilter
     * @param sb data - if inplace is set to false, listener has to release data internally.
     */
    void (*data) (SvObject self_, QBIPDemuxFilter filter, SvBuf sb);
}*QBIPDemuxFilterListener;

/**
 * @brief Set of setup parameters of QBIPDemuxFilter
 */
struct QBIPDemuxFilterSetupParams {
    int pid; ///<! Pid which is requested to be filtered
    QBIPDemuxFilterSelectPidFun* selectPidFun; ///<! If not NULL, function is called each time new format is recived
    SvObject owner; ///<! Owner parameter for QBIPDemuxFilterSelectPidFun function
};

/**
 * @brief Gets SvInterface instance of QBIPDemuxFilterListener
 * @return SvInterface instance of QBIPDemuxFilterListener
 */
SvInterface QBIPDemuxFilterListener_getInterface(void);

/**
 * @brief It creates instance of QBIPDemuxFilter
 * @param[in] task playerTask or pvrTask
 * @return
 */
QBIPDemuxFilter QBIPDemuxFilterCreate(SvObject task);

/**
 * @brief Adds listener. Listener is notified about new filtered data chunks and need of data flush.
 * @param[in] self instance of QBIPDemuxFilter
 * @param[in] listener instance of object which implements QBIPDemuxFilterListener interface
 */
void QBIPDemuxFilterAddListener(QBIPDemuxFilter self, SvObject listener);

/**
 * @brief Removes listener
 * @param[in] self instance of QBIPDemuxFilter
 * @param[in] listener instance of object which implements QBIPDemuxFilterListener interface
 */
void QBIPDemuxFilterRemoveListener(QBIPDemuxFilter self, SvObject listener);

/**
 * @brief It starts filtering process.
 * @param[in] self instance of QBIPDemuxFilter
 * @param[in] params setup params see QBIPDemuxFilterSetupParams for details
 */
void QBIPDemuxFilterStart(QBIPDemuxFilter self, const struct QBIPDemuxFilterSetupParams* params);

/**
 * @brief It stops filtering process.
 * @param[in] self instance of QBIPDemuxFilter
 */
void QBIPDemuxFilterStop(QBIPDemuxFilter self);

/**
 * @brief Sets if data should be reported in place. In case of setting to false , listener is obligated to free data.
 * @param[in] self instance of QBIPDemuxFilter
 * @param[in] inplace if set to false, listener is obligated to free data received by callback. In other case not.
 */
void QBIPDemuxFilterReportDataInplace(QBIPDemuxFilter self, bool inplace);

/**
 * @brief Gets currently filtered pid.
 * @param[in] self instance of QBIPDemuxFilter
 * @return currently filtered pid
 */
int QBIPDemuxFilterGetPid(QBIPDemuxFilter self);

/**
* @}
**/


#endif //QB_IP_DEMUX_FILTER_H_
