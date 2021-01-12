# This is the default target, compile
.PHONY: all
all: compile

# The order of -I options is very significant. It allows you to rebuild
# a library against the local headers (thay may be changed) instead of the
# installed headers.

# Include the headers directory if it is defined.
include $(SvStandardMakefilesInstance)/SharedElements/headers-directory-awareness.mk


# Learn about SILO
include $(SvStandardMakefilesInstance)/SharedElements/silo-awareness.mk

# Learn about basic shell commands
include $(SvStandardMakefilesInstance)/SharedElements/shell-commands.mk

# Learn basic knowledge about building C-like source files
include $(SvStandardMakefilesInstance)/SharedElements/basic-compile-patterns.mk

# Learn about distinctions between release and debug builds.
include $(SvStandardMakefilesInstance)/SharedElements/release-awareness.mk

# Learn about workspace
include $(SvStandardMakefilesInstance)/SharedElements/workspace-awareness.mk



# Learn about external toolchain
include $(SvStandardMakefilesInstance)/SharedElements/external-toolchain-awareness.mk

# Learn about external dependencies
include $(SvStandardMakefilesInstance)/SharedElements/external-dependencies-awareness.mk


# Define product file names (without full paths)
SvProvides              	?= $(SvName)
SvStaticLibraryFile     	:= $(call SvStaticLibraryFileGenerator,$(SvName),$(SvMajorVersion),$(SvMinorVersion),$(SvRevision))
SvSharedLibraryFile     	:= $(call SvSharedLibraryFileGenerator,$(SvName),$(SvMajorVersion),$(SvMinorVersion),$(SvRevision))
SvSharedLibraryMajorFile	:= $(call SvSharedLibrarySONameFileGenerator,$(SvName),$(SvMajorVersion),$(SvMinorVersion),$(SvRevision))
SvSharedLibraryFullFile 	:= $(call SvSharedLibraryFullFileGenerator,$(SvName),$(SvMajorVersion),$(SvMinorVersion),$(SvRevision))
SvSharedLibrarySONameFile	:= $(call SvSharedLibrarySONameFileGenerator,$(SvProvides),$(SvMajorVersion),$(SvMinorVersion),$(SvRevision))
SvPkgConfigFile         	:= $(call SvPkgConfigFileGenerator,$(SvProvides))
SvPkgConfigUninstalledFile	:= $(call SvPkgConfigFileGenerator,$(SvProvides)-uninstalled)
SvUnitTestFile          	:= $(call SvExecutableFileGenerator,unit-test)

# Decide what to build depending on user settings
ifeq ($(SvCompileUnitTests),yes)
SvCompileTargets        += compile-unit-tests
endif
ifeq ($(SvCompileStaticLibrary),yes)
SvInstallTargets        += install-static-library
SvCompileTargets        += compile-static-library
endif
ifeq ($(SvCompileSharedLibrary),yes)
SvInstallTargets        += install-shared-library
SvCompileTargets        += compile-shared-library
ifdef SvEnableSanityChecks
  SvInstallTargets      += check-library-sanity
endif
endif

ifeq ($(SvExtractRTTI),yes)
SvCompileTargets        += extract-rtti
endif

# Darwin prohibits common symbols in dynamic libraries.
ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
CFLAGS			+= -fno-common
endif

ifneq ($(SvTargetPlatformOperatingSystemName),MINGW32_NT-5.1)
# Generate position-independed-code for all libraries. This is necessary for
# AMD64 (even for static libraries) and simplifies the code generation proces
# greatly (no mulitple builds of one thing for all possible configurations).
CFLAGS			+= -fPIC
CXXFLAGS		+= -fPIC
endif

ifeq ($(SvTargetPlatformOperatingSystemName),Linux)
LDFLAGS			+= -Wl,-Bsymbolic-functions
endif

# This regular expression must match all valid public symbols
# By default the sv_ or sv prefix is valid but in specific cases
# it's okay to override it to something different.
SvLegalSymbolPattern = ^[S]v_?[a-zA-Z0-9_]+$
