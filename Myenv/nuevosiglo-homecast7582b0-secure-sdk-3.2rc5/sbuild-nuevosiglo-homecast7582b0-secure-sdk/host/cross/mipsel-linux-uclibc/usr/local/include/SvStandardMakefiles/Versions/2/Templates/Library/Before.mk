# Primary configuration options:
#
# You can define them in your makefile if you whish to alter the defaults.
SvCompileUnitTests      ?= no
SvCompileStaticLibrary  ?= yes
SvCompileSharedLibrary  ?= yes
SvCompileDocumentation  ?= no

# Additional configuration options:
#
# You can define them in your makefile if you need to do so. The defaults
# spelled out explicitly make it clear and have an added benefit of easy
# debugging when running:
# make --warn-undefined-variables'
SvRequires              ?=
SvConflicts             ?=
SvPublicLinkerOptions   ?= -l$(SvName)
SvPublicCompilerOptions ?=
SvPrivateLinkerOptions  ?= -l$(SvName)
SvPrivateLibraries      ?=


# Targets with defaults.
#
# You may append or even entirely replace them in your makefile if you know
# what you are doing. Doing so will alter the work that each particular
# toplevel target actually does. It may be useful for some hack-and-test
# approach for extending the makefile templates and for special-purpose tasks.
SvInstallTargets        ?= install-doc-sources install-pkg-config install-headers install-catalogs
SvCompileTargets        ?= compile-pkg-config compile-catalogs
SvCleanTargets          ?= clean-build-dir

# Directory layout convention used.
#
# You are discouraged from changing this but you may if you want to. 
# Possible values are 'mac', 'unix' and 'custom'.
SvDirectoryLayoutConvention ?= mac

# Conditionally enable sanity checks.
ifdef SvEnableSanityChecks
  SvInstallTargets 	+= check-header-sanity check-source-sanity check-class-declarations check-library-sanity check-makefile-sanity
endif
