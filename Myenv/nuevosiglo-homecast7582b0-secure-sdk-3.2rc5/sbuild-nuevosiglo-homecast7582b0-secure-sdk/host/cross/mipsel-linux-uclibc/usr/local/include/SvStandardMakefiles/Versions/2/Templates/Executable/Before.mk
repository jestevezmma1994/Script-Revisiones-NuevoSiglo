# Primary configuration options:
#
# You can define them in your makefile if you whish to alter the defaults.
# no options yet...

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
SvInstallTargets        ?= install-executable install-catalogs
SvCompileTargets        ?= compile-executable compile-catalogs
SvCleanTargets          ?= clean-build-dir

# Conditionally enable sanity checks.
ifdef SvEnableSanityChecks
  SvInstallTargets      += check-source-sanity check-class-declarations
endif

# Directory layout convention used.
#
# You are discouraged from changing this but you may if you want to. 
# Possible values are 'sentivision', 'unix' and 'custom'.
SvDirectoryLayoutConvention ?= sentivision
