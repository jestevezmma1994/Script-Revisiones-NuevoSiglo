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

#ifndef SV_SENSE_PLAYER_ERRORCODES_H_
#define SV_SENSE_PLAYER_ERRORCODES_H_

/**
 * @file error_codes.h old style error codes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvLog.h>
#include <SvCore/SvLogColors.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup errlib errlib: old style error codes
 * @ingroup Utils
 * @{
 *
 * errlib library defines SV_ERR_* error codes reported by media player components.
 **/

/**
 * errlib error domain.
 **/
#define SvErrDomain "com.sentivision.SvErr"

/**
 * @defgroup SvErrErrorCodes errlib error codes
 * @{
 **/

//#define SV_ERR_OK                0 // just use 0 directly

#define SV_ERR_FINALIZE          -100 // an asynchronous command has been completely handled.
#define SV_ERR_STATE_CHANGED     -101 // called function has already changed state of our FSA.
#define SV_ERR_PREFILL_STARTED   -102
#define SV_ERR_PREFILL_FINISHED  -103
#define SV_ERR_GOT_MESSAGE       -104
#define SV_ERR_EOF_OUTPUT        -105
#define SV_ERR_EOF_BUFFERED      -106
#define SV_ERR_EOF_INPUT         -107
#define SV_ERR_EOF_INPUT_IMMINENT -108
#define SV_ERR_OUTPUT_READY      -109
#define SV_ERR_INPUT_READY       -110
#define SV_ERR_REENCRYPTION_FAILED -111

#define SV_ERR_WOULDBLOCK        -201 // operation could not be completed immediately. call again later.
#define SV_ERR_AGAIN             -202 // operation needs to be restarted.
#define SV_ERR_TIMEBARRIER       -203 // current turn's time has been exceeded. need to return control to upper layer.

#define SV_ERR_INVALID           -300
#define SV_ERR_TIMEOUT           -301
#define SV_ERR_INCOMPATIBLE      -302
#define SV_ERR_EOF_OUT_OF_RANGE  -303
#define SV_ERR_NOT_FOUND         -304
#define SV_ERR_BAD_FORMAT        -305

#define SV_ERR_FATAL             -1000
#define SV_ERR_BUFFEROVERRUN     -1001
#define SV_ERR_OOM               -1002
#define SV_ERR_RESOURCE_LIMIT    -1003
#define SV_ERR_NO_SPACE          -1004
#define SV_ERR_ESTIMATE_NO_SPACE -1005
#define SV_ERR_BROADCASTER_LIMITATION -1006

#define SV_ERR_INIT_DECODER      -1101
#define SV_ERR_DECODING_FAILURE  -1102
#define SV_ERR_UNSUPPORTED_VIDEO_CODEC  -1103
#define SV_ERR_UNSUPPORTED_AUDIO_CODEC  -1104
#define SV_ERR_DECODER_DEAD      -1105

#define SV_ERR_DRM_INIT_FAILURE     -1201
#define SV_ERR_DRM_CLOSE_FAILURE    -1202
#define SV_ERR_NO_DRM               -1203
#define SV_ERR_DRM_CERT_REVOKED     -1204
#define SV_ERR_DRM_KEY_NOT_OBTAINED -1205
#define SV_ERR_DRM_CONN_FAILED      -1206
#define SV_ERR_DRM_CPC_CERT_INVALID -1207
#define SV_ERR_DRM_NO_LICENSE       -1208
#define SV_ERR_DRM_LIC_EXPIRED      -1209
#define SV_ERR_DRM_LIC_NOT_YET_VALID -1210
#define SV_ERR_DRM_OTHER            -1211

#define SV_ERR_SERVER_CONN_FAILED   -1301
#define SV_ERR_BAD_SERVER_RESPONSE  -1302
#define SV_ERR_REQUEST_OUT_OF_RANGE -1303
#define SV_ERR_UNEXPECTED_SERVER_RESPONSE -1304
#define SV_ERR_SERVER_CONN_CLOSED   -1305
#define SV_ERR_SERVER_STREAMING_FAILED -1306
#define SV_ERR_SERVER_INTERNAL_ERROR -1307
#define SV_ERR_SERVER_PERM           -1308
#define SV_ERR_SERVER_FILE_NOT_FOUND -1309
#define SV_ERR_SERVER_PRECONDITION_FAILED -1310
#define SV_ERR_GATEWAY_TIMEOUT -1311

