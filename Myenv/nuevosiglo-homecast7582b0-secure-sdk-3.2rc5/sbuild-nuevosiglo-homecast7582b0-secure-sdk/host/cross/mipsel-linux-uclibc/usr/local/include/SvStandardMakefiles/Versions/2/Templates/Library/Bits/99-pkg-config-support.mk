.PHONY: compile-pkg-config
compile-pkg-config: $(SvProductsDir)$(SvPkgConfigFile) $(SvProductsDir)$(SvPkgConfigUninstalledFile)

# This code exports the templates for normal and uninstalled versions of
# pkg-config files.
#
# To anone thinking exporting an internal variable is ugly: This is the
# *only* way of getting a multi-line output without having to manually echo
# >> each line. Make will make everything to prevent you from getting
# newlines correctly and bash will show you how quoting can ruin your day.
#
# So live with it and enjoy the fact that it works!

ifeq ($(SvCompileSharedLibrary),yes)
  SvPublicLinkerOptions := $(SvPublicLinkerOptions) $(SvRequiredLibraries)
else
  SvPublicLinkerOptions :=
endif
SvPrivateLinkerOptions := $(SvPrivateLinkerOptions) $(SvRequiredLibraries) $(SvPrivateLibraries)

ifdef CROSS_ROOT

# This version uses CROSS_ROOT to get paths right
define SvPkgConfigTemplate
SKIP_PATCH=1
prefix=$(SvPrefix)
libdir=$(SvLibPrefix)/
includedir=$(SvIncludePrefix)/
Name: $(SvName)
Description: $(SvDescription)
Version: $(SvMajorVersion).$(SvMinorVersion).$(SvRevision)
Requires: $(SvRequires)
Requires.private: $(SvPrivateRequires)
Conflicts: $(SvConflicts)
Libs: -L$$$${CROSS_ROOT}/$${libdir} $(SvPublicLinkerOptions)
Libs.private: -L$$$${CROSS_ROOT}/$${libdir} $(SvPrivateLinkerOptions)
Cflags: -I$$$${CROSS_ROOT}/$${includedir} $(SvPublicCompilerOptions)
endef

define SvPkgConfigUninstalledTemplate
libdir=$(SvProductsDir)
includedir=$(SvHeadersDir)
Name: $(SvName)-uninstalled
Description: $(SvDescription) (uninstalled)
Version: $(SvMajorVersion).$(SvMinorVersion).$(SvRevision)
Requires: $(SvRequires)
Requires.private: $(SvPrivateRequires)
Conflicts: $(SvConflicts)
Libs: -L$${libdir} $(SvPublicLinkerOptions)
Libs.private: -L$${libdir} $(SvPrivateLinkerOptions)
Cflags: -I$${includedir} $(SvPublicCompilerOptions)
endef

else

# This version is for building without silo/srm

define SvPkgConfigTemplate
prefix=$${pc_top_builddir}$(SvPrefix)
libdir=$${pc_top_builddir}$(SvLibPrefix)/
includedir=$${pc_top_builddir}$(SvIncludePrefix)/

Name: $(SvName)
Description: $(SvDescription)
Version: $(SvMajorVersion).$(SvMinorVersion).$(SvRevision)
Requires: $(SvRequires)
Requires.private: $(SvPrivateRequires)
Conflicts: $(SvConflicts)
Libs: -L$${libdir} $(SvPublicLinkerOptions)
Libs.private: -L$${libdir} $(SvPrivateLinkerOptions)
Cflags: -I$${includedir} $(SvPublicCompilerOptions)
endef

define SvPkgConfigUninstalledTemplate
libdir=$(SvProductsDir)
includedir=$(SvHeadersDir)

Name: $(SvName)-uninstalled
Description: $(SvDescription) (uninstalled)
Version: $(SvMajorVersion).$(SvMinorVersion).$(SvRevision)
Requires: $(SvRequires)
Requires.private: $(SvPrivateRequires)
Conflicts: $(SvConflicts)
Libs: -L$${libdir} $(SvPublicLinkerOptions)
Libs.private: -L$${libdir} $(SvPrivateLinkerOptions)
Cflags: -I$${includedir} $(SvPublicCompilerOptions)
endef

# ifdef CROSS_ROOT
endif

export SvPkgConfigTemplate
export SvPkgConfigUninstalledTemplate

$(SvProductsDir)$(SvPkgConfigFile): $(firstword $(MAKEFILE_LIST))
	@set -e
	@mkdir -p `dirname $@`
	@$(call SvShellCommand,echo "$$SvPkgConfigTemplate">$@,$(call SvTextGeneratorCreatingPkgConfig,$(SvPkgConfigFile)))

$(SvProductsDir)$(SvPkgConfigUninstalledFile): $(firstword $(MAKEFILE_LIST))
	@set -e
	@mkdir -p `dirname $@`
	@$(call SvShellCommand,echo "$$SvPkgConfigUninstalledTemplate">$@,$(call SvTextGeneratorCreatingPkgConfig,$(SvPkgConfigUninstalledFile)))
