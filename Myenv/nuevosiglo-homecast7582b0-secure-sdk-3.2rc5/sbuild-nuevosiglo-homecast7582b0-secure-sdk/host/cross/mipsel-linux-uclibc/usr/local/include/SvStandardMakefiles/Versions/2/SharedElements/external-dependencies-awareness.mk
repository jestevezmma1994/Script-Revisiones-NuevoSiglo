# External dependencies are tracked with the pkg-config tool.
#
# I want three separate set of options from pkg-config: 
# CFLAGS  - C compiler and preprocessor options (which is usually CPPFLAGS used
#           to pass -I around)
# LDFLAGS - Linker options, usually -Lpathname but sometimes various arcane
# -rdynamic and other things that don't start with -L
# LDLIBS  - List of libraries something needs to link with: -lfoo -lbar
# 
# They are extracted to special variables to avoid calling $(shell) too often

.PHONY: check-dependencies describe-dependencies

SvHelp[check-dependencies]=Make sure that all dependencies are met
SvHelp[describe-dependencies]=Lists all dependencies and their alterations to relevant environment variables

SvPkgConfig ?= $(error You must define SvPkgConfig by including basic-compile-patterns.mk)

# Pkg-config usage functions
SvPackageCFLAGS = $(filter-out -I% -D%,$(shell $(SvPkgConfig) $1 --cflags $2))
SvPackageCPPFLAGS = $(filter -I% -D%,$(shell $(SvPkgConfig) $1 --cflags $2))
SvPackageLDFLAGS = $(filter-out -l%,$(shell $(SvPkgConfig) $1 --libs $2))
SvPackageLDLIBS = $(filter -l%,$(shell $(SvPkgConfig) $1 --libs-only-l $2))

ifneq ($(SvRequires),)
# Verify that each required package is indeed available on compile time!
check-dependencies:
	@$(call SvEchoCommand,Checking package dependencies...)
	@$(if $(findstring problem,$(shell $(SvPkgConfig) --print-errors --exists $(SvRequires) || echo problem)),\
		$(error Some packages are required but appear to be missing! Check pkg-config --list-all to see available packages. Check make describe-dependencies to learn about each particular item.))
# Explain each dependency
describe-dependencies:
	@$(call SvEchoCommand,Package $(SvName) depends on the following packages: $(SvRequires))
	@$(foreach dep,$(SvRequires),\
		$(if $(findstring ok,$(shell $(SvPkgConfig) --exists $(dep) && echo ok)),\
			$(call SvEchoCommand,* Package $(dep) is available with version $(shell $(SvPkgConfig) --modversion $(dep)))\
			$(call SvEchoCommand,   CFLAGS   = $(call SvPackageCFLAGS,,$(dep)))\
			$(call SvEchoCommand,   CPPFLAGS = $(call SvPackageCPPFLAGS,,$(dep)))\
			$(call SvEchoCommand,   LDFLAGS  = $(call SvPackageLDFLAGS,,$(dep)))\
			$(call SvEchoCommand,   LDLIBS   = $(call SvPackageLDLIBS,,$(dep)))\
		,\
			$(call SvEchoCommand,* Required package $(dep) is not available)))

# Save original LDLIBS before injecting dependencies.
SvRequiredLibraries := $(filter -l%,$(LDLIBS))

# Dependency injection!
compile: check-dependencies
CFLAGS += $(call SvPackageCFLAGS,,$(SvRequires))
CPPFLAGS += $(call SvPackageCPPFLAGS,,$(SvRequires))
LDFLAGS += $(call SvPackageLDFLAGS,,$(SvRequires))
LDLIBS += $(call SvPackageLDLIBS,,$(SvRequires))

else

# Explain each dependency
describe-dependencies:
	@$(call SvEchoCommand,Package $(SvName) does not depend on any packages)
endif


ifneq ($(SvPrivateRequires),)
CFLAGS += $(call SvPackageCFLAGS,--static,$(SvPrivateRequires))
CPPFLAGS += $(call SvPackageCPPFLAGS,--static,$(SvPrivateRequires))
LDFLAGS += $(call SvPackageLDFLAGS,--static,$(SvPrivateRequires))
ifneq ($(SvTargetPlatformOperatingSystemName),Darwin)
    LDLIBS += -Wl,-whole-archive $(call SvPackageLDLIBS,--static,$(SvPrivateRequires)) -Wl,-no-whole-archive
else
    LDLIBS += $(call SvPackageLDLIBS,--static,$(SvPrivateRequires))
endif
endif
