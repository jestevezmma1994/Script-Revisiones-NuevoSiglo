# Install support
.PHONY: install install-executable

install: $(SvInstallTargets)

install-executable: $(SvProductsDir)$(SvExecutableFile)
	@set -e
	@$(call SvInstallDirectory,$(SvBinPrefix),$(DESTDIR))
	@$(call SvInstallExecutableFile,$(SvExecutableFile),$(SvProductsDir),$(SvBinPrefix),$(DESTDIR),SvTextGeneratorInstallingExecutableFile)
