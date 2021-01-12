.PHONY: check-header-sanity check-library-sanity check-source-sanity check-class-declarations check-makefile-sanity

SvMakefileName=$(firstword $(MAKEFILE_LIST))

check-header-sanity:
	@$(call SvPlainShellCommand,CFLAGS="$(CPPFLAGS) $(CFLAGS) $(shell $(SvPkgConfig) --cflags $(SvName))" check_header_sanity.sh -c $(SvName) -t $(SvTemporaryDir) $(SvSanityCheckHeaderFiles),$(SvTextGeneratorCheckingHeaderFilesSanity))

check-library-sanity: install-shared-library
	@$(call SvPlainShellCommand,check_lib_sanity.sh $(DESTDIR)/$(SvLibPrefix)/$(SvSharedLibraryFullFile) $(SvSanityCheckHeaderFiles),$(SvTextGeneratorCheckingSharedLibrarySanity))

check-source-sanity:
	@$(call SvPlainShellCommand,CFLAGS="$(CPPFLAGS) $(CFLAGS) $(shell $(SvPkgConfig) --cflags $(SvName))" check_source_sanity.sh -c $(SvName) -t $(SvTemporaryDir) $(SvSanityCheckSourceFiles),$(SvTextGeneratorCheckingSourceFilesSanity))

check-class-declarations:
	@$(call SvPlainShellCommand,CFLAGS="$(CPPFLAGS) $(CFLAGS) $(shell $(SvPkgConfig) --cflags $(SvName))" check_class_declarations.py -c $(SvName) $(SvSanityCheckSourceFiles),$(SvTextGeneratorCheckingClassDeclarations))

check-makefile-sanity:
	@$(call SvPlainShellCommand, check_makefile_sanity.sh $(SvMakefileName) $(SvName) ,$(SvTextGeneratorCheckingMakefileSanity))
