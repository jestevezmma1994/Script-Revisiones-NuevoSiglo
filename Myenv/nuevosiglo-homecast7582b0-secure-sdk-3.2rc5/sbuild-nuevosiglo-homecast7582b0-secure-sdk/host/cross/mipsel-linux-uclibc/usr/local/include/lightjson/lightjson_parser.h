/**************************************************************************
 * lightjson: simple streaming JSON parser/writer
 * Copyright (C) 2012 Tomasz Lukaszewski <lupus@cubiware.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************/

#ifndef LIGHTJSON_PARSER_H_
#define LIGHTJSON_PARSER_H_

#include <stddef.h>
#include <stdbool.h>
#include "lightjson_types.h"

#ifdef __cplusplus
extern "C" {
#endif


struct lightjson_handler {
    void *prv;
    lightjson_status (*compound_opened)(void *prv, lightjson_compound_type type);
    lightjson_status (*compound_closed)(void *prv, lightjson_compound_type type);
    lightjson_status (*object_member_found)(void *prv, const char *key);
    lightjson_status (*value_found)(void *prv, lightjson_value_type type, lightjson_value v);
};


struct lightjson_parser;

extern struct lightjson_parser *lightjson_parser_create(const struct lightjson_handler *const handler, bool relaxed);
extern void lightjson_parser_destroy(struct lightjson_parser *const parser);

extern lightjson_status lightjson_parser_process(struct lightjson_parser *const parser, const char *data, size_t length);


#ifdef __cplusplus
}
#endif

#endif /* LIGHTJSON_PARSER_H_ */
