SvStandardMakefiles = $(CROSS_ROOT)/usr/local/include/SvStandardMakefiles
SvMakefileTemplate=Library
include $(SvStandardMakefiles)/Versions/2/Before.mk

SvName=QBTextSubsManager
SvDescription = manager for printing text subtitles

SvMajorVersion=1
SvMinorVersion=0
SvRevision=0

CPPFLAGS += -DSvLogModule=\"QBTextSubs\" -DSvLogModuleCustomized
SvDirectoryLayoutConvention = unix

SvRequires += SvPlayerManager SvPlayerControllers
SvRequires += CUIT QBSubsManager QBSMPTETTPngDecoder

include $(SvStandardMakefiles)/Versions/2/After.mk
