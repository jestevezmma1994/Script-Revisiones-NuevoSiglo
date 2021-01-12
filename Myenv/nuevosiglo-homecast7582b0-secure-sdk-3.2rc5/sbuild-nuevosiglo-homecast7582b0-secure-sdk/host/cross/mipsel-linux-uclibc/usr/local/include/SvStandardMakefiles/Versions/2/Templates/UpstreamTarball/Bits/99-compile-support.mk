# The compile rule, yeah easy
.PHONY: compile  compile-all

#  -----  compile -----

compile: $(SvCompileTargets)

compile-all:
	@$(call SvPlainShellCommand,cd $(SvBuildDir)$(SvExtractedSourceDir)/$(SvRelevantSubDirectory) && ($(SvSRMLocalRunIfNeeded) $(MAKE)),$(call SvTextGeneratorBuildingPackage,$(SvName)))


patch-paths:
# empty (for now?) -deffer 2009.05.22
