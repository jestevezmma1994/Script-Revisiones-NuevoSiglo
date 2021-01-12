/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
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

#ifndef SV_MEM_COUNTER_H_
#define SV_MEM_COUNTER_H_

/**
 * @file  SvMemCounter.h
 * @brief Memory counter with subtraction notification
 **/


#include <stdlib.h> // for free()
#include <assert.h>

#ifndef likely
#  define likely(x)   __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#  define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_OWNERS 32

/**
 * @defgroup SvMemCounter Memory counter with subtraction notification
 * @ingroup SvPlayerKit
 * @{
 **/


struct SvMemCounter_s;
typedef struct SvMemCounter_s* SvMemCounter;

/**
 * @struct MemCounterOwner
 * Memory counter owner structure.
 **/
struct MemCounterOwner{
  /** Pointer to a function which will be called after mem counter will be subtracted */
  void (*sub_notify)(struct SvMemCounter_s* mc, void* owner);
  /** Pointer to the actual owner of the memory counter */
  void* owner;
};

/**
 * @struct SvMemCounter_s
 * Memory counter structure.
 * This structure shoud be allocated with calloc or zeroed out with some other mechanism (memset) to work properly
 **/
struct SvMemCounter_s{
  /** Array of active owners. Owners are packed at the beggining of an array.
   * First element with NULL-ed fields is one element after last owner.
   **/
  struct MemCounterOwner owners[MAX_OWNERS];
  size_t firstNotifyOwner;
  /** Amount of used memory monitored by the counter */
  unsigned int used;
  /** Flag telling if the mem counter will be automatically freed upon reaching 0 used memory */
  short autodestruction;
};

/**
 * Add the given ammount of memory to memory counter
 *
 * @param[in] mc counter handle
 * @param[in] bytes amount of memory to add to used memory
 * @return none
 **/
static inline void SvMemCounterAdd(struct SvMemCounter_s* mc, unsigned int bytes)
{
  mc->used += bytes;
};

/**
 * Call the notification function on all owners
 *
 * @param[in] mc counter handle
 * @return none
 **/
static inline void SvMemCounterNotifyOwners(struct SvMemCounter_s* mc)
{
  size_t numOwners = 0;
  for (size_t i = mc->firstNotifyOwner; i < MAX_OWNERS; i++) {
    if (mc->owners[i].owner == NULL) {
      break; //encountered an element after last owner
    }
    numOwners++;
    mc->owners[i].sub_notify(mc, mc->owners[i].owner);
  }
  for (size_t i = 0; i < mc->firstNotifyOwner; i++) {
    if (mc->owners[i].owner == NULL) {
      break; //encountered an element after last owner
    }
    numOwners++;
    mc->owners[i].sub_notify(mc, mc->owners[i].owner);
  }
  mc->firstNotifyOwner++;
  if (mc->firstNotifyOwner >= numOwners) {
    mc->firstNotifyOwner = 0;
  }
}

/**
 * Subtract given amount of memory from used memory and call notification on all owners
 *
 * @param[in] mc counter handle
 * @param[in] bytes amount of memory to subtract
 * @return none
 **/
static inline void SvMemCounterSub(struct SvMemCounter_s* mc, unsigned int bytes)
{
  mc->used -= bytes;
  if (unlikely(mc->autodestruction)) {
    if (unlikely(mc->used == 0))
      free(mc);
  } else {
    SvMemCounterNotifyOwners(mc);
  };
};

/**
 * Add an owner to the memory counter.
 *
 * This function adds the owner to the last free position in the array of owners or fails if the array is allready full
 *
 * @param[in] mc counter handle
 * @param[in] owner The structure representing the owner to be added
 * @return 1 if success or 0 otherwise
 **/
static inline int SvMemCounterAddOwner(struct SvMemCounter_s* mc, struct MemCounterOwner owner)
{
  for (int i = 0; i < MAX_OWNERS; i++) {
    if (mc->owners[i].owner == owner.owner) { //allready have this owner - just change notify function
      mc->owners[i].sub_notify = owner.sub_notify;
      return 1;
    }
    if (mc->owners[i].owner == NULL) { //after last element - insert here
      mc->owners[i] = owner;
      return 1;
    }
  }
  //no more space available
  assert(0 && "SvMemCounterAddOwner - Max memory counter owners reached");
  return 0;
}

/**
 * Remove an owner from the owners list.
 *
 * This function finds owner to be removed,
 * finds last owner in the owners array,
 * moves the last owner in place of the one to be removed,
 * and finally NULL-ifies the last owner.
 *
 * @param[in] mc counter handle
 * @param[in] owner The pointer to the owner to be removed
 * @return none
 **/
static inline void SvMemCounterRemoveOwner(struct SvMemCounter_s* mc, void* owner)
{
  int lastOwner = -1;
  int foundOwner = -1;
  for (int i = 0; i < MAX_OWNERS; i++) {
    if (mc->owners[i].owner == NULL) {
      break;
    }
    if (mc->owners[i].owner == owner) {
      foundOwner = i;
    }
    lastOwner = i;
  }
  if (foundOwner != -1) {
    mc->owners[foundOwner] = mc->owners[lastOwner];
    mc->owners[lastOwner].owner = NULL;
    mc->owners[lastOwner].sub_notify = NULL;
  }
}

/**
 * Checks if owner is on the list of owners.
 *
 * @param[in] mc counter handle
 * @param[in] owner The pointer to the owner to be checked
 * @return 1 if owner is on the list of owners or 0 otherwise
 **/
static inline int SvMemCounterHasOwner(struct SvMemCounter_s* mc, void* owner)
{
  for (int i = 0; i < MAX_OWNERS; i++) {
    if (mc->owners[i].owner == owner) {
      return 1;
    } else if (mc->owners[i].owner == NULL) {
      return 0;
    }
  }
  return 0;
}

/**
 * Get number of owners.
 *
 * @param[in] mc counter handle
 * @return       number of owners
 **/
static inline size_t SvMemCounterGetOwnersCount(struct SvMemCounter_s* mc)
{
    size_t numOwners = 0;
    for (size_t i = 0; i < MAX_OWNERS; i++) {
        if (mc->owners[i].owner != NULL) {
            numOwners++;
        }
    }
    return numOwners;
}

/**
 * Removes all owners of a memory counter
 *
 * @param[in] mc counter handle
 * @return none
 **/
static inline void SvMemCounterRemoveAllOwners(struct SvMemCounter_s* mc)
{
  for (int i = 0; i < MAX_OWNERS; i++) {
    mc->owners[i].owner = NULL;
    mc->owners[i].sub_notify = NULL;
  }
}

/**
 * Sets the autodestruction flag and checks if the memory counter should free itself now
 *
 * @param[in] mc counter handle
 * @return none
 **/
static inline void SvMemCounterAutodestroy(struct SvMemCounter_s* mc)
{
  mc->autodestruction = 1;
  SvMemCounterSub(mc, 0);
};

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_MEM_COUNTER_H
