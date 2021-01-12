/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SV_INTRUSIVE_LIST_H
#define SV_INTRUSIVE_LIST_H

/**
 * @file intrusive_list.h Simple intrusive list
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvIntrusiveList Intrusive list
 * @ingroup SvListLibrary
 * @{
 **/

typedef struct intrusive_list_s {
  struct intrusive_list_s *next; // aka. head
  struct intrusive_list_s *prev; // aka. tail
} intrusive_list __attribute__((aligned(8)));


inline static void intrusive_list_init(intrusive_list *lst) {
  lst->next = lst->prev = lst;
};
inline static void intrusive_list_insert_after(intrusive_list *elem_prev, intrusive_list *elem) {
  elem->prev = elem_prev;
  elem->next = elem_prev->next;
  elem_prev->next = elem;
  elem->next->prev = elem;
};
inline static void intrusive_list_insert_before(intrusive_list *elem_next, intrusive_list *elem) {
  elem->next = elem_next;
  elem->prev = elem_next->prev;
  elem_next->prev = elem;
  elem->prev->next = elem;
};
inline static void intrusive_list_push_back(intrusive_list *lst, intrusive_list *elem) {
  intrusive_list_insert_before(lst, elem);
};
inline static void intrusive_list_push_front(intrusive_list *lst, intrusive_list *elem) {
  intrusive_list_insert_after(lst, elem);
};
inline static void intrusive_list_remove(intrusive_list *elem) {
  elem->prev->next = elem->next;
  elem->next->prev = elem->prev;
  elem->next = elem->prev = elem;
};
inline static intrusive_list* intrusive_list_front(intrusive_list *lst) {
  return lst->next;
};
inline static intrusive_list* intrusive_list_back(intrusive_list *lst) {
  return lst->prev;
};
inline static int intrusive_list_empty(const intrusive_list *lst) {
  return lst->next == lst;
};

inline static void intrusive_list_pop_front(intrusive_list *lst) {
  intrusive_list_remove(intrusive_list_front(lst));
};
inline static void intrusive_list_pop_back(intrusive_list *lst) {
  intrusive_list_remove(intrusive_list_back(lst));
};

#define CAT_WITH_LINE(_name_) CAT_WITH_LINE2(_name_, __LINE__)
#define CAT_WITH_LINE2(_name_, _line_) CAT_WITH_LINE3(_name_, _line_)
#define CAT_WITH_LINE3(_name_, _line_) _name_##_line_

#define intrusive_list_for_each(LISTPTR, TYPE, VARNAME) \
  intrusive_list* CAT_WITH_LINE(_end_) = (intrusive_list*) LISTPTR; \
  intrusive_list* CAT_WITH_LINE(_curr_) = intrusive_list_front(CAT_WITH_LINE(_end_)); \
  TYPE VARNAME __attribute__ ((unused)) = (TYPE) CAT_WITH_LINE(_curr_); \
  for ( ; CAT_WITH_LINE(_curr_) != CAT_WITH_LINE(_end_); CAT_WITH_LINE(_curr_) = CAT_WITH_LINE(_curr_)->next, VARNAME = (TYPE)CAT_WITH_LINE(_curr_))

#define intrusive_list_for_each_back(LISTPTR, TYPE, VARNAME) \
  intrusive_list* CAT_WITH_LINE(_end_) = (intrusive_list*) LISTPTR; \
  intrusive_list* CAT_WITH_LINE(_curr_) = intrusive_list_back(CAT_WITH_LINE(_end_)); \
  TYPE VARNAME __attribute__ ((unused)) = (TYPE) CAT_WITH_LINE(_curr_); \
  for ( ; CAT_WITH_LINE(_curr_) != CAT_WITH_LINE(_end_); CAT_WITH_LINE(_curr_) = CAT_WITH_LINE(_curr_)->prev, VARNAME = (TYPE)CAT_WITH_LINE(_curr_))

inline static int intrusive_list_size(const intrusive_list* lst) {
  int cnt = 0;
  intrusive_list_for_each(lst, intrusive_list*, any) {
    cnt++;
  };
  return cnt;
};

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // SV_INTRUSIVE_LIST_H
