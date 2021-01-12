# Learn how to clean perform cleanup
include $(SvStandardMakefilesInstance)/SharedElements/clean-support.mk

clean-internal:
	@$(call SvPlainShellCommand,$(MAKE) DESTDIR=$(SvPreinstallDir) -C $(SvBuildDir)$(SvExtractedSourceDir)/$(SvRelevantSubDirectory) clean,$(call SvTextGeneratorCleaningPackage,$(SvName)))
