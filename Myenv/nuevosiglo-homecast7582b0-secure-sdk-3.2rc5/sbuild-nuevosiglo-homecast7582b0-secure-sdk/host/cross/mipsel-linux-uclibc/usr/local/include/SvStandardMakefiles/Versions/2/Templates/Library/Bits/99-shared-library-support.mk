.PHONY: compile-shared-library

SvHelp[compile-shared-library]=Compiles all source files and links them as a shared library

compile-shared-library: $(SvProductsDir)$(SvSharedLibraryFullFile)

ifeq ($(SvCompileSharedLibrary),yes)
SvSharedLibraryObjects=$(call SvTurnSourcesToObjects,$(SvLibrarySourceFiles))

ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
SvSharedLibraryCommand=$(LINK.o) -dynamiclib -Wl,-undefined,suppress -Wl,-flat_namespace \
		-install_name $(SvLibPrefix)/$(SvSharedLibrarySONameFile) \
		-compatibility_version $(SvMajorVersion).$(SvMinorVersion) \
		-current_version $(SvMajorVersion).$(SvMinorVersion).$(SvRevision) \
		$(OUTPUT_OPTION) $(patsubst $(SvTemporaryDir)%,%,$^) $(LDLIBS)
else ifeq ($(SvTargetPlatformOperatingSystemName),Linux)
 ifneq ($(filter 2.14 2.15 2.16 2.16.1 2.17,$(V_BINUTILS)),)
  SvSharedLibraryCommand=$(LINK.o) -shared -Wl,-soname=$(SvSharedLibrarySONameFile) $(OUTPUT_OPTION) $(patsubst $(SvTemporaryDir)%,%,$^) $(LDLIBS)
 else
  SvSharedLibraryCommand=$(LINK.o) -shared -Wl,-soname=$(SvSharedLibrarySONameFile) -Wl,--build-id $(OUTPUT_OPTION) $(patsubst $(SvTemporaryDir)%,%,$^) $(LDLIBS)
 endif
else ifeq ($(SvTargetPlatformOperatingSystemName),SunOS)
SvSharedLibraryCommand=$(LINK.o) -shared -Wl,-soname=$(SvSharedLibrarySONameFile) $(OUTPUT_OPTION) $(patsubst $(SvTemporaryDir)%,%,$^) $(LDLIBS)
else ifeq ($(SvTargetPlatformOperatingSystemName),FreeBSD)
SvSharedLibraryCommand=$(LINK.o) -shared -Wl,-soname=$(SvSharedLibrarySONameFile) $(OUTPUT_OPTION) $(patsubst $(SvTemporaryDir)%,%,$^) $(LDLIBS)
else
# TODO: Add Win32 support (mingw)
$(error Standard Makefiles does not know how to build shared libraries for your system)
endif

$(SvProductsDir)$(SvSharedLibraryFullFile): $(SvSharedLibraryObjects)
	@set -e
	@mkdir -p `dirname $@`
	@$(call SvShellCommand,cd $(SvTemporaryDir) && $(SvSharedLibraryCommand),$(call SvTextGeneratorCreatingSharedLibraryFrom,$(SvSharedLibraryFile),$(patsubst $(SvTemporaryDir)%,%,$^)))
endif
