#ifndef QBSPLASHVDCINTERNAL_H_
#define QBSPLASHVDCINTERNAL_H_

typedef enum {
    QBSplashVDCVersion_undefined = 0,
    QBSplashVDCVersion_1,
} QBSplashVDCVersion;

#define QB_VDC_MAGIC    (0x44525142)

typedef struct __attribute__((__packed__)) QBSplashVDCHeader_ {
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    int iRulMemIdx;
    uint32_t ulNumTrigger;
    uint32_t ulNumMem;
    uint32_t ulNumSurface;
    uint32_t ulNumDisplay;
    uint32_t ulNumReg;
    uint32_t outputProfile;
} QBSplashVDCHeader;

typedef struct __attribute__((__packed__)) QBSplashVDCTriggerMap_ {
    int TriggerHwNum;
    int SlotNum;
    int ListCountMaxIndex;
    uint32_t aListCountArray; // offset
    uint32_t aListArray; // offset
} QBSplashVDCTriggerMap;

// These come from splash_script_load.h but were copied here to add the __packed__ attribute
typedef struct __attribute__((__packed__)) QBSplashVDCSurfaceInfo_ {
    int                    ihMemIdx;        /* this surface is allocated from *(phMem+ihMemIdx) */
    uint32_t               ePxlFmt;         /* pixel format */
    uint32_t               ulWidth;
    uint32_t               ulHeight;
    uint32_t               ulPitch;
    char                   bmpFile[256];    /* bmp file in splashgen */
} QBSplashVDCSurfaceInfo;

typedef struct __attribute__((__packed__)) QBSplashVDCDisplayInfo_ {
    int                    iSurIdx;        /* uses surface buffer from *(phHeap+ihMemIdx) */
    uint32_t               eDspFmt;        /* display video format */
    uint32_t               ulRdcScratchReg0;
    uint32_t               ulRdcScratchReg1;
} QBSplashVDCDisplayInfo;

#endif // QBSPLASHVDCINTERNAL_H_
