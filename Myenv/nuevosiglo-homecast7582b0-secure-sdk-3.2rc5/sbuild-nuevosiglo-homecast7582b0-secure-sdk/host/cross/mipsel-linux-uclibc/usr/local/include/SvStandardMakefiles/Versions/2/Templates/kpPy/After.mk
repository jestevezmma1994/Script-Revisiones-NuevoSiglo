
SvDirectoryLayoutConvention ?= custom

ifeq ($(SvDirectoryLayoutConvention),custom)
  SvSourcesDir    ?= .
  SvHeadersDir    ?= .
else ifeq ($(SvDirectoryLayoutConvention),mac)
  SvSourcesDir    ?= Sources/
  SvHeadersDir    ?= Headers/
else ifeq ($(SvDirectoryLayoutConvention),flat)
  SvSourcesDir    ?= .
  SvHeadersDir    ?= .
else ifeq ($(SvDirectoryLayoutConvention),unix)
  SvSourcesDir    ?= src/
  SvHeadersDir    ?= include/
else
  $(error Invalid value of SvDirectoryLayoutConvention, allowed values are 'mac', 'unix' and 'custom')
endif

ifneq ($(SvBinPrefix),)
  kpPyBinPrefix := $(SvBinPrefix)
else
  kpPyBinPrefix := /usr/local/bin
endif

ifeq ($(SvMakefileTemplateOrig),Library)
  ifneq ($(origin SvLibraryHeaderFiles),undefined)
    kpPyHeaders := $(SvLibraryHeaderFiles)
  else
    SvLibraryHeaderFiles = $(shell find $(SvHeadersDir) -type f)
  endif
endif

ifeq ($(SvMakefileTemplateOrig),$(SvMakefileTemplate))
  $(error SvMakefileTemplateOrig is badly defined)
endif

ifeq ($(SvMakefileTemplateOrig),Library)
  kpPyLibraryName:=$(SvName)
  ifneq ($(kpPyMakeIgnoreLibraryVersions),yes)
    kpPyLibraryMajor:=$(SvMajorVersion)
    kpPyLibraryMinor:=$(SvMinorVersion)
    kpPyLibraryRevision:=$(SvRevision)
  endif
  ifneq ($(origin SvLibrarySourceFiles),undefined)
    kpPySrcFiles := $(SvLibrarySourceFiles)
  endif
  ifneq ($(SvProvides),)
    kpPyLibraryName := $(SvProvides)
  endif
  kpPyHeadersBaseDir := $(SvHeadersDir)
  kpPySrcBaseDir := $(SvSourcesDir)
endif

ifeq ($(SvMakefileTemplateOrig),Executable)
  kpPyExecName:=$(SvName)
  ifneq ($(origin SvExecutableSourceFiles),undefined)
    kpPySrcFiles := $(SvExecutableSourceFiles)
  endif
  kpPySrcBaseDir := $(SvSourcesDir)
endif

ifeq ($(SvMakefileTemplateOrig),Tests)
  ifneq ($(origin SvExecutablesSourceFiles),undefined)
    kpPyCustomExecNames:=$(patsubst %.c,%,$(SvExecutablesSourceFiles))
  else
    kpPyCustomExecNames:=$(patsubst %.c,%,$(wildcard *.c))
  endif
endif

kpPyPkgDeps := $(sort $(SvRequires))
kpPyPkgDepsPriv := $(sort $(SvPrivateRequires))

kpPyExportedFlags := $(SvPublicCompilerOptions)

CPPFLAGS += $(SvPublicCompilerOptions)
LDLIBS := $(SvPublicLinkerOptions) $(LDLIBS)

kpPyExtraTargets += $(SvInstallTargets)
#kpPyCleanExtraTargets += $(SvCleanTargets) ???

# ############################
# standard flags section:
# ############################

componentDirInode:=$(shell stat -c %i .)

kpPyTmpDir := $(SRM_ROOT)/_tmp/kpPyMake/$(SRM_COMPONENT_FILE)/$(componentDirInode)

TARGET_ROOT ?= $(SRM_DESTDIR)

# CROSS_ROOT should be already defined

