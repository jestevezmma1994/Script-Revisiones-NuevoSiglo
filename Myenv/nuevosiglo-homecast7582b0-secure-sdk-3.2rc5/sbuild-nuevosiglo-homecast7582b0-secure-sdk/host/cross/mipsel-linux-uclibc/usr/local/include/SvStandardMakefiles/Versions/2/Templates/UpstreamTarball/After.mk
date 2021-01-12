# Sanity checks on variables defined by the programmer

# Required variables
SvName                  ?= $(error $(call SvUndefinedVariable,SvName))
SvDescription           ?= $(error $(call SvUndefinedVariable,SvDescription))
SvVersion               ?= $(error $(call SvUndefinedVariable,SvVersion))
SvBaseURL               ?= $(error $(call SvUndefinedVariable,SvBaseURL))

# Check for proper flavour
ifeq ($(filter $(SvFlavours),$(SvFlavour)),)
$(error Unsupported flavour selected. Choose from: $(SvFlavours))
endif

# Support for generated pkg-config for packages that don't provide one
ifeq ($(SvGeneratePkgConfigFile),yes)
SvInstallTargets        += install-pkg-config
endif
