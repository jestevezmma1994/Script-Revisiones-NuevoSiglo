ifeq ($(SvCompileStaticLibrary),yes)

.PHONY: compile-static-library
SvHelp[compile-static-library]=Compiles all source files and puts resulting object files in an ar archive
compile-static-library: $(SvProductsDir)$(SvStaticLibraryFile)

ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
  SvStaticLibraryCommand=libtool -static -o $@ -s $(patsubst $(SvTemporaryDir)%,%,$^)
else ifneq ($(filter Linux SunOS FreeBSD,$(SvTargetPlatformOperatingSystemName)),)
  # Static library build command for Linux and Solaris, will probably work on BSD too
  SvStaticLibraryCommand=$(AR) $(ARFLAGS) $@ $(patsubst $(SvTemporaryDir)%,%,$^)
else ifeq ($(SvTargetPlatformOperatingSystemName),MINGW32_NT-5.1)
  SvStaticLibraryCommand=$(AR) $(ARFLAGS) $@ $(patsubst $(SvTemporaryDir)%,%,$^)
else
  # TODO: Add Win32 support (mingw)
  # TODO: Add BSD support (should be idential to linux, right?)
  $(error Standard Makefiles does not know how to build static libraries for your system)
endif

SvStaticLibraryObjects = $(call SvTurnSourcesToObjects,$(SvLibrarySourceFiles))

$(SvProductsDir)$(SvStaticLibraryFile): $(SvStaticLibraryObjects)
	@set -e
	@mkdir -p `dirname $@`
	@$(call SvShellCommand,cd $(SvTemporaryDir) && $(SvStaticLibraryCommand),$(call SvTextGeneratorCreatingStaticLibraryFrom,$(SvStaticLibraryFile),$(patsubst $(SvTemporaryDir)%,%,$^)))

endif
