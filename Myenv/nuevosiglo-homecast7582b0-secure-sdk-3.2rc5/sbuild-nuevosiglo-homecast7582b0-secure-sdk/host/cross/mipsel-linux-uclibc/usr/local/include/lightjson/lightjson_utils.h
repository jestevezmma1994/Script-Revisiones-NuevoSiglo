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

#ifndef LIGHTJSON_UTILS_H_
#define LIGHTJSON_UTILS_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "lightjson_types.h"
#include "lightjson_writer.h"

#ifdef __cplusplus
extern "C" {
#endif


extern lightjson_status lightjson_parse(const char *data, size_t length, bool relaxed, lightjson_compound_type *const type, lightjson_value *const value);

extern lightjson_status lightjson_read(FILE *f, bool relaxed, lightjson_compound_type *const type, lightjson_value *const value);


extern lightjson_status lightjson_stringify(char **const buffer, size_t *const length, size_t max_length, lightjson_compound_type type, lightjson_value value, const struct lightjson_writer_config *const config);

extern lightjson_status lightjson_write(FILE *f, size_t max_length, lightjson_compound_type type, lightjson_value value, const struct lightjson_writer_config *const config);


#ifdef __cplusplus
}
#endif

#endif /* LIGHTJSON_UTILS_H_ */
