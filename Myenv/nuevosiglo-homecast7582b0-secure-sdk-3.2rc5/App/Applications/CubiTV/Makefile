SvStandardMakefiles = $(CROSS_ROOT)/usr/local/include/SvStandardMakefiles
SvMakefileTemplate = Library
include $(SvStandardMakefiles)/Versions/2/Before.mk

SvName = CubiTV
SvDescription = CubiTV application core

SvMajorVersion = 1
SvMinorVersion = 0
SvRevision = 0

SvRequires = \
	SvCore error_log QBStringUtils fibers SvFoundation CAGE QBResourceManager CUIT SvSettingsManager swl\
	SvPlayerKit QBPCRatings autoplayer SvPlayerManager QBDLNAClient\
	QBCatManager QBCAS SvDRM QBSmartcard2-interface QBLatens QBViewRight QBViewRightIPTV-common QBCASCommonCryptoguard \
	QBStaticStorage QBConfig QBNetManager \
	QBSchedManager QBDecoder \
	QBDataModel3 XMB2 \
	QBContentManager TraxisWebClient\
	QBTunerFilter SvDataBucket2 SvJSON SvEPGDataLayer SvEPG\
	QBDvbSubsManager QBTeletextManager QBTeletextDisplayer QBVBITeletext\
	QBApplicationController QBCarousel QBOSK \
	QBSearch QBUpgrade QBDVBScanner\
	QBSecureLogManager iso-codes QBSubsManager QBScte27Subs QBM3uParser\
	QBIPC QBFSReader CutvManager NPvrManager QBShellCmds QBWebSocket QBOpenSSL \
	QBTextSubsManager QBExtSubsManager QBClosedCaptionSubsManager \
	QBReencryptionManager QuadriCast-MStoreVoD QBDVBMuxesMap \
	QBNATMonitor QBDiagnostics QBTunerReserver QBTunerMonitor QBDVBTableMonitor CubiwareMWClient \
	QBReencryptionManager TranslationMergerHeaders QBMenu QBConaxEMMPullService QBMWClient \
	QBUSBPerfTester QBCrypto QBCall2ActionClient QBPlayReadyManager QBConaxPlayReady QBDSMCC \
	QBServices QBDataProvider QBPVRProvider QBWidgets QBContentSource QBOSD QBNetworkMonitor QBXHTMLWidgets QBAppKit QBAppsManager QBCoreUtils \
	QBPlayerControllers QBDIALServerManager	QBWindows QBDRMManager SvPlayerControllers

SvQBShellCmdsWhitelists = ./QBShellCmdsWhitelist

ifneq ($(CUBITV_MALLOC_REPLACEMENT),)
    SvRequires += $(CUBITV_MALLOC_REPLACEMENT)
endif

SvDirectoryLayoutConvention = custom
AllFiles := $(shell find . -type f -a \( ! -wholename './Windows/Factory/*' -a ! -wholename './Tests/*' \) -a \( -name '*\.[csm]' -o -name '*\.cpp' -o -name '*\.sx' -o -name '*\.cc' \))
SvLibrarySourceFiles := $(AllFiles)
SvLibraryHeaderFiles := $(shell find . -type f -name '*.h')
SvLibraryHeaderFiles := $(filter-out ./.%,$(SvLibraryHeaderFiles))
SvLibraryHeaderFiles := $(filter-out ./Tests/%,$(SvLibraryHeaderFiles))
SvLibraryHeaderFiles := $(filter-out ./Hacks/%,$(SvLibraryHeaderFiles))
SvIncludePrefix := $(SvPrefix)/include/CubiTV

SvDocSourceFiles[external] = $(SvLibraryHeaderFiles)

CPPFLAGS += -I.

CFLAGS += -std=gnu99

LDLIBS += -lcap

SvStrictWarnings = yes

ifneq (,$(CUBITV_FACTORY_CONTEXT))
    CPPFLAGS += -D$(CUBITV_FACTORY_CONTEXT)=1
    SvLibrarySourceFiles += Windows/Factory/$(CUBITV_FACTORY_CONTEXT).c
    SvLibraryHeaderFiles += Windows/Factory/$(CUBITV_FACTORY_CONTEXT).h
endif

ifneq (,$(CUBITV_PVR_REENCRYPTION))
    CPPFLAGS += -DCUBITV_PVR_REENCRYPTION=\"$(CUBITV_PVR_REENCRYPTION)\"
endif

ifeq (${DISABLE_PTHREAD_EXIT},yes)
    CPPFLAGS += -DDISABLE_PTHREAD_EXIT
endif

include $(SvStandardMakefiles)/Versions/2/After.mk

QBTranslateSources=$(AllFiles) Hacks/broken_translations.h
include $(CROSS_ROOT)/usr/local/share/translations/Makefile.mk
