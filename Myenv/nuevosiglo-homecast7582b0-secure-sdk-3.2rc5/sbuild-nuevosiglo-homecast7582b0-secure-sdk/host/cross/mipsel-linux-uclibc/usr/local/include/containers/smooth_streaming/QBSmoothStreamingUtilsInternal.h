/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSMOOTHSTREAMINGUTILSINTERNAL_H_
#define QBSMOOTHSTREAMINGUTILSINTERNAL_H_

/**
 * @file  QBSmoothStreamingUtilsInternal.h
 * @brief Utility methods operating on Smooth Streaming Media object private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvPlayerKit/SvBuf.h>

/**
 * @addtogroup QBSmoothStreamingUtils
 * @{
 **/

/**
 * This method stores in input array at given index size of NAL unit on given number of bytes.
 *
 * @param[in,out] inputData         input data
 * @param[in] index                 position in input data
 * @param[in] nalUnitSize           size of NAL unit
 * @param[in] length                number of bytes for stored size of NAL unit
 **/
SvHidden void
setNALUnitSize(unsigned char *inputData, unsigned int *index, unsigned int nalUnitSize, unsigned int length);

/**
 * This method parses base16-encoded string which contains Sequence Parameter Set (SPS) and (Picture Parameter Set) PPS, in the following form:
 * [start code] [SPS] [start code] [PPS], where [start code] is the follwoing four bytes: 0x00 0x00 0x00 0x01.
 * SPS and PPS fields are described in ISO/IEC 14496-10.
 *
 * @param[in] inputData             input data
 * @param[in] inputDataLength       input data length
 * @param[out] spsNalUnit_          SPS NAL unit
 * @param[out] spsNalUnitLength_    SPS NAL unit length
 * @param[out] ppsNalUnit_          PPS NAL unit
 * @param[out] ppsNalUnitLength_    PPS NAL unit length
 * @param[out] errorOut             error info
 **/
SvHidden void
createSpsAndPpsNalUnits(const char *inputData, unsigned int inputDataLength, unsigned char **spsNalUnit_, unsigned int *spsNalUnitLength_, unsigned char **ppsNalUnit_, unsigned int *ppsNalUnitLength_, SvErrorInfo *errorOut);

/**
 * This method parses input data which is base16-encoded string and creates char string.
 * Input data contains SPS and PPS NAL units.
 * Parsed H264 PPS and SPS units are stored in AVCC format which is described in ISO/IEC 14496-15 (subchapter 5.2.4.1).
 *
 * @param[in] inputData             input data
 * @param[in] inputDataLength       input data length
 * @param[in] nalUnitLength         NAL unit length
 * @param[out] outputDataLength     output data length
 * @param[out] errorOut             error info
 * @return                          created H264 extended format data, otherwise @c NULL in case of error
 **/
SvHidden unsigned char *
createH264PrivateData(const char *inputData, unsigned int inputDataLength, unsigned int nalUnitLength, unsigned int *outputDataLength, SvErrorInfo *errorOut);

/**
 * This method parses input data which is base16-encoded string and creates char string.
 * Input data is AudioSpecificConfig which is described in ISO/IEC-14496-3 (subchapter 1.6.2.1).
 * The length of AudioSpecificConfig data is usually 2 bytes, but for HE-AAC with SBR/PS it
 * can be more than 2 bytes. Currently in QBDecoderBase is accepted only AudioSpecificConfig with 2 bytes length.
 * According to MS documentation input data can be WAVEFORMATEX structure followed by AudioSpecificConfig data.
 *
 * @param[in] inputData             input data
 * @param[in] inputDataLength       input data length
 * @param[out] outputDataLength     output data length
 * @param[out] errorOut             error info
 * @return                          created AAC extended format data, otherwise @c NULL in case of error
 **/
SvHidden unsigned char *
createAACPrivateData(const char *inputData, unsigned int inputDataLength, unsigned int *outputDataLength, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBSMOOTHSTREAMINGUTILSINTERNAL_H_ */
