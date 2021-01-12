# This is the top-level entry point to standard makefiles.
#
# Thanks for reading it by the way. I did everything I could think of to make
# it easy for You to follow but I'm always looking forward to suggestions,
# improvements and patches at <zyga@sentivision.com>
#
# Zygmunt Krynicki
# --

#SHELL = /bin/bash

# Find the full path of this makefile.
#
# This is a little magic if you ask me but MAKEFILE_LIST tracks which makefiles
# have been included so far and this makefile (with a full path, thank you very
# much) is going to be the last item in that list. We just fetch the directory
# part of the last word (last element on the list).
SvStandardMakefilesInstance:=$(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

# Include the template-independent initialization file.
#
# It could as well be placed inline here but that would make reading this file
# more difficult. The primary task of 'Before.mk' (which you are reading) is to
# include other files. Putting other stuff aside makes it easier.
include $(wildcard $(SvStandardMakefilesInstance)Init.mk)

# Perform sanity checks on programmer's makefile.
#
# The programmer makefile must define SvMakefileTemplate. If missing, we will
# inform the programmer that there is something wrong with his makefile and
# point him to an appropriate page.
SvMakefileTemplate ?= $(error $(call SvUndefinedVariable,SvMakefileTemplate))

# Any external modifications, etc.
# Watch that it might redirect us to another template.
include $(sort $(wildcard $(SvStandardMakefilesInstance)Plugins/*.mk))

# Include the early initialization of the selected template
#
# That file contains no rules and has some default values (assigned with ?=) to
# help you out. Programmers can easily extend them in their makefile by
# appending values to proper variables or overwriting them altogether.
include $(SvStandardMakefilesInstance)Templates/$(SvMakefileTemplate)/Before.mk

# This is important for After.mk to be able to detect a common programmer
# error.
SvBeforeHasBeenIncluded=yes

# That's it - the rest of the work will happen once you include 'After.mk'.
