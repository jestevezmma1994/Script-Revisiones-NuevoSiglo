# This is the default target, compile
.PHONY: all
all: compile


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
SvExecutableFile     		:= $(call SvExecutableFileGenerator,$(SvName))

