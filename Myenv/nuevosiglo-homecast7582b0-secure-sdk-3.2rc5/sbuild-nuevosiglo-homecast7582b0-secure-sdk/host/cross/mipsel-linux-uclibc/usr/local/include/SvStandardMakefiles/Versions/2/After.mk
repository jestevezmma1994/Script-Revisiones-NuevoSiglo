# This is the footer file for standard makefiles.
#
# If you want to know how standard makefiles work then please read the
# 'Before.mk' file as it is the real entry point and the best place to learn.

# Perform sanity checks.
#
# Make sure that the progammer has already included 'Before.mk'. 
ifneq ($(SvBeforeHasBeenIncluded),yes)
$(error You did not include "$$(SvStandardMakefiles)/Versions/2/Before.mk" before including "$$(SvStandardMakefiles)/Versions/2/After.mk")
endif

# Include the footer part of the template
include $(SvStandardMakefilesInstance)Templates/$(SvMakefileTemplate)/After.mk

# Include all elements of the requested template
include $(sort $(wildcard $(SvStandardMakefilesInstance)Templates/$(SvMakefileTemplate)/Bits/*.mk))

include $(SvStandardMakefilesInstance)SharedElements/evaluate-common-variables.mk
include $(SvStandardMakefilesInstance)SharedElements/shell_cmds-awareness.mk

.PHONY: debug-sv
# Define a simple debug target that dumps all the variables defined in this
# session along with their origin. The debug target should not be moved to the
# 'Before.mk' file due to the risk of becoming the default target.
debug-sv:
	$(foreach var,$(filter Sv%,$(filter-out .VARIABLES,$(.VARIABLES))),$(info varaiable: '$(var)', value: '$(value $(var))', actual value: '$($(var))', origin: '$(origin $(var))'))

debug-make:
	@echo Make version: $(wordlist 1,3,$(shell $(MAKE) --version))
	@echo Available features: $(.FEATURES)
	@echo Include directories: $(.INCLUDE_DIRS)
SvHelp[debug]=Lists all variables in the Sv namespace, their definition, value and origin

ifeq ($(SvStandardMakefilesMulticoreCompile),yes)
  ifeq ($(MAKECMDGOALS),compile)
    MAKEFLAGS += -j$(shell grep -c processor /proc/cpuinfo)
  endif
endif
