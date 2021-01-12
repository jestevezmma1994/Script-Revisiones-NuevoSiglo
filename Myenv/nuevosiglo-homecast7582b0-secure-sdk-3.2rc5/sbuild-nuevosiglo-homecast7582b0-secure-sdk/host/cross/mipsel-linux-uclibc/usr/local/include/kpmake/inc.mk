
ifneq ($(kpLibraryName),)
  kpPyLibraryName := $(kpLibraryName)
  ifneq ($(kpLibraryMajor),)
    kpPyLibraryMajor := $(kpLibraryMajor)
  endif
  ifneq ($(kpLibraryMinor),)
    kpPyLibraryMinor := $(kpLibraryMinor)
  endif
  ifneq ($(kpLibraryRevision),)
    kpPyLibraryRevision := $(kpLibraryRevision)
  endif
endif
ifneq ($(kpExecName),)
  kpPyExecName := $(kpExecName)
endif
ifneq ($(kpCustomExecNames),)
  kpPyCustomExecNames := $(kpCustomExecNames)
  $(foreach NAME,$(kpCustomExecNames),$(eval kpPyCustomExecSources_$(NAME):=$(kpCustomExecSources[$(NAME)])))
endif

ifneq ($(kpBinPrefix),)
  kpPyBinPrefix := $(kpBinPrefix)
else
  kpPyBinPrefix := /usr/local/bin/
endif

ifneq ($(kpSrcBaseDir),)
  kpPySrcBaseDir := $(kpSrcBaseDir)
endif
ifneq ($(origin kpSrcFiles),undefined)
  kpPySrcFiles := $(kpSrcFiles)
endif

ifneq ($(kpHeadersBaseDir),)
  kpPyHeadersBaseDir := $(kpHeadersBaseDir)
endif
ifneq ($(origin kpHeaderFiles),undefined)
  kpPyHeaders := $(kpHeaderFiles)
endif

kpPyPkgDeps := $(sort $(kpLibsDep[pub])  $(filter-out $(kpLibsDep[pub]),$(kpLibsDep)))
kpPyPkgDepsPriv := $(sort $(kpLibsDep[priv]))

kpPyExportedFlags := $(kpFlagsExported)

LDLIBS += $(kpLibs)

kpPyExtraTargets := $(kpInstallExtraTargets)

# ############################
# standard flags section:
# ############################

componentDirInode:=$(shell stat -c %i .)

kpPyTmpDir := $(SRM_ROOT)/_tmp/kpPyMake/$(SRM_COMPONENT_FILE)/$(componentDirInode)

TARGET_ROOT ?= $(SRM_DESTDIR)

# CROSS_ROOT should be already defined

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
ifeq ($(SV_LOG_LEVEL),0)
  CPPFLAGS += -DNDEBUG
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
FlagsCCxx += -Wwrite-strings -Winit-self -Wformat-security
ifeq ($(firstword $(sort 4.3 $(V_GCC))),4.3)
  # Enable extra warnings available in GCC 4.3.0 or newer
  FlagsCCxx += -Wlogical-op -Wtype-limits
endif
ifeq ($(firstword $(sort 4.6 $(V_GCC))),4.6)
  # Enable extra warnings available in GCC 4.6.0 or newer
  FlagsCCxx += -Wdouble-promotion -Wunused-but-set-parameter
endif
# Extra C-only warnings
FlagsC += -Wimplicit -Woverride-init -Wnested-externs
FlagsC += -Wold-style-definition
ifeq ($(firstword $(sort 4.3 $(V_GCC))),4.3)
  # Enable extra C-only warnings available in GCC 4.3.0 or newer
  FlagsC += -Wold-style-declaration -Wmissing-parameter-type
endif

kpStrictWarnings ?= yes
ifneq ($(kpStrictWarnings),no)
  FlagsCCxx += -Wredundant-decls
  FlagsC    += -Wstrict-prototypes -Wmissing-declarations
  FlagsCxx  += -Wcast-align
  ifneq ($(kpStrictWarnings),lenient)
    FlagsCCxx += -Wsign-compare -Wpointer-arith
    FlagsC += -Wcast-align
  endif
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

kpPyExtraTargets += install-doc-sources install-sanity-targets

################################################

# Find out the absolute path of THIS file (not necessarily CROSS_ROOT-based)
kpMakePath := $(dir $(lastword $(MAKEFILE_LIST)))

include $(kpMakePath)/../kpPyMake/kpPyMakeInc.mk

include $(kpMakePath)/../kpPyMake/kpPyMakeDocs.mk

include $(kpMakePath)/../kpPyMake/kpPyMakeSanityChecks.mk

include $(kpMakePath)/../kpPyMake/kpPyMakeTranslations.mk

include $(kpMakePath)/../kpPyMake/kpPyMakeQBShellCmds.mk
