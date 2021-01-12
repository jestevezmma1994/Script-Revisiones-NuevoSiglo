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

#ifndef LIGHTJSON_ARRAY_H_
#define LIGHTJSON_ARRAY_H_

#include <stddef.h>
#include <stdbool.h>
#include "lightjson_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Create empty JSON array.
 *
 * @return              created hash table
 **/
extern struct lightjson_array *lightjson_array_create(void);

/**
 * Destroy JSON array.
 *
 * @param[in] self      hash table
 **/
extern void lightjson_array_destroy(struct lightjson_array *const array);

/**
 * Get number of items in an array.
 **/
extern size_t lightjson_array_get_length(const struct lightjson_array *const array);

/**
 * Get type of an array item.
 *
 * @param[in] self      hash table
 * @param[in] name      key of a value to find
 * @return              value of @c NULL if not found
 **/
extern lightjson_type lightjson_array_get_item_type(const struct lightjson_array *const array, size_t position);

/**
 * Get value of an array item.
 *
 * @param[in] self      hash table
 * @param[in] name      key of a value to find
 * @return              value of @c NULL if not found
 **/
extern lightjson_type lightjson_array_get_item(const struct lightjson_array *const array, size_t position, lightjson_value *const value);

/**
 * Insert new item into array.
 *
 * @param[in] self      hash table
 * @param[in] key       key to associate @a value with
 * @param[in] value     value to be added
 **/
extern int lightjson_array_insert_item(struct lightjson_array *const array, size_t position, lightjson_type type, const lightjson_value value);

static inline int lightjson_array_add_item(struct lightjson_array *const array, lightjson_type type, const lightjson_value value)
{
    size_t length = lightjson_array_get_length(array);
    return lightjson_array_insert_item(array, length, type, value);
}

/**
 * Remove item from array.
 *
 * @param[in] self      hash table
 * @param[in] key       key to associate @a value with
 * @param[in] value     value to be added
 **/
extern int lightjson_array_remove_item(struct lightjson_array *const array, size_t position);


#ifdef __cplusplus
}
#endif

#endif /* LIGHTJSON_ARRAY_H_ */