#define SV_ERR_SERVER_MEDIA_FILE_NOT_FOUND       1320
#define SV_ERR_SERVER_MEDIA_LIST_UNAVAILABLE    -1321
#define SV_ERR_ALL_VARIANTS_BROKEN              -1322
#define SV_ERR_SERVER_DESYNC_TOO_HIGH           -1323

#define SV_ERR_UNSUPPORTED_FORMAT       -1400
#define SV_ERR_UNSUPPORTED_PROTOCOL     -1401
#define SV_ERR_BAD_URL                  -1402
#define SV_ERR_UNSUPPORTED_ENCRYPTION   -1403

#define SV_ERR_CPC_XML_SYNTAX_ERROR -1500
#define SV_ERR_CPC_SYNTAX_ERROR     -1501
#define SV_ERR_ASX_XML_SYNTAX_ERROR -1502
#define SV_ERR_ASX_SYNTAX_ERROR     -1503

#define SV_ERR_SYSTEM               -2000

#define SV_ERR_WAS_SHUTDOWN         -6000

#define SV_ERR_INTERNAL             -10000
#define SV_ERR_NOT_IMPLEMENTED      -10001
#define SV_ERR_INTERNAL_FRAME_ORDER -10002 // wrong order of frames in stream
#define SV_ERR_INDUCED              -10003

#define SV_ERR_DVB_FATAL            -20000
/**
 * @}
 **/


/** @cond */

#define ASSERT_ERRORS_CAT_IDS_2( _id1_, _id2_ ) _id1_##_id2_
#define ASSERT_ERRORS_CAT_IDS( _id1_, _id2_ ) ASSERT_ERRORS_CAT_IDS_2(_id1_,_id2_)

#define ASSERT_ERRORS( _var_, _allow_blocking_, _allow_fatals_, ... ) do { \
    static const int _tab_[] = { __VA_ARGS__ }; \
    static const int _elems_ = sizeof(_tab_)/sizeof(int); \
    if ((_allow_fatals_) && (_var_) <= SV_ERR_FATAL) { \
    } else if ((_allow_blocking_) && ((_var_) == SV_ERR_WOULDBLOCK || (_var_) == SV_ERR_TIMEBARRIER)) { \
    } else { \
      int _i_; \
      for (_i_=0; _i_<_elems_; ++_i_) { \
        if ((_var_) == _tab_[_i_]) \
          goto ASSERT_ERRORS_CAT_IDS(_ASSERT_ERRORS_passed_, __LINE__); \
      }; \
      SvLogError(COLBEG() \
                 "ASSERT_ERRORS :: %s [%s, %d]\r\n" \
                 "   received : [%d, %s]" \
                 COLEND_COL(red), __FUNCTION__, __FILE__, __LINE__, \
                 (_var_), sv_err_to_string((_var_)) \
                ); \
    }; \
    ASSERT_ERRORS_CAT_IDS(_ASSERT_ERRORS_passed_, __LINE__): \
    if (1) {}; \
  } while(0)


#define NOT_IMPLEMENTED__ret_1() do { return SV_ERR_NOT_IMPLEMENTED; } while (0)
#define NOT_IMPLEMENTED__ret_0() do { } while (0)
#define NOT_IMPLEMENTED__(ret) do { \
    SvLogError(COLBEG() "Function [%s] is not yet implemented [%s, %d]" COLEND_COL(red), \
               __FUNCTION__, __FILE__, __LINE__); \
    NOT_IMPLEMENTED__ret_##ret(); \
  } while (0)
#define NOT_IMPLEMENTED()        NOT_IMPLEMENTED__(0)
#define NOT_IMPLEMENTED_return() NOT_IMPLEMENTED__(1)

#define SV_ERR_SOURCE( _name_ ) (SV_ERR_##_name_)

/** @endcond */


/**
 * Get textual description of an errlib error code.
 *
 * @param[in] err error code
 * @return error code description or 'Unknown error.'
 **/
extern const char* sv_err_to_string(int err);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
