/**************************************************************************
 * zonetrace: tracing memory allocator
 * Copyright (C) 2013 Tomasz Lukaszewski <lupus@cubiware.com>
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

#ifndef ZONE_TRACE_H_
#define ZONE_TRACE_H_

#include <stdint.h>


#define ZONE_TRACE_ZONEID_BITS      12U
#define ZONE_TRACE_ZONEID_OFFSET    4U
#define ZONE_TRACE_ZONEID_CNT       (1U << ZONE_TRACE_ZONEID_BITS)
#define ZONE_TRACE_ZONEID_MASK      0xfff0U

#define ZONE_TRACE_FLAGS_BITS       4U
#define ZONE_TRACE_FLAGS_OFFSET     0U
#define ZONE_TRACE_FLAGS_MASK       0x0fU

#define ZONE_TRACE_FLAG_TRACE       0x01U


struct zone_trace_zinfo {
    unsigned int id;
    unsigned long int blocks_count;
    unsigned long int total_used;
};

typedef void (*zone_trace_zinfo_fn)(void *prv, const struct zone_trace_zinfo *zone_info);
typedef void (*zone_trace_binfo_fn)(void *prv, const struct zone_trace_zinfo *zone_info, void *block, uint64_t tag);

struct zone_trace_entries {
    void (*setup)(uint64_t default_tag, uint64_t default_thread_tag);
    void (*set_tag)(uint64_t tag);
    void *(*alloc_tagged)(unsigned long int size, unsigned long int alignment, uint64_t tag, uint64_t mask);
    void *(*alloc_linked)(unsigned long int size, unsigned long int alignment, const void *block);
    int (*get_zone_info)(unsigned int zone_id, struct zone_trace_zinfo *info);
    int (*get_block_info)(const void *block, unsigned long int *size, uint64_t *tag);
    int (*get_zones)(zone_trace_zinfo_fn callback, void *prv);
    int (*get_blocks)(unsigned int zone_id, zone_trace_binfo_fn callback, void *prv);
};

extern const struct zone_trace_entries *zone_trace_get_entries(void);


#endif
