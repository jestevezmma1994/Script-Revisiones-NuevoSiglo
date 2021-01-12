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

#ifndef LIGHTJSON_TYPES_H_
#define LIGHTJSON_TYPES_H_

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Error codes.
 **/
typedef enum {
    lightjson_status_unknown = -15,
    lightjson_status_invalid_state = -9,
    lightjson_status_overflow = -8,
    lightjson_status_write_error = -7,
    lightjson_status_data_truncated = -6,
    lightjson_status_read_error = -5,
    lightjson_status_max_depth_exceeded = -4,
    lightjson_status_out_of_memory = -3,
    lightjson_status_parse_error = -2,
    lightjson_status_invalid_argument = -1,
    lightjson_status_OK = 0,
    lightjson_status_finished = lightjson_status_OK,
    lightjson_status_parsing = 1,
    lightjson_status_writing = 1
} lightjson_status;


typedef enum {
    lightjson_type_invalid = 0,
    lightjson_type_string,
    lightjson_type_integer,
    lightjson_type_double,
    lightjson_type_boolean,
    lightjson_type_null,
    lightjson_type_array,
    lightjson_type_object
} lightjson_type;

typedef enum {
    lightjson_value_type_invalid = 0,
    lightjson_value_type_string = lightjson_type_string,
    lightjson_value_type_integer = lightjson_type_integer,
    lightjson_value_type_double = lightjson_type_double,
    lightjson_value_type_boolean = lightjson_type_boolean,
    lightjson_value_type_null = lightjson_type_null
} lightjson_value_type;

typedef enum {
    lightjson_compound_type_invalid = 0,
    lightjson_compound_type_array = lightjson_type_array,
    lightjson_compound_type_object = lightjson_type_object
} lightjson_compound_type;


/**
 * JSON object.
 **/
struct lightjson_object;

/**
 * JSON array.
 **/
struct lightjson_array;

typedef union {
    struct {
        const char *str;
        size_t length;
    } string_value;
    long long int integer_value;
    double double_value;
    bool boolean_value;
    struct lightjson_array *array_value;
    struct lightjson_object *object_value;
} lightjson_value;


#ifdef __cplusplus
}
#endif

#endif /* LIGHTJSON_TYPES_H_ */
