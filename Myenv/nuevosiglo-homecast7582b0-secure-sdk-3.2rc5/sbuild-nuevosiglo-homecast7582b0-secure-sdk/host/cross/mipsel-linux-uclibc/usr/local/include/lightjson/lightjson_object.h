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

#ifndef LIGHTJSON_OBJECT_H_
#define LIGHTJSON_OBJECT_H_

#include <stddef.h>
#include <stdbool.h>
#include "lightjson_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Create empty JSON object.
 *
 * @return              created hash table
 **/
extern struct lightjson_object *lightjson_object_create(void);

/**
 * Destroy JSON object.
 *
 * @param[in] self      hash table
 **/
extern void lightjson_object_destroy(struct lightjson_object *const object);

/**
 * Get number of member of an object.
 **/
extern size_t lightjson_object_get_members_count(const struct lightjson_object *const object);

/**
 * Get type of an object member.
 *
 * @param[in] self      hash table
 * @param[in] name      key of a value to find
 * @return              value of @c NULL if not found
 **/
extern lightjson_type lightjson_object_get_member_type(const struct lightjson_object *const object, const char *member_name);

/**
 * Get value of an object member.
 *
 * @param[in] self      hash table
 * @param[in] name      key of a value to find
 * @return              value of @c NULL if not found
 **/
extern lightjson_type lightjson_object_get_member(const struct lightjson_object *const object, const char *member_name, lightjson_value *const value);

/**
 * Insert member into object.
 *
 * @param[in] self      hash table
 * @param[in] key       key to associate @a value with
 * @param[in] value     value to be added
 * @param[in] dtor      optional destructor for @a value
 **/
extern int lightjson_object_set_member(struct lightjson_object *const object, const char *member_name, lightjson_type type, const lightjson_value value);

/**
 * Remove member from object.
 *
 * @param[in] self      hash table
 * @param[in] key       key to associate @a value with
 * @param[in] value     value to be added
 * @param[in] dtor      optional destructor for @a value
 **/
extern int lightjson_object_remove_member(struct lightjson_object *const object, const char *member_name);


typedef void *lightjson_object_iterator;

extern lightjson_object_iterator lightjson_object_get_iterator(const struct lightjson_object *const object);

extern lightjson_type lightjson_object_iterator_get_next(lightjson_object_iterator *const iterator, const char **member_name, lightjson_value *const value);


#ifdef __cplusplus
}
#endif

#endif /* LIGHTJSON_OBJECT_H_ */
