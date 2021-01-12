#include "BoldUsageLog.h"


#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)syslog.c    5.28 (Berkeley) 6/27/90";
#endif /* LIBC_SCCS and not lint */

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it outputs to the
 * log file instead of the standard output.  Also:
 *    adds a timestamp,
 *    prints the module name in front of the message,
 *    has some other formatting types (or will sometime),
 *    adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by syslogd(8).
 *
 * Author: Eric Allman
 * Modified to use UNIX domain IPC by Ralph Campbell
 *
 * Sat Dec 11 11:58:31 CST 1993: Dr. Wettstein
 *    Changes to allow compilation with no complains under -Wall.
 *
 * Thu Jan 18 11:16:11 CST 1996: Dr. Wettstein
 *    Added patch to close potential security hole.  This is the same
 *    patch which was announced in the linux-security mailing lists
 *    and incorporated into the libc version of syslog.c.
 *
 * Sun Mar 11 20:23:44 CET 2001: Martin Schulze <joey@infodrom.ffis.de>
 *    Use SOCK_DGRAM for loggin, renables it to work.
 *
 * Wed Aug 27 17:48:16 CEST 2003: Martin Schulze <joey@Infodrom.org>
 *    Improved patch by Michael Pomraning <mjp@securepipe.com> to
 *    reconnect klogd to the logger after it went away.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <paths.h>
#include <stdio.h>
#include <stdbool.h>
#include <QBConfig.h>
#include <SvFoundation/SvGenericObject.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <errno.h>
#include <main_decl.h>
#include <main.h>




#define    _PATH_LOGNAME    "/dev/log"

static int    LogFile = -1;                /* fd for log */
static int    connected;                   /* have done connect */
static int    LogStat = 0;                 /* status bits, set by openlog() */
static const char *LogTag = "syslog";      /* string to tag the entry with */
static int    LogFacility = LOG_USER;      /* default facility code */



static bool enabled = false;
static SvString destinationAddress;
static SvString scid;
static SvString chipId;
static SvString identifier;
static uint16_t port;
static AppGlobals appGlobals;

static bool initialLogSended = false;
SvFiber initialLogFiber = NULL;
SvFiberTimer initialLogTimer = NULL;
static int initialLogCount = 0;

/*
 * CLOSELOG -- close the system log
 */
void
BoldUsageLogClose(void)
{
    (void) close(LogFile);

    LogFile = -1;
    connected = 0;
}

bool BoldUsageLogOpen(const char *ident,int logstat,int logfac);
void BoldUsageLogVsyslog(int pri,const char *fmt,va_list ap);

void BoldUsageLogInitialize(AppGlobals app)
{
    appGlobals = app;
    const char *tmp;
    if ((tmp = QBConfigGet("USAGELOGS.ENABLED")) && strcmp(tmp, "enabled") == 0)
        enabled = true;

    if (enabled)
    {
        identifier = QBConfigGet("USAGELOGS.IDENTIFIER") ? SvStringCreate(QBConfigGet("USAGELOGS.IDENTIFIER"), NULL) : SVSTRING("BOLD_USAGE");
        destinationAddress = QBConfigGet("USAGELOGS.SERVER") ? SvStringCreate(QBConfigGet("USAGELOGS.SERVER"), NULL) : SVSTRING("127.0.0.1");
        port = QBConfigGet("USAGELOGS.PORT") ? atoi(QBConfigGet("USAGELOGS.PORT")) : 514;
        CubiwareMWCustomerInfoStatus status =  CubiwareMWCustomerInfoGetStatus(appGlobals->customerInfoMonitor);
        if (status == CubiwareMWCustomerInfoStatus_ok)
        {
            BoldUsageLogOpen(SvStringCString(identifier), LOG_ODELAY, LOG_USER);
        }
    }
}

void BoldUsageLogUpdateIds(SvString sc_id, SvString chip_id)
{
    if(sc_id && !SvStringEqualToCString(sc_id,"unknown")) /// < GRIAL: [#2525] - Reporte vía syslog de eventos tuner con SCID = "Unknown"
    {
        SVTESTRELEASE(scid);
        scid = SVRETAIN(sc_id);
    }

    if(chip_id)
    {
        SVTESTRELEASE(chipId);
        chipId = SVRETAIN(chip_id);
    }
}

