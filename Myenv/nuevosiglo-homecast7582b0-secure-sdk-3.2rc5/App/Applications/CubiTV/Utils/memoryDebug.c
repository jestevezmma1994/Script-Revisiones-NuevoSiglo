/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include <Services/core/QBDebugUtils.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvAtomic.h>
#include <SvCore/SvAllocator.h>
#include <SvCore/SvLog.h>
#include <SvQuirks/SvMemStats.h>


#if SV_LOG_LEVEL > 0

// ------------------------ DEBUG PART SECTION -------------------------

typedef struct QBDebugUtilsMemoryTracer_ {
    SvFiber fiber;
    SvFiberTimer timer;

    struct SvMallocStats prevStats;
    bool havePrevious;

    ssize_t lastZonesCount;
} *QBDebugUtilsMemoryTracer;

/**
 * Return handle to the memory tracer singleton.
 * @return memory tracer handle
**/
SvLocal QBDebugUtilsMemoryTracer QBDebugUtilsGetMemoryTracer(void)
{
    static struct QBDebugUtilsMemoryTracer_ singleton = {
        .fiber        = NULL,
        .timer        = NULL,
        .havePrevious = false
    };

    if (SvOnce()) {
        singleton.lastZonesCount = SvAllocatorGetZonesStats(SvDefaultAllocator, NULL, 0);
    }

    return &singleton;
}

/**
 * Print overall malloc() usage statistics.
 * @param[in] self memory tracer handle
 **/
SvLocal void QBDebugUtilsPrintMallocStats(QBDebugUtilsMemoryTracer self)
{
    struct SvMallocStats stats;
    SvGetMallocStats(&stats);

    if (stats.usedSize < 0 || stats.freeSize < 0) {
        // probably not supported on this platform
        return;
    }

    if (!self->havePrevious) {
        self->prevStats = stats;
        self->havePrevious = true;
    }

    ssize_t usedMem = stats.usedSize / 1024;
    ssize_t freeMem = stats.freeSize / 1024;
    ssize_t totalMem = usedMem + freeMem;

    ssize_t diff = ((stats.usedSize + stats.freeSize) - (self->prevStats.usedSize + self->prevStats.freeSize)) / 1024;

    SvLogNotice("CubiTV: memory usage [kB]: total %zd (%s%zd), used %zd, free %zd",
                totalMem, (diff >= 0) ? "+" : "", diff, usedMem, freeMem);

    self->prevStats = stats;
}

/**
 * Print malloc() usage statistics per zone.
 *
 * This function lists all registered memory zones and their usage statistics.
 * Format of this information is JSON-based:
 *
 *   {
 *     "name": "memoryTrace",
 *     "zone": "<zone name>",
 *     "blocksCount": <number of allocated memory blocks in this zone>,
 *     "memoryUsed": <total amount of memory in bytes allocated in this zone>
 *   }
 *
 * @param[in]   self memory tracer handle
 * @return      @c true if whole stats was printed, @c false otherwise
 **/
SvLocal bool QBDebugUtilsPrintZoneStats(QBDebugUtilsMemoryTracer self)
{
    size_t maxCount = 0, count = (size_t) self->lastZonesCount;
    SvAllocatorZoneStats *stats = NULL;
    static ssize_t lastIndex = -1; ///< last examined index from stats.
                                   ///< If it's set on -1 it means that whole statistics
                                   ///< was printed.
    bool fullStatsPrinted = true;

    do {
        maxCount = count * 5 / 4;
        free(stats);
        stats = malloc(maxCount * sizeof(SvAllocatorZoneStats));
        if (!stats) {
            count = 0;
            break;
        }

        self->lastZonesCount = SvAllocatorGetZonesStats(SvDefaultAllocator, stats, maxCount);
        if (self->lastZonesCount <= 0) {
            count = 0;
            break;
        }

        count = (size_t) self->lastZonesCount;
    } while (count >= maxCount);

    for (size_t i = lastIndex + 1; i < count; i++) {
        SvLogNotice("{\"name\":\"memoryTrace\",\"zone\":\"%s\",\"blocksCount\":%zu,\"memoryUsed\":%zu}",
                    stats[i].name, stats[i].blocksCount, stats[i].totalUsed);
        if (SvFiberTimePassed()) {
            fullStatsPrinted = false;
            lastIndex = i;
            break;
        }
    }

    if (fullStatsPrinted)
        lastIndex = -1;

    free(stats);

    return fullStatsPrinted;
}

SvLocal void QBDebugUtilsMemoryTracerStep(void *self_)
{
    QBDebugUtilsMemoryTracer self = self_;
    bool fullStatsPrinted = true;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    if (self->lastZonesCount >= 0) {
        fullStatsPrinted = QBDebugUtilsPrintZoneStats(self);
    }

    // print malloc statistics on the end of zone statistics
    if (fullStatsPrinted) {
        QBDebugUtilsPrintMallocStats(self);
    }

    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(
                              fullStatsPrinted ? CUBITV_DEBUG_PRINT_OBJECTS_DELAY : CUBITV_DEBUG_PRINT_OBJECTS_DELAY_AFTER_TIME_PASS));
}

void QBDebugUtilsStartMemoryTrace(SvScheduler scheduler)
{
    QBDebugUtilsMemoryTracer tracer = QBDebugUtilsGetMemoryTracer();
    if (tracer->fiber) {
        SvLogError("%s(): already started", __func__);
        return;
    }
    tracer->fiber = SvFiberCreate(scheduler, NULL, "DebugMemoryTracer", QBDebugUtilsMemoryTracerStep, tracer);
    tracer->timer = SvFiberTimerCreate(tracer->fiber);
    SvFiberTimerActivateAfter(tracer->timer, SvTimeFromMs(CUBITV_DEBUG_PRINT_OBJECTS_DELAY / 2));
}

void QBDebugUtilsStopMemoryTrace(void)
{
    QBDebugUtilsMemoryTracer tracer = QBDebugUtilsGetMemoryTracer();
    if (!tracer->fiber) {
        SvLogError("%s(): not started", __func__);
        return;
    }
    SvFiberDestroy(tracer->fiber);
    tracer->fiber = NULL;
}

#else // SV_LOG_LEVEL is 0

// ----------------------------- SECURE PART SECTION --------------------------

// stub for secure version
void QBDebugUtilsStartMemoryTrace(SvScheduler scheduler)
{
}

// stub for secure version
void QBDebugUtilsStopMemoryTrace(void)
{
}

#endif // SV_LOG_LEVEL > 0
