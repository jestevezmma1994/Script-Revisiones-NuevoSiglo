# Support for building universal binaries on OS X.
#
# Building such binaries takes longer but catches _way_ more errors and is
# generally a good thing on a mac.
ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
  TARGET_ARCH = -arch x86_64
else
  TARGET_ARCH =
endif

# Fallback to empty for --warn-undefined-variables
LDFLAGS ?=
LDLIBS ?=
CFLAGS ?=
CXXFLAGS ?=
CPPFLAGS ?=
OBJCFLAGS ?=

# We use ISO C99 features with GNU extensions (for example: anonymous fields in
# structs and unions) everywhere. Some newer gcc versions turn C99 support on by
# default, older versions do not, so we have to do it explicitly.
CFLAGS += -std=gnu99

# By default we want to enable all standard warnings.
CFLAGS += -Wall
CXXFLAGS += -Wall

# Enable extra warning flags that work for C and C++.
# TODO: introduce -Wjump-misses-init
COMMON_WARNING_FLAGS += -Wwrite-strings -Winit-self -Wformat-security -Wuninitialized
ifeq ($(firstword $(sort 4.3 $(V_GCC))),4.3)
  # Enable warning flags available in GCC 4.3.0 or newer
  COMMON_WARNING_FLAGS += -Wlogical-op -Wtype-limits -Wempty-body -Wignored-qualifiers
endif
ifeq ($(firstword $(sort 4.6 $(V_GCC))),4.6)
  # Enable warning flags available in GCC 4.6.0 or newer
  COMMON_WARNING_FLAGS += -Wdouble-promotion -Wunused-but-set-parameter
endif
CFLAGS += $(COMMON_WARNING_FLAGS)
CXXFLAGS += $(COMMON_WARNING_FLAGS)

# Add C-only warning flags.
CFLAGS += -Wimplicit -Woverride-init -Wnested-externs -Wmissing-field-initializers
CFLAGS += -Wold-style-definition
ifeq ($(firstword $(sort 4.3 $(V_GCC))),4.3)
  # Enable C-only warning flags available in GCC 4.3.0 or newer
  CFLAGS += -Wold-style-declaration -Wmissing-parameter-type
endif

# Conditionally add more warning flags.
SvStrictWarnings ?= yes
ifneq ($(SvStrictWarnings),no)
  # TODO: introduce -Wsign-conversion and -Wbad-function-cast
  CXXFLAGS  += -Wcast-align -Wredundant-decls -Wsign-compare -Wpointer-arith
  CFLAGS    += -Wredundant-decls -Wstrict-prototypes -Wmissing-declarations -Wshadow -Wsign-compare -Wpointer-arith -Wcast-align
endif

ifneq ($(V_CLANG),)
  # Remove warning flags not supported by clang.
  GCC_ONLY_FLAGS += -Wlogical-op -Wdouble-promotion -Woverride-init
  GCC_ONLY_FLAGS += -Wold-style-declaration -Wmissing-parameter-type -Wunused-but-set-parameter
  CFLAGS := $(filter-out $(GCC_ONLY_FLAGS),$(CFLAGS))
  CXXFLAGS := $(filter-out $(GCC_ONLY_FLAGS),$(CXXFLAGS))
  # Explicitly disable some warnings that clang enables with -Wall.
  CFLAGS += -Wno-tautological-compare -Wno-unused-value -Wno-deprecated-declarations
  CXXFLAGS += -Wno-tautological-compare -Wno-unused-value -Wno-deprecated-declarations
endif

# This is a high-quality marker and it really does force people to fix those
# warnings that otherwise look innocent.
ifneq ($(SvErrorNonfatal),yes)
  CFLAGS += -Werror
  CXXFLAGS += -Werror
endif

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

# Use the 64 bit interface to low-level filesystem elements.
CPPFLAGS += -D_FILE_OFFSET_BITS=64

# Use GNU features like asprintf() or CLOCK_MONOTONIC.
ifeq ($(SvTargetPlatformOperatingSystemName),Linux)
  CPPFLAGS += -D_GNU_SOURCE
endif

