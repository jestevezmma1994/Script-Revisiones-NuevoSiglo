# A set of rules implementing the 'clean' target
.PHONY: clean clean-build-dir
clean: $(SvCleanTargets)
SvHelp[clean]=Removes the build directory
clean-build-dir:
	@$(call SvShellCommand,$(call SvRemoveTreeCommand,$(SvBuildDir)),$(call SvTextGeneratorRemovingRecursively,$(SvBuildDir)))
