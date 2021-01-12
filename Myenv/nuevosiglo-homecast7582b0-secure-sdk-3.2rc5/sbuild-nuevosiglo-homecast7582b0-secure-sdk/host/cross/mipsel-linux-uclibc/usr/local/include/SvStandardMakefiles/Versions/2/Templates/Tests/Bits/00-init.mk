# This is the default target, compile
.PHONY: all
all: compile

# Learn about SILO
include $(SvStandardMakefilesInstance)/SharedElements/silo-awareness.mk


# Learn about basic shell commands
include $(SvStandardMakefilesInstance)/SharedElements/shell-commands.mk

# Learn basic knowledge about building C-like source files
include $(SvStandardMakefilesInstance)/SharedElements/basic-compile-patterns.mk

# Distinctions between release and debug builds.
include $(SvStandardMakefilesInstance)/SharedElements/release-awareness.mk

# Learn about workspace
include $(SvStandardMakefilesInstance)/SharedElements/workspace-awareness.mk



# Learn about external toolchain
include $(SvStandardMakefilesInstance)/SharedElements/external-toolchain-awareness.mk

# Define product file names (with full paths)
SvExecutableFiles:= $(call SvTurnSourcesToExecutablesNoPath,$(SvExecutablesSourceFiles))

