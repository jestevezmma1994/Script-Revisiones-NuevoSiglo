/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef FILE_MPEG_PSI_PARSER_H
#define FILE_MPEG_PSI_PARSER_H

#include <sv-list.h>

#include "mpeg_common.h"
#include "mpeg_demux.h"

// points to mpeg data - data fields, descriptors, etc.
typedef struct _mpeg_data mpeg_data;

struct _mpeg_data
{
    int length;
    const uint8_t* data;
};

void mpeg_data_init(mpeg_data* d0, const mpeg_data* d1);
void mpeg_data_copy(mpeg_data* d0, const mpeg_data* d1);
void mpeg_data_clear(mpeg_data* d0);

char* mpeg_data_strdup( mpeg_data* data );

//
typedef struct _mpeg_descriptor mpeg_descriptor;

struct _mpeg_descriptor
{
    list_linkage_t linkage; // sv-list link
    mpeg_data data;
};

int mpeg_descriptor_get_tag(mpeg_descriptor* desc);

/**
 * Get tag extension from extension descriptor.
 *
 * @param[in] desc    unparsed mpeg descriptor
 * @return            int value tag in extension descriptor
 */
int mpeg_descriptor_get_extension_tag(mpeg_descriptor *desc);

// map descriptor_tag -> descriptors list
typedef struct _mpeg_descriptor_map mpeg_descriptor_map;

struct _mpeg_descriptor_map
{
    list_t* map[MPEG_DESC_TAG_NUMBER];
};

mpeg_descriptor_map* mpeg_descriptor_map_create(void);
void mpeg_descriptor_map_destroy(mpeg_descriptor_map* descriptor_map);

// get a mpeg_data - single (the first) descriptor for descriptor_tag
mpeg_data* mpeg_descriptor_map_get(mpeg_descriptor_map* descriptor_map, int descriptor_tag);

// get a mpeg_descriptor list - descriptors with descriptor_tag
list_t* mpeg_descriptor_map_get_list(mpeg_descriptor_map* descriptor_map, int descriptor_tag);

// get a mpeg_data - single (the first) descriptor for extension_tag (descriptor_tag equals extension descriptor (0x7f))
mpeg_data* mpeg_descriptor_map_get_extended(mpeg_descriptor_map* descriptor_map, int descriptor_tag_extension);

void mpeg_descriptor_map_append(mpeg_descriptor_map* descriptor_map, mpeg_data* data, int descriptor_tag);

/// remove from descriptor map all descriptors with given tag.
void mpeg_descriptor_map_remove(mpeg_descriptor_map* descriptor_map, int descriptor_tag);

// get length of descriptors loop
// useful when the descriptors loop was adjusted, and its table must be re-generated
int descriptor_map_get_length( mpeg_descriptor_map* descriptor_map );

/// write \a descriptor_map to \a ptr
/// \pre ptr points to at least descriptor_map_get_length bytes
/// \return ptr + descriptor_map length
uint8_t* descriptor_map_write( mpeg_descriptor_map* descriptor_map, uint8_t* ptr);

void descriptor_map_report( mpeg_descriptor_map* descriptor_map, const char* indent_string );

// declares local "desc_class_name* desc_var_name;" and inits it for first desc_tag in desc_map
// use MPEG_DESC_DESTROY to destroy desc_var_name
#define MPEG_DESC_INIT( desc_class_name, desc_var_name, desc_map, desc_tag ) \
    mpeg_data* __ ## desc_class_name ## _data = mpeg_descriptor_map_get( desc_map, desc_tag ); \
    desc_class_name __ ## desc_var_name; \
    desc_class_name* desc_var_name = 0; \
    if( __ ## desc_class_name ## _data != 0 ) \
    { \
        desc_var_name = & ( __ ## desc_var_name ); \
        desc_class_name ## _init( desc_var_name, __ ## desc_class_name ## _data ); \
    }

// destroy "desc_class_name* desc_var_name;"
#define MPEG_DESC_DESTROY( desc_class_name, desc_var_name ) \
    if( desc_var_name != 0 ) \
    { \
        desc_class_name ## _destroy( desc_var_name ); \
        desc_var_name = 0; \
    }

// for a given "mpeg_data accessor_name(desc_class_name* desc_var_name);"
// convert returned mpeg_data to strdup'ed char*
#define MPEG_DATA_STRDUP( str_var_name, desc_var_name, accessor_name ) \
    if( desc_var_name != 0 ) \
    { \
        mpeg_data __ ## desc_var_name ## _data = accessor_name( desc_var_name ); \
        (*str_var_name) = mpeg_data_strdup( & ( __ ## desc_var_name ## _data ) ); \
    } \
    else \
    { \
        (*str_var_name) = 0; \
    }

// for a given: "mpeg_data accessor_name(desc_class_name* desc_var_name);",
// desc_map and desc_tag,
// convert returned mpeg_data to strdup'ed char*
#define MPEG_DESC_STRDUP( str_var_name, desc_class_name, accessor_name, desc_map, desc_tag ) \
    { \
        MPEG_DESC_INIT( desc_class_name, __ ## accessor_name ## _desc, desc_map, desc_tag ); \
        MPEG_DATA_STRDUP( str_var_name, __ ## accessor_name ## _desc, accessor_name ); \
        MPEG_DESC_DESTROY( desc_class_name, __ ## accessor_name ## _desc ); \
    }

#endif
/* _MPEG_PARSER_H */

