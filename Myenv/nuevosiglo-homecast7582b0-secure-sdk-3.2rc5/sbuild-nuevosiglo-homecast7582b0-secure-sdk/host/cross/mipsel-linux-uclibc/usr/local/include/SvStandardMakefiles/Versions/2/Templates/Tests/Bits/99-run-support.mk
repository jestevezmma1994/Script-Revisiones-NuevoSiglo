.PHONY: run
run-%: $(SvProductsDir)%
	@$(call SvPlainShellCommand,$* $(ARGS),$(call SvTextGeneratorRunningExecutable,$*))
run-gdb-%: $(SvProductsDir)%
	@$(call SvPlainShellCommand,gdb --args $* $(ARGS),$(call SvTextGeneratorRunningExecutableViaGdb,$*))
