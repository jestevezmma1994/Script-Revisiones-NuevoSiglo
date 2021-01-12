#ifndef BOLD_FOX_TRACKING_H_
#define BOLD_FOX_TRACKING_H_


#include <main_decl.h>
#include <syslog.h>
#include <Services/BoldHttpApi.h>

#define BOLD_FOX_TRACKING_KEEPALIVE 120 // 2 minutos

typedef enum {
   QBPercentageDisplayed_NONE = 0,
   QBPercentageDisplayed_0,
   QBPercentageDisplayed_10,
   QBPercentageDisplayed_25,
   QBPercentageDisplayed_50,
   QBPercentageDisplayed_75,
   QBPercentageDisplayed_95
} QBPercentageDisplayed;

extern void BoldContentTrackingInitialize(AppGlobals app, BoldHttpApi api);
extern void BoldContentTrackingUpdateIds(SvString sc_id);
extern void BoldContentTrackingSendMessageLogin(void);
extern void BoldContentTrackingSendMessageKeepAlive(SvString uri, BoldHttpApiCallback keepAliveCallback, void* keepAliveData, SvString mwUri);
extern void BoldContentTrackingSendMessageCommand(SvString uri, SvString commandType, double position, SvString mwUri, int playbackTime);
extern void BoldContentTrackingSendMessageStartResume(SvString uri, SvString commandType, double position, int playbackTime, SvString mwUri, SvString pathinfo); //jhernanadez
extern void BoldContentTrackingSendMessageStop(SvString uri, SvString commandType, double position, SvString mwUri, int playbackTime, double currentBitrate, SvString finishStop);
extern void BoldContentTrackingSendMessageProgress(SvString uri, SvString commandType, double position, SvString mwUri, int playbackTime, int progress, double currentBitrate);
extern void BoldContentTrackingSendMessageJoinTime(SvString uri, SvString commandType, int playbackTime, SvString mwUri);
extern void BoldContentTrackingSendMessageBufferUnderrun(SvString uri, SvString commandType, double position, SvString mwUri, int duration);
extern void BoldContentTrackingSendMessageError(SvString uri, SvString errorCode, SvString message, double position, SvString mwUri, int playbackTime);
extern void BoldContentTrackingSendMessageSeek(SvString uri, int seekTime, SvString mwUri);

#endif
