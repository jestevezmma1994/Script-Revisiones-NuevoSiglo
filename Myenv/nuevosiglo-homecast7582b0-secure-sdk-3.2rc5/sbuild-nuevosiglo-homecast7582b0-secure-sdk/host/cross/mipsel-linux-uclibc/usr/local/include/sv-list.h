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

#ifndef SV_LIST_H_
#define SV_LIST_H_

/**
 * @file sv-list.h Simple intrusive lists library
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvListLibrary sv-list: simple intrusive lists library
 * @ingroup Utils
 *
 * This library provides two different intrusive list implementations.
 **/


/**
 * @defgroup SvList Intrusive list with explicit pointer to list element
 * @ingroup SvListLibrary
 * @{
 **/

/**
 * @page ReferenceManual
 *
 * @section SvListUsage Using intrusive lists
 *
 * This is a simple, complete example of list usage. It is worth noting
 * that lists do not manage their own memory which makes them suitable for
 * various environments.
 *
@code
  #include <sv-list.h>

  // single element of the list
  typedef struct {
    const char *text;
    list_linkage_t linkage;
  } line_of_text_;

  static line_of_text_ *create_line_of_text(const char *text)
  {
    line_of_text_ *self = malloc(sizeof(*self));
    self->text = text;
    // initializing linkage elements is required, the second argument
    // is the base of the structure that holds the linkage field
    list__init_linkage(&self->linkage, self);
    return self;
  }

  int main(void)
  {
    list_t all_lines = LIST__INITIALIZER(all_lines);

    // elements are added to the list using an explicit linkage field
    list__push_back(&all_lines, &(create_line_of_text("First line")->linkage));
    list__push_back(&all_lines, &(create_line_of_text("Second line")->linkage));

    // linkage variable is required to support older (before C99) syntax,
    // it must be a pointer to list_linkage_t
    list_linkage_t *linkage;

    // iteration over all elements is easy
    line_of_text_ *line_of_text;
    int line_num = 0;
    list__for_each(&all_lines, line_of_text, linkage) {
      printf ("03%d) %s\n", ++line_num, line_of_text->text);
    }

    // safe iteration must be used when list is modified during iteration
    list_linkage_t *backup_linkage;
    list__for_each_safe(&all_lines, line_of_text, linkage, backup_linkage) {
      list__unlink(&(line_of_text->linkage));
      free(line_of_text);
    }

    return EXIT_SUCCESS;
  }
@endcode
**/

/**
 * List linkage node.
 *
 * Linkage node contains pointers to neighbour elements. To simplify list usage
 * it also contains an offset to the base element; it could be derived from the
 * type, but that would seriously complicate loop code and be more error prone.
 **/
typedef struct list_linkage_t {
    /// next element on the list
    struct list_linkage_t *next;
    /// previous element on the list
    struct list_linkage_t *prev;
    /// offset of the linkage node from the start of the structure that contains it
    ptrdiff_t base_offset;
} list_linkage_t;

/**
 * Intrusive list type.
 *
 * List type itself simply encapsulates a single linkage node. This allows for
 * the compiler to check for linkage vs list pointers and thus makes the code safer.
 **/
typedef struct {
    /// head of the list
    list_linkage_t head;
} list_t;

/**
 * Initialize intrusive list.
 *
 * This function initializes a list. In static context you can use
 * LIST__INITIALIZER() for identical result.
 *
 * @param[in] list intrusive list
 **/
extern void list__init(list_t *list);

/**
 * Initialize linkage node.
 *
 * @param[in] node linkage node inside a structure
 * @param[in] base pointer to the structure containing @a node
 **/
extern void list__init_linkage(list_linkage_t *node, void *base);

/**
 * Link given node just before the other element.
 *
 * @param[in] node linkage node to be added to the list
 * @param[in] before_me linkage node before which to add @a node
 **/
extern void list__link_before(list_linkage_t *node, list_linkage_t *before_me);

/**
 * Link given node just after the other element.
 *
 * @param[in] node linkage node to be added to the list
 * @param[in] after_me linkage node after which to add @a node
 **/
extern void list__link_after(list_linkage_t *node, list_linkage_t *after_me);

/**
 * Add a node to the beginning of the list.
 *
 * @param[in] list intrusive list
 * @param[in] node linkage node to be added to the list
 **/
extern void list__push(list_t *list, list_linkage_t *node);

/**
 * Add a node to the end of the list.
 *
 * @param[in] list intrusive list
 * @param[in] node linkage node to be added to the list
 **/
extern void list__push_back(list_t *list, list_linkage_t *node);

/**
 * Remove and return the first element of the list.
 *
 * @param[in] list intrusive list
 * @return removed element if @a list is not empty, @c NULL if @a list is empty
 **/
extern void *list__pop(list_t *list);

/**
 * Remove and return the last element of the list.
 *
 * @param[in] list intrusive list
 * @return removed element if @a list is not empty, @c NULL if @a list is empty
 **/
extern void *list__pop_back(list_t *list);

/**
 * Check if list is empty.
 *
 * @param[in] list intrusive list
 * @return @c true of @a list is empty
 **/
extern bool list__empty(list_t *list);

/**
 * Counts elements in list (linear complexity).
 *
 * @param[in] list intrusive list
 * @return count of items in a list
 **/
