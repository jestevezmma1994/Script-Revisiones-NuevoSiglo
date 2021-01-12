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

#ifndef SV_RTSP_SAR_H
#define SV_RTSP_SAR_H

#include <SvPlayerKit/SvBuf.h>
#include <SvPlayerKit/SvBufMan.h>
#include <SvPlayerKit/SvMemCounter.h>
#include <stdbool.h>

struct epoint_t;

struct rtsp_sar;
struct rtsp_request;
struct rtsp_parser;

typedef int (sar_got_reponse_fun)(void* owner, struct rtsp_sar* sar, struct rtsp_parser* parser);

struct rtsp_sar_handlers
{
  void (*error) (void* owner, struct rtsp_sar* sar, int error);
  int  (*got_announce) (void* owner, struct rtsp_sar* sar, struct rtsp_parser* message);
  int  (*got_hb_response) (void* owner, struct rtsp_sar* sar, struct rtsp_parser* message);
  int  (*got_packet) (void* owner, struct rtsp_sar* sar, SvBuf sb);
  int  (*connected) (void* owner, struct rtsp_sar* sar);
};

struct rtsp_sar_params
{
  const struct rtsp_sar_handlers* handlers;
  void* owner;

  struct epoint_t* ep;
  bool  client_only;
  // debug only.
  const char* full_url;
};


struct rtsp_sar* rtsp_sar_create(const struct rtsp_sar_params* params);
void rtsp_sar_destroy(struct rtsp_sar* sar);
const char* rtsp_sar_get_session_id(struct rtsp_sar* sar);
void rtsp_sar_set_session_id(struct rtsp_sar* sar, const char* session_id);
void rtsp_sar_set_hb_timeout(struct rtsp_sar* sar, int timeout_sec);
void rtsp_sar_set_hb_message(struct rtsp_sar* sar, struct rtsp_request* rr, int with_response, int add_headers);
void rtsp_sar_set_teardown_message(struct rtsp_sar* sar, struct rtsp_request* rr);
void rtsp_sar_set_user_agent(struct rtsp_sar* sar, char* user_agent);

void rtsp_sar_set_name(struct rtsp_sar* sar, char* name);

void rtsp_sar_send(struct rtsp_sar* sar, struct rtsp_request* rr, sar_got_reponse_fun* response_fun);
void rtsp_sar_send_ex(struct rtsp_sar* sar, struct rtsp_request* rr, sar_got_reponse_fun* response_fun,
                      int expecting_response, int with_headers);

void rtsp_sar_move_session_ownership(struct rtsp_sar* sar_dest, struct rtsp_sar* sar_src);
void rtsp_sar_set_buf_man(struct rtsp_sar* sar, SvBufMan buf_man, SvMemCounter mem_counter);

/// in seconds
int rtsp_sar_get_hb_timeout(const struct rtsp_sar* sar);

#endif // #ifndef SV_RTSP_SAR_H
