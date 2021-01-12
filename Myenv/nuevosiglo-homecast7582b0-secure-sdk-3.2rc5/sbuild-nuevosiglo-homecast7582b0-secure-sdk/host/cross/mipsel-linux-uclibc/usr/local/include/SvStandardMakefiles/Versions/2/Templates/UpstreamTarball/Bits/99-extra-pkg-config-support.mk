compile-pkg-config: $(SvTemporaryDir)$(SvPkgConfigFile)

define SvPkgConfigTemplate
Name: $(SvName)
Description: $(SvDescription)
URL: $(SvURL)
Version: $(SvVersion)
Requires: $(SvRequires)
Conflicts: $(SvConflicts)
Libs: $(SvPublicLinkerOptions)
Libs.private: $(SvPrivateLibraries)
Cflags: $(SvPublicCompilerOptions)
endef
export SvPkgConfigTemplate

$(SvTemporaryDir)$(SvPkgConfigFile): Makefile
	@set -e
	mkdir -p `dirname $@`
	echo "$$SvPkgConfigTemplate" >$@