# Objective-C support
COMPILE.m = $(CC) $(CFLAGS) $(OBJCFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
    OBJCFLAGS += -fnext-runtime -fconstant-string-class=NSConstantString
else
    OBJCFLAGS += -fgnu-runtime -fconstant-string-class=NSConstantString
endif

# Do not copy dependencies of the shared libraries we are linking with
# as our own dependencies.
ifeq ($(SvTargetPlatformOperatingSystemName),Linux)
    LDFLAGS += -Wl,--no-add-needed
endif

# Standard ar flags.
#
# I like to define them explicitly to get a nice set of features:
# c - create archive if doesn't exist
# s - update index (no ranlib needed)
# r - replace old versions
# u - only replace older than existing object files
ARFLAGS			= csru

# A set of filename generators and patterns.
SvPkgConfigFileGenerator        	= $1.pc
SvObjectFileGenerator           	= $1.o
SvExecutableFileGenerator       	= $1
SvExecutableSuffixPattern         	= 
SvStaticLibraryFileGenerator    	= lib$1.a
SvStaticLibrarySuffixPattern      	= *.a
ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
    # Shared library name patterns on Mac OS
    SvSharedLibraryFileGenerator    	= lib$1.dylib
    SvSharedLibrarySONameFileGenerator	= lib$1.$2.dylib
    SvSharedLibraryFullFileGenerator	= lib$1.$2.$3.$4.dylib
    SvSharedLibrarySuffixPattern      	= *.dylib
else ifneq ($(filter Linux SunOS,$(SvTargetPlatformOperatingSystemName)),)
    # Shared library name patterns on Linux and Solaris
    SvSharedLibraryFileGenerator    	= lib$1.so
    SvSharedLibrarySONameFileGenerator	= lib$1.so.$2.$3
    SvSharedLibraryFullFileGenerator	= lib$1.so.$2.$3.$4
    SvSharedLibrarySuffixPattern      	= .so*
else ifneq ($(filter FreeBSD,$(SvTargetPlatformOperatingSystemName)),)
    # Shared library name patterns on FreeBSD
    SvSharedLibraryFileGenerator    	= lib$1.so
    SvSharedLibrarySONameFileGenerator	= lib$1.so.$2
    SvSharedLibraryFullFileGenerator	= lib$1.so.$2
    SvSharedLibrarySuffixPattern      	= .so*
else
    # TODO: Add Win32 support (XXX: we need a supporter for this platform)
    # TODO: Add BSD support (on my TODO list, should be really easy)
    $(error Your platform is not recognized. Standard Makefiles cannot compile anything in this environment)
endif

# Rather easy macro that turns list of source files (with paths) to list of
# object files (with extended paths).
# It doesn't fit on 80 column display, sorry.
SvTurnSourcesToObjects = $(foreach base_file,$(basename $1),$(SvTemporaryDir)$(call SvObjectFileGenerator,$(base_file)))

# Rather easy macro that turns list of source files (with paths) to list of
# object files (without extended paths).
# It doesn't fit on 80 column display, sorry.
SvTurnSourcesToObjectsNoPath = $(foreach base_file,$(basename $1),$(call SvObjectFileGenerator,$(base_file)))

# Rather easy macro that turns list of source files (with paths to list of
# executable files (with extended paths).
# It doesn't fit on 80 column display, sorry.
SvTurnSourcesToExecutables = $(foreach base_file,$(basename $1),$(SvProductsDir)$(call SvExecutableFileGenerator,$(base_file)))

# Rather easy macro that turns list of source files (with paths to list of
# executable files (without extended paths).
# It doesn't fit on 80 column display, sorry.
SvTurnSourcesToExecutablesNoPath = $(foreach base_file,$(basename $1),$(call SvExecutableFileGenerator,$(base_file)))

# It seems that exported variables are not forwarded to $(shell ...) so this
# is necessary for pkg-config to work.
SvPkgConfigEnv		?=
SvPkgConfigFlags	?=
SvPkgConfig		= $(SvPkgConfigEnv) pkg-config $(SvPkgConfigFlags)

# Compilation of pure C files
$(SvTemporaryDir)$(call SvObjectFileGenerator,%): %.c
	@set -e
	@mkdir -p "$$(dirname $@)"
	@$(call SvShellCommand,$(COMPILE.c) $(OUTPUT_OPTION) $^,$(call SvTextGeneratorCompilingC,$(@:$(SvTemporaryDir)%=%)))

# Compilation of C++ files
$(SvTemporaryDir)$(call SvObjectFileGenerator,%): %.cpp
	@set -e
	@mkdir -p "$$(dirname $@)"
	@$(call SvShellCommand,$(COMPILE.cpp) $(OUTPUT_OPTION) $^,$(call SvTextGeneratorCompilingCxx,$(@:$(SvTemporaryDir)%=%)))
$(SvTemporaryDir)$(call SvObjectFileGenerator,%): %.cc
	@set -e
	@mkdir -p "$$(dirname $@)"
	@$(call SvShellCommand,$(COMPILE.cpp) $(OUTPUT_OPTION) $^,$(call SvTextGeneratorCompilingCxx,$(@:$(SvTemporaryDir)%=%)))

# Compilation of assembly files
$(SvTemporaryDir)$(call SvObjectFileGenerator,%): %.s
	@set -e
	@mkdir -p "$$(dirname $@)"
	@$(call SvShellCommand,$(COMPILE.s) $(OUTPUT_OPTION) $^,$(call SvTextGeneratorCompilingAsm,$(@:$(SvTemporaryDir)%=%)))

# Compilation of assembly files preprocessed with C preprocessor
$(SvTemporaryDir)$(call SvObjectFileGenerator,%): %.sx
	@set -e
	@mkdir -p "$$(dirname $@)"
	@$(call SvShellCommand,$(COMPILE.c) -x assembler-with-cpp $(OUTPUT_OPTION) $^,$(call SvTextGeneratorCompilingAsm,$(@:$(SvTemporaryDir)%=%)))

# Compilation of Objective C files
$(SvTemporaryDir)$(call SvObjectFileGenerator,%): %.m
	@set -e
	@mkdir -p "$$(dirname $@)"
	@$(call SvShellCommand,$(COMPILE.m) $(OUTPUT_OPTION) $^,$(call SvTextGeneratorCompilingObjc,$(@:$(SvTemporaryDir)%=%)))

# XXX: unused, move elsewhere
define SvDependencyRuleForSourceFile
$$(shell $$(CC) $$(CPPFLAGS) -MM $1)
endef
