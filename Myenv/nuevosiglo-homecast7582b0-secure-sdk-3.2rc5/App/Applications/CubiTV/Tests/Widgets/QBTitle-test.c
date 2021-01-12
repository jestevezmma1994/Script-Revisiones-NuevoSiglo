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

#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBAsyncService.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <check.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <QBInput/QBInputService.h>
#include <Services/core/QBTextRenderer.h>
#include <QBResourceManager/QBResourceManager.h>
#include <Widgets/QBTitle.h>
#include <settings.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <QBPlatformHAL/QBPlatformInit.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvCommonDefs.h> // SvConstructor

extern void register_suite(Suite *s);

//How long are we going to wait until all renders have finished.
//This should be very high, but if the test passes we will never
//wait this long.
#define RENDER_DEADLINE (10 * 1000)

static SvScheduler scheduler = NULL;
static QBTextRenderer renderer = NULL;
static SvApplication app = NULL;
static SvFiber fiber = NULL;
static SvFiberTimer timer = NULL;
static int stepNum = 0;
static int subStepNum = 0;


//This value has to be synchronized with QBTitle implementation.
#define MAX_PARTS 4
static const char* titles[][MAX_PARTS] = { {"1", "2", "3", "4"}, {"ą","Ę", "ś", "Ż"}, {"Aą", "cĆ", "gI", "dT" } };
static const char* titles2[][MAX_PARTS] = { {"5", "6", "7", "8"}, {"Ą","ę", "Ś", "ż"}, {"aĄ", "Cć", "Gi", "Dt" } };

static int addSearchPath(char* buf, int max_len, const char* path)
{
  int len = snprintf(buf, max_len, ":%s", path);
  SvLogNotice("  %s", path);
  return len;
}

static SvWidget createTitle(void)
{
    stepNum = 0;

    SvSchedulerCreateMain();
    scheduler = SvSchedulerGet();
    QBInputServiceInitialize(NULL);
    QBPlatformInit();
    renderer = QBTextRendererCreate(10, 20, NULL);

    const char *prefix = SvGetRuntimePrefix();
    char buf_settingsPrefix[256];
    snprintf(buf_settingsPrefix, sizeof(buf_settingsPrefix),
            "%s/usr/local/share/CubiTV/data/%s", prefix, "720p");

    char buf[4096];
    int max_len = sizeof(buf);
    int len = 0;

    len += snprintf(buf+len, max_len-len, "./");
    len += addSearchPath(buf+len, max_len-len, buf_settingsPrefix);
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s/usr/local/share/CubiTV", prefix);
    len += addSearchPath(buf+len, max_len-len, tmp);
    snprintf(tmp, sizeof(tmp), "%s/usr/local/share/fonts/", prefix);
    len += addSearchPath(buf+len, max_len-len, tmp);
    snprintf(tmp, sizeof(tmp), "%s/opt/share", prefix);
    len += addSearchPath(buf+len, max_len-len, tmp);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
    QBResourceManagerSetCacheSize(resourceManager, 10 * 1024 * 1024, 20 * 1024 * 1024, NULL);
    QBResourceManagerSetSearchPaths(resourceManager, buf, "ignore_me", NULL);

    app = svAppCreate(1280, 720, 25, NULL);

    svSettingsInit(NULL);
    svSettingsPushComponent("Title.settings");
    SvWidget title = QBTitleNew(app, "Title", renderer);
    svSettingsPopComponent();

    return title;
}

static void startTitle(void)
{
    QBServiceRegistryStartServices(QBServiceRegistryGetInstance(), scheduler, NULL);
    SvInvokeInterface(QBAsyncService, renderer, start, scheduler, NULL);
    svAppSetupMainLoop(app, scheduler);
}

static void stopTitle(void)
{
    if (fiber)
        SvFiberDeactivate(fiber);
    if (timer)
        SvFiberEventDeactivate(timer);
    SvInvokeInterface(QBAsyncService, renderer, stop, NULL);
    QBServiceRegistryStopServices(QBServiceRegistryGetInstance(), 0, NULL);
    ck_assert(subStepNum == 0);
    svAppPostEvent(app, NULL, svQuitEventCreate());
}

static void destroyTitle(SvWidget title)
{
    if (fiber)
        SvFiberDestroy(fiber);
    fiber = NULL;
    timer = NULL;
    svWidgetDestroy(title);
    svSettingsDeinit();
    svAppDestroy(app);
    app = NULL;
    SVRELEASE(renderer);
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("ResourceManager"));
    QBResourceManagerRemoveAllResources(resourceManager, NULL);
    QBPlatformDeinit();
    QBInputServiceCleanup();
    renderer = NULL;
    SvSchedulerCleanup();
    scheduler = NULL;
    sleep(1);
}

static void setTitle(SvWidget w, const char* title, int pos)
{
    SvString titleStr = SvStringCreate(title, NULL);
    QBTitleSetText(w, titleStr, pos);
    SVRELEASE(titleStr);
}

