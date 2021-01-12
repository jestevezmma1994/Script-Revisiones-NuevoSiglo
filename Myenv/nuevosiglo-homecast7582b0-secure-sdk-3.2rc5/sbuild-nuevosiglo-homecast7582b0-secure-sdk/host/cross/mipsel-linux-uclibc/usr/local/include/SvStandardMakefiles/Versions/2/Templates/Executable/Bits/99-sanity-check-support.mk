.PHONY: check-source-sanity check-class-declarations

check-source-sanity:
	@$(call SvPlainShellCommand,CFLAGS="$(CPPFLAGS) $(CFLAGS)" check_source_sanity.sh -c $(SvName) -t $(SvTemporaryDir) $(SvSanityCheckSourceFiles),$(SvTextGeneratorCheckingSourceFilesSanity))

check-class-declarations:
	@$(call SvPlainShellCommand,CFLAGS="$(CPPFLAGS) $(CFLAGS)" check_class_declarations.py -c $(SvName) $(SvSanityCheckSourceFiles),$(SvTextGeneratorCheckingClassDeclarations))