SvLocal void BoldUsageLogSendInitialLogStep(void)
{
    initialLogCount++;
    SvFiberDeactivate(initialLogFiber);
    SvFiberEventDeactivate(initialLogTimer);
    
    if (initialLogSended || (initialLogCount > 5 * 2 * 60)) // Si se ha enviado el log, o si se ha intentado durante 5 minutos
    {
        SvFiberDestroy(initialLogFiber);
        initialLogFiber = NULL;
        return;
    }
    
    CubiwareMWCustomerInfoStatus status =  CubiwareMWCustomerInfoGetStatus(appGlobals->customerInfoMonitor);
    if (status != CubiwareMWCustomerInfoStatus_ok)
    {
        SvLogNotice("%s(): Wait to send initial log", __func__);
        SvFiberTimerActivateAfter(initialLogTimer, SvTimeFromMs(500));
    }
    else
    {
        BoldUsageLog(LOG_INFO, "\"Boot\":\"Success\"");
        SvLogNotice("%s(): Message Sended (END)", __func__);
        initialLogSended = true;
        SvFiberDestroy(initialLogFiber);
        initialLogFiber = NULL;
    }
    
}

void BoldUsageLogSendInitialLog(void)
{
    if (!enabled)
    {
        return; // Si no está habilitado no envía el log inicial
    }
    
    if (!initialLogSended && !initialLogFiber)
    {
        initialLogFiber = SvFiberCreate(SvSchedulerGet(), NULL, "BoldUsageLogSendInitialLog",
                                       (SvFiberStepFun*) &BoldUsageLogSendInitialLogStep, NULL);
        initialLogTimer = SvFiberTimerCreate(initialLogFiber);
        
        SvLogNotice("%s(): Setting Fiber to send message (BEGIN)", __func__);
        initialLogCount = 0;
        SvFiberTimerActivateAfter(initialLogTimer, SvTimeFromMs(100));
    }
}

void
BoldUsageLog(int pri, const char *fmt, ...)
{
    if(enabled)
    {
        CubiwareMWCustomerInfoStatus status =  CubiwareMWCustomerInfoGetStatus(appGlobals->customerInfoMonitor);
        if (status != CubiwareMWCustomerInfoStatus_ok)
        {
            return;
        }
        
        va_list ap;
        va_start(ap, fmt);
        BoldUsageLogVsyslog(pri, fmt, ap);
        va_end(ap);
    }
}

void
BoldUsageLogVsyslog(int pri, const char *fmt, va_list ap)
{
    register int cnt;
    register char *p;
    time_t now;
    int fd, saved_errno;
    int result;
    char tbuf[2048], fmt_cpy[1024];

    saved_errno = errno;

    /* see if we should just throw out this message */
    if (!LOG_MASK(LOG_PRI(pri)) || (pri &~ (LOG_PRIMASK|LOG_FACMASK)))
        return;

    /* check if socket created and connected*/
    if (LogFile < 0 || !connected)
    {
        if (!BoldUsageLogOpen(SvStringCString(identifier), LogStat | LOG_NDELAY, LogFacility))
        {
            return;
        }
    }

    /* set default facility if none specified */
    if ((pri & LOG_FACMASK) == 0)
        pri |= LogFacility;

    /* build the message */
    (void)time(&now);
    (void)sprintf(tbuf, "<%d>%.15s ", pri, ctime(&now) + 4);
    for (p = tbuf; *p; ++p);

    /*Open { for json & Add " to logTag*/
    (void)strcpy(p, "{\"");
    for (; *p; ++p);

    if (LogTag)
    {
        (void)strcpy(p, LogTag);
        for (; *p; ++p);
        /*Add " to logTag*/
        *p++ = '\"';
    }

    if (LogStat & LOG_PID)
    {
        (void)sprintf(p, "[%d]", getpid());
        for (; *p; ++p);
    }

    if (LogTag)
    {
        *p++ = ':';
        *p++ = ' ';
    }

    /*Add scid & chipId information */
    (void)sprintf(p,"{\"chip_id\":\"%s\",\"smartcard_id\":\"%s\", ", chipId ? SvStringCString(chipId) : "unknown", scid ? SvStringCString(scid) : "unknown");
    for (; *p; ++p);

    /* substitute error message for %m */
    {
        register char ch, *t1, *t2;
        //char *strerror();

        for (t1 = fmt_cpy;
                (ch = *fmt) != '\0' && t1<fmt_cpy+sizeof(fmt_cpy);
                ++fmt)
            if (ch == '%' && fmt[1] == 'm')
            {
                ++fmt;
                for (t2 = strerror(saved_errno);
                    (*t1 = *t2++); ++t1);
            }
            else
                *t1++ = ch;
        *t1 = '\0';
    }

    /*copy message from input */
    (void)vsprintf(p, fmt_cpy, ap);
    for (; *p; ++p);

    /*Close } at the end of JSON*/
    (void)strcpy(p, "}}");
    for (; *p; ++p);

    cnt = strlen(tbuf);

    /* output the message to the local logger */
    result = write(LogFile, tbuf, cnt + 1);

    if (result == -1
        && (errno == ECONNRESET || errno == ENOTCONN || errno == ECONNREFUSED))
    {
        BoldUsageLogClose();
        if (!BoldUsageLogOpen(SvStringCString(identifier), LogStat | LOG_NDELAY, LogFacility))
        {
            return;
        }
        result = write(LogFile, tbuf, cnt + 1);
    }

    if (result >= 0 || !(LogStat&LOG_CONS))
        return;

    /*
     * output the message to the console; don't worry about
     * blocking, if console blocks everything will.
     */
    if ((fd = open(_PATH_CONSOLE, O_WRONLY|O_NOCTTY, 0)) < 0)
        return;
    (void)strcat(tbuf, "\r\n");
    cnt += 2;
    p = index(tbuf, '>') + 1;
    (void)write(fd, p, cnt - (p - tbuf));
    (void)close(fd);
}