START_TEST(qbtitle_lifetime)
{
    SvWidget w = createTitle();
    startTitle();
    stopTitle();
    destroyTitle(w);
}
END_TEST

/**
 * This is a 'simple' state machine. Input is stepNum, which says which test should be executed.
 * The test will set 'subStepNum', which says how many times QBTitle callback will be called.
 * After it is called this many times the test has ended, and we proceed to the next one.
 *
 * In some tests subStepNum is intentionally lower, but it is synchronized with next tests. This way
 * we can do some operations in parallel.
 **/
static void qbtitle_fill_all_parts_next_test(SvWidget w)
{
    ck_assert(!SvFiberEventIsActive(timer));
    SvFiberDeactivate(fiber);
    SvFiberEventDeactivate(timer);

    SvFiberTimerActivateAfter(timer, SvTimeFromMs(RENDER_DEADLINE));
    int prevStepNum = stepNum;
    stepNum++;
    if (prevStepNum < 16) {
        /**
         * Simplest test, just set a title.
         **/
        subStepNum = MAX_PARTS;
        for (int i = 0; i < MAX_PARTS; i++) {
            setTitle(w, titles[prevStepNum % (sizeof(titles)/sizeof(*titles))][i], i);
        }
        return;
    }
    prevStepNum -= 16;
    if (prevStepNum < MAX_PARTS) {
        /**
         * Clear the title backwards.
         **/
        subStepNum = 1;
        setTitle(w, "", MAX_PARTS - prevStepNum - 1);
        return;
    }
    prevStepNum -= MAX_PARTS;
    if (prevStepNum < 1) {
        /**
         * Fill it again with new texts.
         **/
        subStepNum = MAX_PARTS;
        for (int i = 0; i < MAX_PARTS; i++) {
            setTitle(w, titles2[0][i], i);
        }
        return;
    }
    prevStepNum -= 1;
    if (prevStepNum < 1) {
        /**
         * Fill the title partialy backwards, but execute the next test
         * just after the first text is rendered.
         **/
        subStepNum = 1;
        for (int i = 0; i < MAX_PARTS / 2; i++) {
            setTitle(w, titles2[1][MAX_PARTS - i - 1], MAX_PARTS - i - 1);
        }
        return;
    }
    prevStepNum -= 1;
    if (prevStepNum < 1) {
        /**
         * Fill the title partialy backwards, completing the previous test.
         **/
        subStepNum = MAX_PARTS - 1;
        for (int i = 0; i < MAX_PARTS - MAX_PARTS / 2; i++) {
            setTitle(w, titles2[1][MAX_PARTS - MAX_PARTS / 2 - i - 1], MAX_PARTS - MAX_PARTS / 2 - i - 1);
        }
        return;
    }
    prevStepNum -= 1;
    if (prevStepNum < 1) {
        /**
         * Empty the title in the middle.
         **/
        subStepNum = 1;
        setTitle(w, "", MAX_PARTS / 2);
        return;
    }
    prevStepNum -= 1;
    if (prevStepNum < 1) {
        /**
         * Empty the title altogether.
         **/
        subStepNum = 1;
        setTitle(w, "", 0);
        return;
    }
    prevStepNum -= 1;
    if (prevStepNum < 1) {
        /**
         * Fill the title backwards.
         **/
        subStepNum = 1;
        for (int i = MAX_PARTS - 1; i >= 0; i--)
            setTitle(w, titles2[2][i], i);
        return;
    }
    prevStepNum -= 1;
    if (prevStepNum < 1) {
        /**
         * Quickly replace a single title in the middle.
         **/
        subStepNum = 2;
        setTitle(w, "", MAX_PARTS / 2);
        setTitle(w, titles[0][0], MAX_PARTS / 2);
        return;
    }

    stopTitle();
}

static void qbtitle_fill_all_parts_fiber_step(void *self_)
{
    ck_abort();
}

static void qbtitle_fill_all_parts_callback(void *self_, SvWidget w)
{
    subStepNum--;
    ck_assert(subStepNum >= 0);
    if (!subStepNum)
        qbtitle_fill_all_parts_next_test(w);
}

START_TEST(qbtitle_fill_all_parts)
{
    SvWidget w = createTitle();
    startTitle();

    QBTitleSetCallback(w, qbtitle_fill_all_parts_callback, NULL);

    fiber = SvFiberCreate(scheduler, NULL, "testWaitFiber", qbtitle_fill_all_parts_fiber_step, NULL);
    timer = SvFiberTimerCreate(fiber);

    qbtitle_fill_all_parts_next_test(w);

    SvSchedulerLoop(scheduler, false);

    destroyTitle(w);
}
END_TEST

static SvConstructor void init(void)
{
    Suite *s = suite_create("QBTitle");
    TCase *tc = tcase_create("Core");
    suite_add_tcase(s, tc);

    tcase_add_loop_test(tc, qbtitle_lifetime, 0, 2);
    tcase_add_test(tc, qbtitle_fill_all_parts);

    register_suite(s);
}
