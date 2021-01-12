/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef KPEBML_ELEM_H
#define KPEBML_ELEM_H

#include "../external/common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t kpebml_id_t;
typedef uint64_t kpebml_size_t;

struct kpebml_elem_desc;

typedef int (kpebml_section_fun_t) (void* parser, bool starting);
typedef int (kpebml_string_fun_t)  (void* parser, const char* buf, int len);
typedef int (kpebml_uint_fun_t)    (void* parser, uint64_t val);
typedef int (kpebml_double_fun_t)  (void* parser, double val);
typedef int (kpebml_data_fun_t)    (void* parser, kpebml_size_t size);

#define section_fun( _fun_ ) ((void*) (_fun_))
#define string_fun( _fun_ )  ((void*) (_fun_))
#define uint_fun( _fun_ )    ((void*) (_fun_))
#define double_fun( _fun_ )  ((void*) (_fun_))
#define data_fun( _fun_ )    ((void*) (_fun_))

enum kpebml_elem_e
{
  kpebml_elem_section,
  kpebml_elem_string,
  kpebml_elem_uint,
  kpebml_elem_double,
  kpebml_elem_data,
};
union kpebml_functions_u
{
  kpebml_section_fun_t* section_f;
  kpebml_string_fun_t*  string_f;
  kpebml_uint_fun_t*    uint_f;
  kpebml_double_fun_t*  double_f;
  kpebml_data_fun_t*    data_f;
};


#define KPEBML_ELEM_FLAG__ALLOW_INFINITE  0x1

struct kpebml_elem_desc
{
  kpebml_id_t  id;
  const char* name; // for debugging only.
  uint32_t flags;

  enum kpebml_elem_e  type;
  void* fun_u;//void (*fun_u)();//union ebml_functions_u  fun_u;

  const struct kpebml_elem_desc* const * sub_elems;
};

typedef const struct kpebml_elem_desc* const*  kpebml_desc_tab;

#ifdef __cplusplus
}
#endif

#endif // #ifndef KPEBML_ELEM_H
