/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SV_RTSP_PARSER_H
#define SV_RTSP_PARSER_H

#include <SvPlayerKit/SvBuf.h>
#include <SvCore/SvLog.h>
#include <error_codes.h>

#define RTSP_PARSER_MAX_LINES  20

struct rtsp_parser
{
  char* begin; // we own this.
  char* end;
  // pointers to the above buffer.
  char* keys[RTSP_PARSER_MAX_LINES];
  const char* values[RTSP_PARSER_MAX_LINES];
  int line_cnt;
  int status_code;

  SvBuf sb_data;
};

void rtsp_parser_destroy(struct rtsp_parser* parser);

int rtsp_parser_set_response(struct rtsp_parser* parser, char* data, bool with_header, bool parse_header);
void rtsp_parser_set_data(struct rtsp_parser* parser, SvBuf sb);
void rtsp_parser_clear(struct rtsp_parser* parser);

int rtsp_parser_create_from_buffer(const char* data_orig, int data_len,
                                   bool with_header, bool parse_header,
                                   struct rtsp_parser** parser_out);

const char* rtsp_parser_get_str_header(const struct rtsp_parser* parser, const char* field);
const char* rtsp_parser_get_str_header_suffix(const struct rtsp_parser* parser, const char* suffix);
int rtsp_parser_get_int_header(const struct rtsp_parser* parser, const char* field, int* res);
int rtsp_parser_get_double_header(const struct rtsp_parser* parser, const char* field, double* res);

int rtsp_parser_get_status(const struct rtsp_parser* parser);
const char* rtsp_parser_get_description(const struct rtsp_parser* parser);

void rtsp_parser_dump(const struct rtsp_parser* parser, int printable_data);

int rtsp_find_str_field(const char* str, const char* name, const char** start_out);

/** @param format any format that could sscanf to int : "%d", "%x", "%u". */
int rtsp_find_int_field(const char* str, const char* name, const char* format, int* val_out);
int rtsp_find_double_field(const char* str, const char* name, const char* format, double* val_out);

#define RTSP_PARSER_FORMAT_TO_FUN_d  rtsp_find_int_field
#define RTSP_PARSER_FORMAT_TO_FUN_x  rtsp_find_int_field
#define RTSP_PARSER_FORMAT_TO_FUN_u  rtsp_find_int_field
#define RTSP_PARSER_FORMAT_TO_FUN_f  rtsp_find_double_field
#define RTSP_PARSER_FORMAT_TO_FUN_lf rtsp_find_double_field

#define RTSP_PARSER_GET_FIELD_(_str_, _field_name_, _format_, _var_, _print_err_) do { \
    res = RTSP_PARSER_FORMAT_TO_FUN_##_format_ ((_str_), (_field_name_), "%"#_format_, &(_var_)); \
    if (res < 0) { \
      if ((_print_err_)) \
        SvLogError(COLBEG() \
            "RTSP : Unable to %s field \"%s\" in \"%s\" header in %s response." COLEND_COL(red), \
            (res == SV_ERR_NOT_FOUND) ? "find" : "parse", \
            (_field_name_), header_name, op_name); \
      res = SV_ERR_BAD_SERVER_RESPONSE; \
    } else { \
      /*SvLogDebug(COLBEG() "Field %s parsed as [" "%"#_format_ "]." COLEND_COL(cyan), \
                 (_field_name_), _var_);*/ \
    }; \
  } while (0)

#define RTSP_PARSER_GET_FIELD(_str_, _field_name_, _format_, _var_) do { \
    RTSP_PARSER_GET_FIELD_(_str_, _field_name_, _format_, _var_, 1); \
    if (res < 0) \
      goto out; \
  } while (0)

#define RTSP_PARSER_GET_FIELD_2(_str_, _field_name_1_, _try_field_2_, _field_name_2_, _format_, _var_) do { \
    int print_errors = ((_try_field_2_) == 0); \
    RTSP_PARSER_GET_FIELD_(_str_, _field_name_1_, _format_, _var_, print_errors); \
    if (res < 0) { \
      if (res == SV_ERR_NOT_FOUND && (_try_field_2_)) \
        RTSP_PARSER_GET_FIELD_(_str_, _field_name_2_, _format_, _var_, 1); \
    }; \
  } while (0)

#define RTSP_PARSER_CHECK_GET_(_format_, _var_) do { \
    if (res < 0) { \
      SvLogError(COLBEG() "RTSP : Unable to %s header \"%s\" in %s response." COLEND_COL(red), \
           (res == SV_ERR_NOT_FOUND) ? "find" : "parse", \
           header_name, op_name); \
      res = SV_ERR_BAD_SERVER_RESPONSE; \
    } else { \
      /*SvLogDebug(COLBEG() "Header %s parsed as [" _format_ "]." COLEND_COL(cyan), \
                   header_name, _var_);*/ \
    }; \
  } while (0)

#define RTSP_PARSER_CHECK_GET(_format_, _var_) do { \
    RTSP_PARSER_CHECK_GET_(_format_, _var_); \
    if (res < 0) \
      goto out; \
  } while (0)

int rtsp_status_to_sv_err(int status);

#endif // #indef SV_RTSP_PARSER_H
