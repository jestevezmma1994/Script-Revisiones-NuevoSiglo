# Install support
.PHONY: install install-pkg-config install-headers install-static-library
install: $(SvInstallTargets)

SvHelp[install]=Installs library archives, header files and pkg-config files to \$DESTDIR
SvHelp[install-shared-library]=Install shared library files
SvHelp[install-static-library]=Install static library files
SvHelp[install-headers]=Install all header files
SvHelp[install-pkg-config]=Install pkg-config file

# Small CROSS_ROOT hack
ifdef CROSS_ROOT
ORIG_DESTDIR:=$(DESTDIR)
override DESTDIR:=$(CROSS_ROOT)
endif

install-pkg-config: $(SvProductsDir)$(SvPkgConfigFile)
	@set -e
	@$(call SvInstallDirectory,$(SvLibPrefix)/pkgconfig,$(DESTDIR))
	@$(call SvInstallFile,$(SvPkgConfigFile),$(SvProductsDir),$(SvLibPrefix)/pkgconfig,$(DESTDIR),SvTextGeneratorInstallingPkgConfigFile)

install-headers: $(SvLibraryHeaderFiles) 
	@set -e
	@for header in $(^:$(SvHeadersDir)%=%); do \
		extra_dir=$$(dirname $$header); \
		if [ "$$extra_dir" = "." ]; then extra_dir=; fi; \
		if [ ! -d "$(DESTDIR)$(SvIncludePrefix)/$$extra_dir" ]; then \
			$(call SvInstallDirectory,$(SvIncludePrefix)/$$extra_dir,$(DESTDIR))\
		fi; \
		$(call SvInstallFile,$$header,$(SvHeadersDir),$(SvIncludePrefix)/$$extra_dir,$(DESTDIR),SvTextGeneratorInstallingHeaderFile) \
	done

install-static-library: $(SvProductsDir)$(SvStaticLibraryFile)
	@set -e
	@$(call SvInstallDirectory,$(SvLibPrefix),$(DESTDIR))
	@$(call SvInstallFile,$(SvStaticLibraryFile),$(SvProductsDir),$(SvLibPrefix),$(DESTDIR),SvTextGeneratorInstallingStaticLibraryFile)

install-shared-library: $(SvProductsDir)$(SvSharedLibraryFullFile)
	@set -e
	@$(call SvInstallDirectory,$(SvLibPrefix),$(DESTDIR))
	@$(call SvInstallExecutableFile,$(SvSharedLibraryFullFile),$(SvProductsDir),$(SvLibPrefix),$(DESTDIR),SvTextGeneratorInstallingSharedLibraryFile)
ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
	@(cd $(DESTDIR)$(SvLibPrefix)/ && ln -fs $(SvSharedLibraryFullFile) $(SvSharedLibraryFile))
	@(cd $(DESTDIR)$(SvLibPrefix)/ && ln -fs $(SvSharedLibraryFullFile) $(SvSharedLibraryMajorFile))
	@(cd $(DESTDIR)$(SvLibPrefix)/ && ln -fs $(SvSharedLibraryFullFile) $(SvSharedLibrarySONameFile))
ifdef CROSS_ROOT
	@$(call SvInstallDirectory,$(SvLibPrefix),$(ORIG_DESTDIR))
	@$(call SvInstallExecutableFile,$(SvSharedLibraryFullFile),$(SvProductsDir),$(SvLibPrefix),$(ORIG_DESTDIR),SvTextGeneratorInstallingSharedLibraryFile)
	@(cd $(ORIG_DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibraryFile)")
	@(cd $(ORIG_DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibraryMajorFile)")
	@(cd $(ORIG_DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibrarySONameFile)")
endif
else ifneq ($(filter Linux SunOS,$(SvTargetPlatformOperatingSystemName)),)
	@(cd $(DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibraryMajorFile)")
	@(cd $(DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryMajorFile)" "$(SvSharedLibraryFile)")
	@(cd $(DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibrarySONameFile)")
ifdef CROSS_ROOT
	@$(call SvInstallDirectory,$(SvLibPrefix),$(ORIG_DESTDIR))
	@$(call SvInstallExecutableFile,$(SvSharedLibraryFullFile),$(SvProductsDir),$(SvLibPrefix),$(ORIG_DESTDIR),SvTextGeneratorInstallingSharedLibraryFile)
	@(cd $(ORIG_DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibraryMajorFile)")
	@(cd $(ORIG_DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibrarySONameFile)")
endif
else ifneq ($(filter FreeBSD,$(SvTargetPlatformOperatingSystemName)),)
	@(cd $(DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibraryFile)")
ifdef CROSS_ROOT
	@$(call SvInstallDirectory,$(SvLibPrefix),$(ORIG_DESTDIR))
	@$(call SvInstallExecutableFile,$(SvSharedLibraryFullFile),$(SvProductsDir),$(SvLibPrefix),$(ORIG_DESTDIR),SvTextGeneratorInstallingSharedLibraryFile)
	@(cd $(ORIG_DESTDIR)$(SvLibPrefix)/ && ln -fs "$(SvSharedLibraryFullFile)" "$(SvSharedLibraryFile)")
endif
else
	@echo "Standard Makefiles does not know how to install shared libraries on your system" >&1 && false
endif
