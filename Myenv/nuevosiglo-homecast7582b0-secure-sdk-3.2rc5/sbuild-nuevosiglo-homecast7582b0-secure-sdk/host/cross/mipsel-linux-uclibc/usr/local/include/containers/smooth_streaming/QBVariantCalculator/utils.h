/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_VARIANT_CALCULATOR_UTILS_H_
#define QB_VARIANT_CALCULATOR_UTILS_H_

/**
 * @file utils.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * Largest of two values
 *
 * @param[in] a fist value
 * @param[in] b second value
 *
 * @return largest of a and b
 **/
static inline int
max(int a, int b)
{
    return a < b ? b : a;
}
/**
 * Smallest of two values
 *
 * @param[in] a fist value
 * @param[in] b second value
 *
 * @return smallest of a and b
 **/
static inline int
min(int a, int b)
{
    return -max(-a, -b);
}

/**
 * Clamps a value between an upper and lower bound
 *
 * @param[in] x value to be bounded
 * @param[in] leftBound left bound
 * @param[in] rightBound right bound
 *
 * @return leftBound if x is less then leftBound,
 *         rightBound if x is grater than rightBound,
 *         x otherwiase
 **/
static inline int
clamp(int x, int leftBound, int rightBound)
{
    return min(max(leftBound, x), rightBound);
}

#endif // QB_VARIANT_CALCULATOR_UTILS_H_
