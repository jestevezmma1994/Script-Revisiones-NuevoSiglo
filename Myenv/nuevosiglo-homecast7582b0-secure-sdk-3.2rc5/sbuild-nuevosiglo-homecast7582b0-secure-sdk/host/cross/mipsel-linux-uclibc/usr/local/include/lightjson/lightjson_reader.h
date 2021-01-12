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

#ifndef LIGHTJSON_READER_H_
#define LIGHTJSON_READER_H_

#include "lightjson_types.h"

#ifdef __cplusplus
extern "C" {
#endif


struct lightjson_reader;
struct lightjson_handler;


extern struct lightjson_reader *lightjson_reader_create(unsigned int max_depth);
extern void lightjson_reader_destroy(struct lightjson_reader *const reader);

extern lightjson_status lightjson_reader_get_status(const struct lightjson_reader *const reader);

extern const struct lightjson_handler *lightjson_reader_get_handler(const struct lightjson_reader *const reader);

extern lightjson_compound_type lightjson_reader_get_root(const struct lightjson_reader *const reader, lightjson_value *const value);


#ifdef __cplusplus
}
#endif

#endif /* LIGHTJSON_READER_H_ */
