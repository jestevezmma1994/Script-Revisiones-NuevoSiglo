/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_ACCES_MANAGET_TEST_CASES_H
#define QB_ACCES_MANAGET_TEST_CASES_H

#include <check.h>

//add new test cases here

/**
 * adding plugins to access Manager
 */
TCase* QBAccessManagetTest_addPlugins_testCase(void);

/**
 * authenticating using access manager
 */
TCase* QBAccessManagetTest_authenticate_testCase(void);

/**
 * password management using access manager
 */
TCase* QBAccessManagetTest_password_testCase(void);

/**
 * blocking of access manager after unsuccessful authentication attempts
 */
TCase* QBAccessManagetTest_blocking_testCase(void);

#endif /* QB_ACCES_MANAGET_TEST_CASES_H */
