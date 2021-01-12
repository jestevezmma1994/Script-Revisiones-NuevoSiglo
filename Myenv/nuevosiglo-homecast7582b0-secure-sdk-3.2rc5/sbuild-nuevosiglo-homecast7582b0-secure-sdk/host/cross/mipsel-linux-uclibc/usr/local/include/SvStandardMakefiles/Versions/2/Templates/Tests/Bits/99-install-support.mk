# Install support
.PHONY: install install-executables
install: $(SvInstallTargets)
install-executables: $(addprefix $(SvProductsDir),$(SvExecutableFiles))
	@set -e
	@$(call SvInstallDirectory,$(SvBinPrefix),$(DESTDIR))
	@$(foreach executable,$(SvExecutableFiles),$(call SvInstallExecutableFile,$(executable),$(SvProductsDir),$(SvBinPrefix),$(DESTDIR),SvTextGeneratorInstallingExecutableFile))
