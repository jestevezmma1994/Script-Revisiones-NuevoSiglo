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

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <Logic/QBLogicFactory.h>
#include <check.h>


extern void register_suite(Suite *s);


static SvType AbstractBaseTestLogic_getType(void)
{
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("AbstractBaseTestLogic",
                            sizeof(struct SvObject_),
                            SvObject_getType(),
                            &type,
                            NULL);
        SvTypeSetAbstract(type, NULL);
    }
    return type;
}

struct BaseTestLogic_ {
    struct SvObject_ super_;
};

static SvType BaseTestLogic_getType(void)
{
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("BaseTestLogic",
                            sizeof(struct BaseTestLogic_),
                            AbstractBaseTestLogic_getType(),
                            &type,
                            NULL);
    }
    return type;
}

struct ChildTestLogic_ {
    struct BaseTestLogic_ super_;
};

static SvType ChildTestLogic_getType(void)
{
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("ChildTestLogic",
                            sizeof(struct ChildTestLogic_),
                            BaseTestLogic_getType(),
                            &type,
                            NULL);
    }
    return type;
}

struct GrandChildTestLogic_ {
    struct ChildTestLogic_ super_;
};

static SvType GrandChildTestLogic_getType(void)
{
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("GrandChildTestLogic",
                            sizeof(struct GrandChildTestLogic_),
                            ChildTestLogic_getType(),
                            &type,
                            NULL);
    }
    return type;
}


/**
 * Test if QBLogicFactoryGetInstance() really returns a singleton.
 **/
START_TEST(isSingleton)
{
    QBLogicFactory factory = QBLogicFactoryGetInstance();
    ck_assert_ptr_ne(factory, NULL);

    QBLogicFactory testFactory = QBLogicFactoryGetInstance();
    ck_assert_ptr_eq(testFactory, factory);
}
END_TEST

/**
 * Test if QBLogicFactoryRegisterClass() correctly handles invalid arguments.
 **/
START_TEST(invalidRegisterArguments)
{
    SvErrorInfo error = NULL;

    QBLogicFactory factory = QBLogicFactoryGetInstance();
    ck_assert_ptr_ne(factory, NULL);

    QBLogicFactoryRegisterClass(NULL, NULL, &error);
    ck_assert_ptr_ne(error, NULL);
    ck_assert(SvErrorInfoEquals(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDestroy(error);

    QBLogicFactoryRegisterClass(NULL, BaseTestLogic_getType(), &error);
    ck_assert_ptr_ne(error, NULL);
    ck_assert(SvErrorInfoEquals(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDestroy(error);

    QBLogicFactoryRegisterClass(factory, NULL, &error);
    ck_assert_ptr_ne(error, NULL);
    ck_assert(SvErrorInfoEquals(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDestroy(error);
}
END_TEST

/**
 * Test if factory rejects abstract classes.
 **/
START_TEST(registerAbstractClass)
{
    QBLogicFactory factory = QBLogicFactoryGetInstance();
    ck_assert_ptr_ne(factory, NULL);

    SvErrorInfo error = NULL;
    QBLogicFactoryRegisterClass(factory, AbstractBaseTestLogic_getType(), &error);
    ck_assert_ptr_ne(error, NULL);
    ck_assert(SvErrorInfoEquals(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDestroy(error);
}
END_TEST

/**
 * Test if QBLogicFactoryFindImplementationOf() correctly handles invalid arguments.
 **/
START_TEST(invalidFindArguments)
{
    SvErrorInfo error = NULL;
    SvType impl;

    QBLogicFactory factory = QBLogicFactoryGetInstance();
    ck_assert_ptr_ne(factory, NULL);

    impl = QBLogicFactoryFindImplementationOf(NULL, NULL, &error);
    ck_assert_ptr_eq(impl, NULL);
    ck_assert_ptr_ne(error, NULL);
    ck_assert(SvErrorInfoEquals(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDestroy(error);

    QBLogicFactoryRegisterClass(NULL, BaseTestLogic_getType(), &error);
    ck_assert_ptr_eq(impl, NULL);
    ck_assert_ptr_ne(error, NULL);
    ck_assert(SvErrorInfoEquals(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDestroy(error);

    QBLogicFactoryRegisterClass(factory, NULL, &error);
    ck_assert_ptr_eq(impl, NULL);
    ck_assert_ptr_ne(error, NULL);
    ck_assert(SvErrorInfoEquals(error, SvCoreErrorDomain, SvCoreError_invalidArgument));
    SvErrorInfoDestroy(error);
}
END_TEST

/**
 * Test if factory correctly returns registered classes.
 **/
START_TEST(registerClasses)
{
    SvErrorInfo error = NULL;
    SvType impl;

    QBLogicFactory factory = QBLogicFactoryGetInstance();
    ck_assert_ptr_ne(factory, NULL);

    // after registering base class it should be the only available implementation
    QBLogicFactoryRegisterClass(factory, BaseTestLogic_getType(), &error);
    ck_assert_ptr_eq(error, NULL);
    impl = QBLogicFactoryFindImplementationOf(factory, BaseTestLogic_getType(), &error);
    ck_assert_ptr_eq(error, NULL);
    ck_assert_ptr_eq(impl, BaseTestLogic_getType());

    // registering grandchild class should replace base class
    QBLogicFactoryRegisterClass(factory, GrandChildTestLogic_getType(), &error);
    ck_assert_ptr_eq(error, NULL);
    impl = QBLogicFactoryFindImplementationOf(factory, BaseTestLogic_getType(), &error);
    ck_assert_ptr_eq(error, NULL);
    ck_assert_ptr_eq(impl, GrandChildTestLogic_getType());

    // registering child class should fail, because grandchild class is already registered
    QBLogicFactoryRegisterClass(factory, ChildTestLogic_getType(), &error);
    ck_assert_ptr_ne(error, NULL);
    ck_assert(SvErrorInfoEquals(error, SvCoreErrorDomain, SvCoreError_invalidState));
    SvErrorInfoDestroy(error);
    error = NULL;

    impl = QBLogicFactoryFindImplementationOf(factory, BaseTestLogic_getType(), &error);
    ck_assert_ptr_eq(error, NULL);
    ck_assert_ptr_eq(impl, GrandChildTestLogic_getType());
}
END_TEST


static SvConstructor void init(void)
{
    Suite *s = suite_create("CubiTV");

    TCase *tc = tcase_create("QBLogicFactory");
    suite_add_tcase(s, tc);

    tcase_add_test(tc, isSingleton);
    tcase_add_test(tc, invalidRegisterArguments);
    tcase_add_test(tc, registerAbstractClass);
    tcase_add_test(tc, invalidFindArguments);
    tcase_add_test(tc, registerClasses);

    register_suite(s);
}
