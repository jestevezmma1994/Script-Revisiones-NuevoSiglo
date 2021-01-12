#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "splash_script_load.h"
#include "QBSplash.h"
//#include "QBSplashVDC.h"
#include "QBSplashVDCInternal.h"

#define __profile_header(x) #x
#define _reg_header(x) __profile_header(splash_vdc_reg_##x.h)
#define _rul_source(x) __profile_header(splash_vdc_rul_##x.c)
#define _rul_header(x) __profile_header(splash_vdc_rul_##x.h)
#define reg_header(x) _reg_header(x)
#define rul_source(x) _rul_source(x)
#define rul_header(x) _rul_header(x)

#if !defined(PROFILE_NAME)
#error profile name not defined
#endif

#include rul_header(PROFILE_NAME)
#ifdef SPLASH_VERSION_2
#include rul_source(PROFILE_NAME)
#else
#include reg_header(PROFILE_NAME)
#endif // SPLASH_VERSION_2

#define __output_profile(x) QBSplashOutputProfile_ ## x
#define _output_profile(x) __output_profile(x)
#define OUTPUT_PROFILE _output_profile(PROFILE_NAME)

int main(int argc, char *argv[]) {
    QBSplashVDCHeader *header;
    QBSplashVDCTriggerMap *triggerMap;
    QBSplashVDCSurfaceInfo *surfaceInfo;
    QBSplashVDCDisplayInfo *displayInfo;
    uint32_t offset;
    uint32_t size;
#ifdef SPLASH_VERSION_2
    SplashData* splashData = GetSplashData();
    SplashSurfaceInfo *g_SplashSurfaceInfo = splashData->pSurfInfo;
    SplashDisplayInfo *g_SplashDisplayInfo = splashData->pDispInfo;
    SplashTriggerMap *g_aTriggerMap = splashData->pTrigMap;
    uint32_t *g_aulReg = splashData->pAulReg;
    uint32_t numTrigger = splashData->iNumTrigMap;
    uint32_t numMem = splashData->ulNumMem;
    uint32_t numSurface = splashData->iNumSurface;
    uint32_t numDisplay = splashData->iNumDisplay;
    uint32_t numAulReg = splashData->nAulReg;
    uint32_t iRulMemIdx = splashData->iRulMemIdx;
#else
    uint32_t numTrigger = (sizeof(g_aTriggerMap)/sizeof(struct stTriggerMap));
    uint32_t numMem = BSPLASH_NUM_MEM;
    uint32_t numSurface = BSPLASH_NUM_SURFACE;
    uint32_t numDisplay = BSPLASH_NUM_DISPLAY;
    uint32_t numAulReg = (sizeof(g_aulReg)/(2*sizeof(uint32_t)));
    uint32_t iRulMemIdx = g_iRulMemIdx;
#endif

    char *dump;

    if (argc != 2) {
        fprintf(stderr, "Usage: QBSplashVDCDump <output_file>\n");
        return 1;
    }

    offset = 0;
    size = sizeof(QBSplashVDCHeader); // account for header
    size += sizeof(QBSplashVDCSurfaceInfo) * numSurface;
    size += sizeof(QBSplashVDCDisplayInfo) * numDisplay;
    size += numAulReg * 2 * sizeof(uint32_t); // account for reg dump
    size += sizeof(QBSplashVDCTriggerMap) * numTrigger; // account for trigger map struct

    // account for list count arrays and the list entries
    for (int i = 0; i < numTrigger; ++i) {
        if (g_aTriggerMap[i].TriggerHwNum != -1) {
            size += (g_aTriggerMap[i].ListCountMaxIndex + 1) * sizeof(uint32_t);
            size += g_aTriggerMap[i].aListCountArray[g_aTriggerMap[i].ListCountMaxIndex] * sizeof(uint32_t);
        }
    }

    dump = malloc(size);

    header = (QBSplashVDCHeader*)dump;
    header->magic = QB_VDC_MAGIC;
    header->version = QBSplashVDCVersion_1;
    header->size = sizeof(header);
    header->iRulMemIdx = iRulMemIdx;
    header->ulNumTrigger = numTrigger;
    header->ulNumMem = numMem;
    header->ulNumSurface = numSurface;
    header->ulNumDisplay = numDisplay;
    header->ulNumReg = numAulReg;
    header->outputProfile = OUTPUT_PROFILE;

    // header
    offset += sizeof(QBSplashVDCHeader);
//    printf("%d %p\n", __LINE__, dump + offset);

    // surface infos
    for (int i = 0; i < numSurface; ++i) {
        surfaceInfo = (QBSplashVDCSurfaceInfo*)(dump + offset);
        surfaceInfo->ihMemIdx = g_SplashSurfaceInfo[i].ihMemIdx;
        surfaceInfo->ePxlFmt = g_SplashSurfaceInfo[i].ePxlFmt;
        surfaceInfo->ulWidth = g_SplashSurfaceInfo[i].ulWidth;
        surfaceInfo->ulHeight = g_SplashSurfaceInfo[i].ulHeight;
        surfaceInfo->ulPitch = g_SplashSurfaceInfo[i].ulPitch;
        memcpy(surfaceInfo->bmpFile, g_SplashSurfaceInfo[i].bmpFile, sizeof(surfaceInfo->bmpFile));
        offset += sizeof(QBSplashVDCSurfaceInfo);
    }
//    printf("%d %p\n", __LINE__, dump + offset);

    // display infos
    for (int i = 0; i < numDisplay; ++i) {
        displayInfo = (QBSplashVDCDisplayInfo*)(dump + offset);
        displayInfo->iSurIdx = g_SplashDisplayInfo[i].iSurIdx;
        displayInfo->eDspFmt = g_SplashDisplayInfo[i].eDspFmt;
        displayInfo->ulRdcScratchReg0 = g_SplashDisplayInfo[i].ulRdcScratchReg0;
        displayInfo->ulRdcScratchReg1 = g_SplashDisplayInfo[i].ulRdcScratchReg1;
        offset += sizeof(QBSplashVDCDisplayInfo);
    }
//    printf("%d %p\n", __LINE__, dump + offset);

    // register dump
    memcpy(dump + offset, g_aulReg, numAulReg * 2 * sizeof(uint32_t));
    offset += numAulReg * 2 * sizeof(uint32_t);
//    printf("%d %p\n", __LINE__, dump + offset);
//    printf("numReg: %d\n", header->ulNumReg);

    // trigger map
    triggerMap = (QBSplashVDCTriggerMap*)(dump + offset);
    offset += numTrigger * sizeof(QBSplashVDCTriggerMap);
//    printf("%d %p\n", __LINE__, dump + offset);
    for (int i = 0; i < numTrigger; ++i) {
        triggerMap[i].TriggerHwNum = g_aTriggerMap[i].TriggerHwNum;
        triggerMap[i].SlotNum = g_aTriggerMap[i].SlotNum;
        triggerMap[i].ListCountMaxIndex = g_aTriggerMap[i].ListCountMaxIndex;
        if (g_aTriggerMap[i].TriggerHwNum != -1) {
            uint32_t listCountLength = g_aTriggerMap[i].ListCountMaxIndex + 1;
            uint32_t listLength = g_aTriggerMap[i].aListCountArray[g_aTriggerMap[i].ListCountMaxIndex];

            triggerMap[i].aListCountArray = offset;
            memcpy(dump + offset, g_aTriggerMap[i].aListCountArray, listCountLength * sizeof(uint32_t));
            offset += listCountLength * sizeof(uint32_t);
//            printf("%d %p\n", __LINE__, dump + offset);

            triggerMap[i].aListArray = offset;
            memcpy(dump + offset, g_aTriggerMap[i].aListArray, listLength * sizeof(uint32_t));
            offset += listLength * sizeof(uint32_t);
//            printf("%d %p\n", __LINE__, dump + offset);
        } else {
            triggerMap[i].aListCountArray = 0;
            triggerMap[i].aListArray = 0;
        }
    }

    FILE *out = fopen(argv[1], "w");
    if (out) {
        fwrite(dump, size, 1, out);
    } else {
        fprintf(stderr, "Cannot open file\n");
        exit(1);
    }
    fclose(out);

    free(dump);

    return 0;
}
