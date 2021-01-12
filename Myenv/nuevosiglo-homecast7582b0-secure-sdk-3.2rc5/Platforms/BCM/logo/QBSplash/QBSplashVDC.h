#ifndef QBSPLASHVDC_H_
#define QBSPLASHVDC_H_

#include "splash_script_load.h"
#include "QBSplash.h"

extern uint32_t *g_pulReg;
extern uint32_t g_ulNumReg;

struct stTriggerMap
{
	int TriggerHwNum;          /* The hardware position of the trigger */
	int SlotNum;               /* The RDMA slot correposponding to the trigger */
	int ListCountMaxIndex;     /* The Max Index of the ListCount Array */
	uint32_t *aListCountArray; /* The list count array pointer */
	uint32_t *aListArray;   /* The list array pointer */
};

extern struct stTriggerMap *g_aTriggerMap;
extern uint32_t g_triggerCount;

int initOutputParams(QBSplash splash);

#endif // QBSPLASHVDC_H_