static struct sockaddr_in mySyslogAddr;    /* AF_UNIX address of local logger */
/*
 * OPENLOG -- open system log
 */
bool
BoldUsageLogOpen(const char *ident, int logstat, int logfac)
{
    if(enabled)
    {
        if (ident != NULL)
            LogTag = ident;
        LogStat = logstat;

        if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
            LogFacility = logfac;

        if (LogFile == -1 || !connected)
        {
            struct addrinfo hints, *servinfo = NULL, *p = NULL;
            int rv;

            memset(&hints, 0, sizeof (struct addrinfo));

            hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
            hints.ai_socktype = SOCK_DGRAM;


            if (LogStat & LOG_NDELAY)
            {
                LogFile = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            }

            mySyslogAddr.sin_family      = AF_INET;
            mySyslogAddr.sin_port        = htons(port);

            if ((rv = getaddrinfo( SvStringCString(destinationAddress) , NULL , &hints , &servinfo)) != 0)
            {
                return false;
                //inet_aton(SvStringCString(destinationAddress), &mySyslogAddr.sin_addr);
            }
            else
            {
                /* Loop through all the results and connect to the first we can */
                for(p = servinfo; p != NULL; p = p->ai_next)
                {
                    struct sockaddr_in *tmpAddr;
                    memcpy(&(tmpAddr), &p->ai_addr, sizeof(p->ai_addr));
                    memcpy(&(mySyslogAddr.sin_addr), &tmpAddr->sin_addr, sizeof(tmpAddr->sin_addr));

                    int connection_result =  connect(LogFile, (struct sockaddr *) &mySyslogAddr, sizeof(struct sockaddr));
                    if (LogFile != -1 && !connected &&
                            connection_result != -1)
                    {
                        connected = 1;
                    }
                }
            }

            freeaddrinfo(servinfo); // all done with this structure
            freeaddrinfo(p); // all done with this structure
        }
        
        return true;
    }
    
    return false;
}

static int    LogMask = 0xff;        /* mask of priorities to be logged */
/*
 * SETLOGMASK -- set the log mask level
 */
int
setlogmask(int pmask)
{
    int omask;

    omask = LogMask;
    if (pmask != 0)
        LogMask = pmask;
    return (omask);
}

void BoldUsageLogStop(void)
{
    if (initialLogTimer && initialLogFiber)
    {
        SvFiberEventDeactivate(initialLogTimer);
    }
    if (initialLogFiber)
    {
        SvFiberDeactivate(initialLogFiber);
        SvFiberDestroy(initialLogFiber);
    }
    initialLogSended = true;
}
