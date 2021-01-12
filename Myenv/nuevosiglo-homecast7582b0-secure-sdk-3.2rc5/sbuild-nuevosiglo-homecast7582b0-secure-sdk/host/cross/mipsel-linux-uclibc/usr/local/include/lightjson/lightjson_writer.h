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

#ifndef LIGHTJSON_WRITER_H_
#define LIGHTJSON_WRITER_H_

#include <stddef.h>
#include <stdbool.h>
#include "lightjson_types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    lightjson_line_break_none = 0,
    lightjson_line_break_LF,
    lightjson_line_break_CR,
    lightjson_line_break_CRLF
} lightjson_line_break_type;


struct lightjson_writer_config {
    lightjson_line_break_type line_break_type;
    struct {
        unsigned int character;
        unsigned int depth;
    } indentation;
    bool always_escape_non_ASCII;
};

typedef size_t (*lightjson_flush_fn)(void *prv, const char *buffer, size_t length);


struct lightjson_writer;
struct lightjson_handler;


extern struct lightjson_writer *lightjson_writer_create(const struct lightjson_writer_config *const config, lightjson_flush_fn flush_fn, void *prv);
extern void lightjson_writer_destroy(struct lightjson_writer *const writer);

extern lightjson_status lightjson_writer_get_status(const struct lightjson_writer *const writer);

extern const struct lightjson_handler *lightjson_writer_get_handler(const struct lightjson_writer *const writer);

extern lightjson_status lightjson_writer_flush_buffer(struct lightjson_writer *const writer);

extern lightjson_status lightjson_writer_open_compound(struct lightjson_writer *const writer, lightjson_compound_type type);
extern lightjson_status lightjson_writer_close_compound(struct lightjson_writer *const writer, lightjson_compound_type type);
extern lightjson_status lightjson_writer_emit_object_member(struct lightjson_writer *const writer, const char *key);
extern lightjson_status lightjson_writer_emit_value(struct lightjson_writer *const writer, lightjson_value_type type, lightjson_value v);


#ifdef __cplusplus
}
#endif

#endif /* LIGHTJSON_WRITER_H_ */
