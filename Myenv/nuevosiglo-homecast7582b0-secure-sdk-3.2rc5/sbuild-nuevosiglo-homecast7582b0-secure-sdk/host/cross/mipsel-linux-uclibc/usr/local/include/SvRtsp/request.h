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

#ifndef SV_RTSP_REQUEST_H
#define SV_RTSP_REQUEST_H

struct rtsp_request;

const char* rtsp_req_get_head(const struct rtsp_request* req);
int rtsp_req_get_head_len(const struct rtsp_request* req);

const char* rtsp_req_get_body(const struct rtsp_request* req);
int rtsp_req_get_body_len(const struct rtsp_request* req);

struct rtsp_request* rtsp_req_create_pure(void);
struct rtsp_request* rtsp_req_create_with_title(const char* title);
struct rtsp_request* rtsp_req_create(const char* method, const char* url, const char* stream_id);
void rtsp_req_free(struct rtsp_request* req);
struct rtsp_request* rtsp_req_copy(const struct rtsp_request* req);

void rtsp_req_add_header(struct rtsp_request* req, const char* name, const char* value);
void rtsp_req_add_int_header(struct rtsp_request* req, const char* name, int value);
void rtsp_req_add_print_header(struct rtsp_request* req, const char* name, const char* fmt, ...);
void rtsp_req_remove_header(struct rtsp_request *rr, const char *name);

void rtsp_req_add_body(struct rtsp_request* req, char* body, int len);


const char* rtsp_req_find_header(const struct rtsp_request* req, const char* header_name);

#endif // #ifndef SV_RTSP_REQUEST_H
