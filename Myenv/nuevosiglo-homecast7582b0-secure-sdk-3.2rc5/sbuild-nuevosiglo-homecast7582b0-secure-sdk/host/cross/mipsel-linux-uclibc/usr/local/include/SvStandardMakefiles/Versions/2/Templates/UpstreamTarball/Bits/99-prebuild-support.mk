# The prebuild rule
.PHONY: prebuild  prebuild-configure  configure-help

#  -----  prebuild -----

prebuild: $(SvPrebuildTargets)

#  -----  configure -----

prebuild-configure:
ifeq ($(SvConfigureEnabled),yes)
	@$(call SvPlainShellCommand,cd $(SvBuildDir)$(SvExtractedSourceDir)/$(SvRelevantSubDirectory) && ($(SvSRMLocalRunIfNeeded) $(call SvRunConfigureScriptGenerator,$(SvConfigureScript),$(SvConfigureOptions))),$(call SvTextGeneratorConfiguringPackageWithOptions,$(SvName),$(SvConfigureOptions)))
endif


configure-help: $(SvBuildDir)$(SvExtractedSourceDir)
	@$(call SvPlainShellCommand,cd $</$(SvRelevantSubDirectory) && ($(SvSRMLocalRunIfNeeded) $(call SvRunConfigureScriptGenerator,$(SvConfigureScript),--help)),$(call SvTextGeneratorConfiguringPackageWithOptions,$(SvName),--help))
