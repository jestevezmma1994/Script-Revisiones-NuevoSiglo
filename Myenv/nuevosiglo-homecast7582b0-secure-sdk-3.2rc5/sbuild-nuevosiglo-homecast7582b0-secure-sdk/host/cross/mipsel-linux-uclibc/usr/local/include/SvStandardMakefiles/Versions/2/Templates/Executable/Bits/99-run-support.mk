.PHONY: run
SvRunCommand=$^ $(ARGS)
SvRunGdbCommand=gdb --args $^ $(ARGS)
run: $(SvProductsDir)$(SvExecutableFile)
	@$(call SvPlainShellCommand,$(SvRunCommand),$(call SvTextGeneratorRunningExecutable,$(SvExecutableFile)))
run-gdb: $(SvProductsDir)$(SvExecutableFile)
	@$(call SvPlainShellCommand,$(SvRunGdbCommand),$(call SvTextGeneratorRunningExecutableViaGdb,$(SvExecutableFile)))
