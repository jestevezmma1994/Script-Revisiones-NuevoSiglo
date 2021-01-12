/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include <Services/QBDiskActivator.h>
#include <SvCore/SvCommonDefs.h> // SvConstructor
#include <check.h>

extern void register_suite(Suite *s);

// just create and destroy diska activator object
START_TEST(QBDiskActivator_QBDiskActivatorCreate)
{
    QBDiskActivator diskActivator = QBDiskActivatorCreate(NULL, NULL);
    SVTESTRELEASE(diskActivator);
}
END_TEST

static SvConstructor void init(void)
{
    Suite *s = suite_create("CubiTV");
    TCase *tc = tcase_create("QBDiskActivator");
    suite_add_tcase(s, tc);

    tcase_add_test(tc, QBDiskActivator_QBDiskActivatorCreate);

    register_suite(s);
}