extern int list__count(list_t *list);

/**
 * Remove a node from the list it is linked to.
 *
 * @param[in] node linkage node to remove from the list
 **/
extern void list__unlink(list_linkage_t *node);

/**
 * Get pointer to the structure containing linkage node.
 *
 * @hideinitializer
 **/
#define LIST__BASE_PTR(ptr_to_node) (void *)(((char *)(ptr_to_node)) - (ptr_to_node)->base_offset)

/**
 * Forward list iteration.
 *
 * To do something with all elements of a list:
@code
  element_t *element;
  list_linkage_t *linkage;
  list__for_each(&some_list, element, linkage) {
    do_something_with_an_element(element);
  }
@endcode
 *
 * @note If you are altering the list while iterating use list__for_each_safe().
 *
 * @param[in] ptr_to_list intrusive list
 * @param[out] ptr_to_item pointer to the structure containing linkage node,
 *                      updated on each step of iteration
 * @param[out] ptr_to_linkage auxiliary pointer to the linkage node
 *
 * @hideinitializer
 **/
#define list__for_each(ptr_to_list, ptr_to_item, ptr_to_linkage)        \
    for (                                                               \
        ptr_to_linkage = (ptr_to_list)->head.next;                      \
        (ptr_to_item) = LIST__BASE_PTR(ptr_to_linkage),                 \
        ptr_to_linkage != &(ptr_to_list)->head;                         \
        ptr_to_linkage = ptr_to_linkage->next                           \
        )

/**
 * Backward list iteration.
 *
 * To do something with all elements of a list:
@code
  element_t *element;
  list_linkage_t *linkage;
  list__for_each_back(&some_list, element, linkage) {
    do_something_with_an_element(element);
  }
@endcode
 *
 * @note If you are altering the list while iterating use list__for_each_back_safe().
 *
 * @param[in] ptr_to_list intrusive list
 * @param[out] ptr_to_item pointer to the structure containing linkage node,
 *                      updated on each step of iteration
 * @param[out] ptr_to_linkage auxiliary pointer to the linkage node
 *
 * @hideinitializer
 **/
#define list__for_each_back(ptr_to_list, ptr_to_item, ptr_to_linkage)   \
    for (                                                               \
        ptr_to_linkage = (ptr_to_list)->head.prev;                      \
        (ptr_to_item) = LIST__BASE_PTR(ptr_to_linkage),                 \
        ptr_to_linkage != &(ptr_to_list)->head;                         \
        ptr_to_linkage = ptr_to_linkage->prev                           \
        )

/**
 * Forward list iteration suitable for alterations of the list.
 *
 * To remove all elements from a list:
@code
  element_t *element;
  list_linkage_t *linkage, *backup;
  list__for_each_safe(&some_list, element, linkage, backup) {
    list__unlink(&element->linkage);
    free_the_element_somehow(element);
  }
@endcode
 *
 * @param[in] ptr_to_list intrusive list
 * @param[out] ptr_to_item pointer to the structure containing linkage node,
 *                      updated on each step of iteration
 * @param[out] ptr_to_linkage auxiliary pointer to the linkage node
 * @param[out] ptr_to_backup auxiliary pointer to the linkage node
 *
 * @hideinitializer
 **/
#define list__for_each_safe(ptr_to_list, ptr_to_item, ptr_to_linkage, ptr_to_backup) \
    for (                                                               \
        ptr_to_linkage = (ptr_to_list)->head.next;                      \
        (ptr_to_item) = LIST__BASE_PTR(ptr_to_linkage),                 \
        ptr_to_backup = ptr_to_linkage->next,                           \
        ptr_to_linkage != &(ptr_to_list)->head;                         \
        ptr_to_linkage = ptr_to_backup                                  \
        )

/**
 * Backward list iteration suitable for alterations of the list.
 *
 * To remove all elements from a list:
@code
  element_t *element;
  list_linkage_t *linkage, *backup;
  list__for_each_back_safe(&some_list, element, linkage, backup) {
    list__unlink(&element->linkage);
    free_the_element_somehow(element);
  }
@endcode
 *
 * @param[in] ptr_to_list intrusive list
 * @param[out] ptr_to_item pointer to the structure containing linkage node,
 *                      updated on each step of iteration
 * @param[out] ptr_to_linkage auxiliary pointer to the linkage node
 * @param[out] ptr_to_backup auxiliary pointer to the linkage node
 *
 * @hideinitializer
 **/
#define list__for_each_back_safe(ptr_to_list, ptr_to_item, ptr_to_linkage, ptr_to_backup) \
    for (                                                               \
        ptr_to_linkage = (ptr_to_list)->head.prev;                      \
        (ptr_to_item) = LIST__BASE_PTR(ptr_to_linkage),                 \
        ptr_to_backup = ptr_to_linkage->prev,                           \
        ptr_to_linkage != &(ptr_to_list)->head;                         \
        ptr_to_linkage = ptr_to_backup                                  \
        )

/**
 * Static list initializer.
 *
 * Static initializer allows for declaring static variables of list type:
@code
  static list_t some_list = LIST__INITIALIZER(some_list);
@endcode
 *
 * @hideinitializer
 **/
#define LIST__INITIALIZER(name) {{&(name).head, &(name).head}}

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif /* SV_LIST_H_ */
