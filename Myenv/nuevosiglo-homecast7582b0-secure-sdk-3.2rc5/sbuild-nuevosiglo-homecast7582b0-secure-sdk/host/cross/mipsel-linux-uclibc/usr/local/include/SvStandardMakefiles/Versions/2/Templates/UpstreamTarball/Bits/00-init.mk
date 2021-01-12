# This is the default target, compile
.PHONY: all compile
all: compile

# Learn about SILO/SRM
include $(SvStandardMakefilesInstance)/SharedElements/silo-awareness.mk


# Learn about basic shell commands
include $(SvStandardMakefilesInstance)/SharedElements/shell-commands.mk

# Learn some stuff about compiling to known about pkg-config files
include $(SvStandardMakefilesInstance)/SharedElements/basic-compile-patterns.mk

# Learn about workspace
include $(SvStandardMakefilesInstance)/SharedElements/workspace-awareness.mk

# Learn about external toolchain
include $(SvStandardMakefilesInstance)/SharedElements/external-toolchain-awareness.mk


# Internal variables
SvPkgConfigFile         = $(call SvPkgConfigFileGenerator,$(SvName))

# Cross compilation support!
ifdef CROSS_COMPILING

SvConfigureOptions      := $(filter-out --prefix=%,$(SvConfigureOptions))
SvConfigureOptions      += --prefix=$(SvPrefix)

# SILO/hosted mode:
# - use srm-local-run and override pkg-config to see hosted .pc files by unsetting cross_compiling
# - install stuff to BUILD_ROOT
ifeq ($(SvDoNotCrossCompile),yes)
SvSRMLocalRunIfNeeded   = . $(BUILD_ROOT)/bin/srm-local-run; export BUILD_ROOT=$(BUILD_ROOT); unset cross_compiling;
else
# SILO/cross mode:
# - use srm-local-run and override pkg-config to see only cross-compiled .pc
#   files by setting cross_compiling to yes
# - add local sbin to path (workaround for silo bug)
# - add cross bin and sbin to path to see various xxx-config scripts
# - install stuff to CROSS_ROOT
# - patch paths in pkg-config and libtool files
# - pass --host and --build options to configure script
#   (Host triplet is where you plan on running the built stuff)
#   (Build triplet is where you are currently building)
#define SvSRMLocalRunIfNeeded
#. $(BUILD_ROOT)/bin/srm-local-run;
define SvSRMLocalRunIfNeeded
export CROSS_ROOT=$(CROSS_ROOT); \
export cross_compiling=yes; \
export PATH=$(CROSS_ROOT)$(SvBinPrefix):$(CROSS_ROOT)$(SvSBinPrefix):$(BUILD_ROOT)$(SvSBinPrefix):$$PATH;
endef
SvHostTriplet           ?= $(error $(call SvUndefinedVariable,SvHostTriplet))
SvBuildTriplet          ?= $(error $(call SvUndefinedVariable,SvBuildTriplet))
SvConfigureOptions      += --host=$(SvHostTriplet)
SvConfigureOptions      += --build=$(SvBuildTriplet)
SvCompileTargets        += patch-paths
endif

else # !CROSS_COMPILING
# Normal hosted build without SILo
SvConfigureOptions      += --prefix=$(SvPrefix)
endif


