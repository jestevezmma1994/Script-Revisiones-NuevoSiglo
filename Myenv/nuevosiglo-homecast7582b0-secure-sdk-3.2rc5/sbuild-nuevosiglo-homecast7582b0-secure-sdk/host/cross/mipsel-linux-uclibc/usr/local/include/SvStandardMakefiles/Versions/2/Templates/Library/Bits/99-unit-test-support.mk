ifeq ($(SvCompileUnitTests),yes)
.PHONY: compile-unit-tests check
SvHelp[check]=Builds and runs unit tests
SvHelp[compile-unit-tests]=Compiles unit tests
compile-unit-tests: $(SvProductsDir)$(SvUnitTestFile)
# List of objects that that are used to build the unit test executable.
SvUnitTestObjects=$(call SvTurnSourcesToObjects,$(SvUnitTestSourceFiles))

ifeq ($(SvCompileSharedLibrary),yes)
SvAnyLibraryFile=$(SvSharedLibraryFullFile)
endif
ifeq ($(SvCompileStaticLibrary),yes)
SvAnyLibraryFile=$(SvStaticLibraryFile)
endif
SvAnyLibraryFile?=$(error "Cannot build unit-tests without selecting either static or shared library as well")

ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
ifneq ($(filter-out $(SvName)-uninstalled,$(SvUnitTestRequires)),)
ifneq ($(shell $(SvPkgConfig) --short-errors --print-errors $(filter-out $(SvName)-uninstalled,$(SvUnitTestRequires)) && echo yes),yes)
$(error "Pkg-config detected dependency problem preventing compilation of unit tests")
endif
endif
endif

# This tricky thing is required for unit tests to see the internal, not
# installed, headers of a library. The unit test must assume that the file
# is installed and use #include <foo.h> to access it
ifneq ($(SvSourcesDir),)
$(SvUnitTestObjects): CPPFLAGS += -I$(SvSourcesDir)
endif

# We depend on having this file on time!
$(SvProductsDir)$(SvUnitTestFile): $(SvProductsDir)$(SvPkgConfigUninstalledFile)
$(SvProductsDir)$(SvUnitTestFile): LDLIBS.unit-test += $(shell PKG_CONFIG_PATH=$(SvProductsDir):$(PKG_CONFIG_PATH) $(SvPkgConfig) --libs-only-l --static $(SvUnitTestRequires))
$(SvProductsDir)$(SvUnitTestFile): LDFLAGS.unit-test += $(shell PKG_CONFIG_PATH=$(SvProductsDir):$(PKG_CONFIG_PATH) $(SvPkgConfig) --libs-only-L --static $(SvUnitTestRequires))
SvUnitTestCommand=$(CC) $(LDFLAGS.unit-test) $(OUTPUT_OPTION) $(filter %.o,$^) $(LDLIBS.unit-test)

# Build unit test binary
$(SvProductsDir)$(SvUnitTestFile): $(SvUnitTestObjects) $(SvProductsDir)$(SvPkgConfigUninstalledFile) $(SvProductsDir)$(SvAnyLibraryFile)
	@set -e
	@mkdir -p `dirname $@`
	@$(call SvShellCommand,$(SvUnitTestCommand),$(call SvTextGeneratorLinking,$(SvUnitTestFile)))

ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
# This rule runs unit tests on Darwin with setting the DYLD_LIBRARY_PATH to include
# the products directory where an uninstalled version of dynamic library is located
SvCheckCommand=DYLD_LIBRARY_PATH=$(SvProductsDir):$(DYLD_LIBRARY_PATH) $^
SvCheckGdbCommand=DYLD_LIBRARY_PATH=$(SvProductsDir):$(DYLD_LIBRARY_PATH) gdb $^
else ifneq ($(filter Linux SunOS,$(SvTargetPlatformOperatingSystemName)),)
# This rule runs unit tests on Linux and Solaris with setting the LD_LIBRARY_PATH to
# include the products directory where an uninstalled version of shared library is located
SvCheckCommand=LD_LIBRARY_PATH=$(SvProductsDir):$(LD_LIBRARY_PATH) $^
SvCheckGdbCommand=LD_LIBRARY_PATH=$(SvProductsDir):$(LD_LIBRARY_PATH) gdb $^
else
# TODO: Add Win32 support
# TODO: Add BSD support
$(error Standard Makefiles does not know how to run unit tests for your system)
endif

check: $(SvProductsDir)$(SvUnitTestFile)
	@$(call SvShellCommand,$(SvCheckCommand),$(call SvTextGeneratorRunningExecutable,$(SvUnitTestFile)))
check-gdb: $(SvProductsDir)$(SvUnitTestFile)
	@$(call SvPlainShellCommand,$(SvCheckGdbCommand),$(call SvTextGeneratorRunningExecutableViaGdb,$(SvUnitTestFile)))
endif