#############################
# Translation flags
kpMessageCatalogFiles ?= $(SvMessageCatalogFiles)
SvTextDomain ?= $(SvName)
kpTextDomain ?= $(SvTextDomain)


#############################

CPPFLAGS += -D_GNU_SOURCE -DREENTRANT -D_FILE_OFFSET_BITS=64
# This *should* be taken care of by cross-gcc wrappers, but in some stupid makefiles, CC is overwritten,
# ... so until that is fixed, we have to add this here:
#CPPFLAGS += -I$(CROSS_ROOT)/usr/local/include -I$(CROSS_ROOT)/usr/include

#CPPFLAGS += -I/usr/local/include -I/usr/include

# Global logging level
ifeq ($(SV_LOG_LEVEL),)
  SV_LOG_LEVEL:=7
endif

ifeq ($(SvDisableAsserts),)
  ifeq ($(SV_LOG_LEVEL),0)
    CPPFLAGS += -DNDEBUG
  endif
else ifeq ($(SvDisableAsserts),yes)
  CPPFLAGS += -DNDEBUG
endif
CPPFLAGS += -DSV_LOG_LEVEL=$(SV_LOG_LEVEL)


# Do not copy dependencies of the shared libraries we are linking with
# as our own dependencies
LDFLAGS += -Wl,--no-add-needed

# not for host tools
ifeq ($(cross_compiling),yes)
  LDFLAGS += -Wl,--build-id
endif

LDFLAGS += -L$(CROSS_ROOT)/usr/local/lib


CFLAGS += -std=gnu99

FlagsCCxx :=
FlagsCCxx += -g3

ifneq ($(SvDebug),yes)
  FlagsCCxx += -Os
endif

FlagsCCxx += -Wall

# Extra warnings
FlagsCCxx += -Wwrite-strings -Winit-self -Wformat-security -Wuninitialized
ifeq ($(firstword $(sort 4.3 $(V_GCC))),4.3)
  # Enable extra warnings available in GCC 4.3.0 or newer
  FlagsCCxx += -Wlogical-op -Wtype-limits -Wempty-body -Wignored-qualifiers
endif
ifeq ($(firstword $(sort 4.6 $(V_GCC))),4.6)
  # Enable extra warnings available in GCC 4.6.0 or newer
  FlagsCCxx += -Wdouble-promotion -Wunused-but-set-parameter
endif
# Extra C-only warnings
FlagsC += -Wimplicit -Woverride-init -Wnested-externs -Wmissing-field-initializers
FlagsC += -Wold-style-definition
ifeq ($(firstword $(sort 4.3 $(V_GCC))),4.3)
  # Enable extra C-only warnings available in GCC 4.3.0 or newer
  FlagsC += -Wold-style-declaration -Wmissing-parameter-type
endif

SvStrictWarnings ?= yes
ifneq ($(SvStrictWarnings),no)
  FlagsCCxx += -Wredundant-decls
  FlagsC    += -Wredundant-decls -Wstrict-prototypes -Wmissing-declarations -Wshadow -Wsign-compare -Wpointer-arith -Wcast-align
  FlagsCxx  += -Wcast-align -Wredundant-decls -Wsign-compare -Wpointer-arith
endif

ifneq ($(SvErrorNotfatal),yes)
  FlagsCCxx += -Werror
endif

ifeq ($(SvKeepStack),yes)
  FlagsCCxx += -fno-omit-frame-pointer -funwind-tables -fno-exceptions
endif

CFLAGS   += $(FlagsCCxx) $(FlagsC)
CXXFLAGS += $(FlagsCCxx) $(FlagsCxx)



################################################
# extra targets that we define internally

kpPyExtraTargets += install-doc-sources install-sanity-targets install-translation-catalogs

################################################

# Find out the absolute path of THIS file (not necessarily CROSS_ROOT-based)
svMakePath := $(dir $(lastword $(MAKEFILE_LIST)))

include $(svMakePath)/../../../../../kpPyMake/kpPyMakeInc.mk

include $(svMakePath)/../../../../../kpPyMake/kpPyMakeDocs.mk

include $(svMakePath)/../../../../../kpPyMake/kpPyMakeSanityChecks.mk

include $(svMakePath)/../../../../../kpPyMake/kpPyMakeTranslations.mk
