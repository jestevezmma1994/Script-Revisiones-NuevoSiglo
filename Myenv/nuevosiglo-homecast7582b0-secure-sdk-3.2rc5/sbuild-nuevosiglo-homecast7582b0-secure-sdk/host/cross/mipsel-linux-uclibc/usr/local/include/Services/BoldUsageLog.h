#ifndef BOLD_USAGE_LOG_H_
#define BOLD_USAGE_LOG_H_

#include <syslog.h>
#include <main_decl.h>
#include <SvFoundation/SvString.h>

extern void BoldUsageLog(int pri, const char *fmt, ...);
extern void BoldUsageLogInitialize(AppGlobals app);
extern void BoldUsageLogClose(void);
extern void BoldUsageLogUpdateIds(SvString sc_id, SvString chip_id);
extern void BoldUsageLogSendInitialLog(void);
extern void BoldUsageLogStop(void);
#endif
