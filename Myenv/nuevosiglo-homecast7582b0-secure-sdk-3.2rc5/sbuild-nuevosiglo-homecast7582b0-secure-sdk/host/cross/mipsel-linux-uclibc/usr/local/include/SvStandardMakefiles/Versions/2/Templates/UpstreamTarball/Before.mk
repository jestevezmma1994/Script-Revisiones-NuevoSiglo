# Decision-making variables
SvPrefix                := $(subst /usr/local,/usr/,$(SvPrefix))
SvConfigureOptions      ?= --prefix=$(SvPrefix) --libdir=$(SvPrefix)/lib
SvConfigureEnabled      ?= yes
SvPatches               ?= $(sort $(wildcard Patches/*.patch) $(wildcard Patches/$(SvFlavour)/*.patch))
SvPkgConfigOverride     ?= no
SvRequires              ?=
SvConflicts             ?=
SvPublicLinkerOptions   ?=
SvPublicCompilerOptions ?=
SvPrivateLibraries      ?=
SvDoNotCrossCompile     ?= no
SvFilesToPatch          ?= $(shell find $(SvBuildDir) -name "*.pc" -o -name "lib*.la*")
SvConfigureScript       ?= configure
SvRelevantSubDirectory  ?=
SvRunConfigureScriptGenerator ?= sh $1 $2
SvFlavours              ?= default
SvFlavour               ?= default

# Tarball, URL and extracted directory name
SvTarballFileBase       ?= $(SvName)-$(SvVersion)
SvTarballFileExtraJunk  ?=
SvExtractedSourceDir    ?= $(SvTarballFileBase)
SvTarballFileExt        ?= .tar.gz
SvTarballFileNoExt      ?= $(SvTarballFileBase)$(SvTarballFileExtraJunk)
SvTarballFile           ?= $(SvTarballFileNoExt)$(SvTarballFileExt)
SvURL                   ?= $(SvBaseURL)$(SvTarballFile)

# Some extra variables
SvPackageTarballExt     ?= .tar.bz2
SvPackageName           ?= $(SvName)
SvPackageVersion        ?= $(SvVersion)

# Targets with defaults.
#
# You may append or even entirely replace them in your makefile if you know
# what you are doing. Doing so will alter the work that each particular
# toplevel target actually does. It may be useful for some hack-and-test
# approach for extending the makefile templates and for special-purpose tasks.
SvCleanTargets          ?= clean-build-dir
SvPrebuildTargets       ?= prebuild-fetch  prebuild-extract  prebuild-patch  prebuild-configure
SvCompileTargets        ?= compile-all
SvPreinstallTargets     ?= install-pre-all
SvInstallTargets        ?= install-pre  install-all
