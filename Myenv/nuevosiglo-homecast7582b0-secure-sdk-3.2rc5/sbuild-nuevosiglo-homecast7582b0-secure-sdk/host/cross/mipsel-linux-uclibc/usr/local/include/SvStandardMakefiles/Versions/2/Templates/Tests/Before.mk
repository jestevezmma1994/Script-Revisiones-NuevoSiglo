# Primary configuration options:
#
# You can define them in your makefile if you whish to alter the defaults.
# no options yet...

# Mac-style universal libraries are enabled by default when building for darwin. 
ifeq ($(SvTargetPlatform),Darwin)
SvUniversalBinaries		?= yes
else
SvUniversalBinaries		= no
endif

# TODO: add support for compile-time selected features
SvEnabledFeatures       ?=

# Additional configuration options:
#
# You can define them in your makefile if you need to do so. The defaults
# spelled out explicitly make it clear and have an added benefit of easy
# debugging when running:
# make --warn-undefined-variables'
SvRequires              ?=

# Targets with defaults.
#
# You may append or even entirely replace them in your makefile if you know
# what you are doing. Doing so will alter the work that each particular
# toplevel target actually does. It may be useful for some hack-and-test
# approach for extending the makefile templates and for special-purpose tasks.
SvInstallTargets        ?= install-executables
SvCompileTargets        ?= compile-executables
SvCleanTargets          ?= clean-build-dir
