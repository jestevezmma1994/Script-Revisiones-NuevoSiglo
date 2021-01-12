# This is the common intialization file for standard makefiles.
#
# If you want to know how standard makefiles work then please read the
# 'Before.mk' file as it is the real entry point and the best place to learn.

# Define the target and source platform.
#
# This is ill-defined for now and should be much more smarter when
# cross-compiling. It doesn't hurt that much as linux->linux is okay and
# darwin->linux doesn't work in cross-compile mode yet.
#
# NOTE: be wary of happily using switches to uname, many are not portable and
# will break on other OSes!


# Source platform details (we assume normal compilation by default)
SvSourcePlatformTriple                  := $(shell uname -s -r -p)
SvSourcePlatformOperatingSystemName     := $(word 1,$(SvSourcePlatformTriple))
SvSourcePlatformOperatingSystemRelease  := $(word 2,$(SvSourcePlatformTriple))
SvSourcePlatformProcessorArchitecture   := $(word 3,$(SvSourcePlatformTriple))

# Target platform details (we assume normal compilation by default)
SvTargetPlatformProcessorArchitecture   ?= $(SvSourcePlatformProcessorArchitecture)
SvTargetPlatformOperatingSystemName     ?= $(SvSourcePlatformOperatingSystemName)
SvTargetPlatformOperatingSystemRelease  ?= $(SvSourcePlatformOperatingSystemRelease)

# Target and source platform
SvSourcePlatform                        = $(SvSourcePlatformProcessorArchitecture)-$(SvSourcePlatformOperatingSystemName)-$(SvSourcePlatformOperatingSystemRelease)
SvTargetPlatform                        = $(SvTargetPlatformProcessorArchitecture)-$(SvTargetPlatformOperatingSystemName)-$(SvTargetPlatformOperatingSystemRelease)

# Assign default prefix and path configurations.
#
# This is used by various install targets.
SvPrefix                        ?= /usr/local
SvBinPrefix                     ?= $(SvPrefix)/bin
SvSBinPrefix                    ?= $(SvPrefix)/sbin
SvLibPrefix                     ?= $(SvPrefix)/lib
SvIncludePrefix                 ?= $(SvPrefix)/include
SvDataPrefix                    ?= $(SvPrefix)/share
SvPrivateDataPrefix             ?= $(SvDataPrefix)/$(SvName)
SvSysConfDir                    ?= $(SvPrefix)/etc/
SvManPrefix                     ?= $(SvDataPrefix)/man
SvDocPrefix                     ?= $(SvDataPrefix)/doc/$(SvName)

# Assign important immutable variables.
#
# These important variables must not be changed. All building takes place in a
# subdirectory of $(SvBuildDir) directory. 
SvNowDir                        := $(shell pwd)/
SvBuildDirPrefix                 = $(SRM_ROOT)/_tmp/SvStdMk/$(dir SRM_COMPONENT_FILE)/
ifeq ($(SvTargetPlatformOperatingSystemName),Linux)
 SvComponentDirInode            := $(shell stat --format '%i' .)
else
 SvComponentDirInode            := $(shell stat -f '%i' .)
endif
SvBuildDir                      ?= $(SRM_ROOT)/_tmp/SvStdMk/$(SRM_COMPONENT_FILE)/$(SvComponentDirInode)/
SvSourceSpecificBuildDir         = $(SvBuildDir)/S/
SvTargetSpecificBuildDir         = $(SvBuildDir)/T/

# Some relative variables
SvTemporaryDir                  = $(SvTargetSpecificBuildDir)Tmp/
SvObjectsDir                    = $(SvTargetSpecificBuildDir)Obj/
SvStampsDir                     = $(SvTargetSpecificBuildDir)Stamps/
SvProductsDir                   = $(SvTargetSpecificBuildDir)Products/

# Learn about terminal colors
include $(SvStandardMakefilesInstance)/SharedElements/terminal-colors.mk

# Get preferences!
-include $(HOME)/.SvStandardMakefiles.mk

# Load output strings
include $(SvStandardMakefilesInstance)/Localization/English.mk

# Display short error message about undefined variable
SvUndefinedVariable=The variable $(1) is not defined
