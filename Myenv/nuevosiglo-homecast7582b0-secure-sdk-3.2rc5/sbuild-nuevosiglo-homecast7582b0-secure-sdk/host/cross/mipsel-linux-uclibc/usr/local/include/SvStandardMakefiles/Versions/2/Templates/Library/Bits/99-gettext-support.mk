.PHONY: compile-catalogs install-catalogs

SvCompiledMessageCatalogFiles=$(foreach f,$(basename $(SvMessageCatalogFiles)),$(SvProductsDir)$(f).mo)

compile-catalogs: $(SvCompiledMessageCatalogFiles)

$(SvProductsDir)%.mo: %.po
	@set -e; \
	mkdir -p $(shell dirname $@) $(shell dirname $(addprefix $(SvTemporaryDir),$(@:$(SvProductsDir)%=%))); \
	$(call SvShellCommand,msgconv -t UTF-8 $^ > $(addprefix $(SvTemporaryDir),$(@:$(SvProductsDir)%.mo=%.po).tmp),$(call SvTextGeneratorFormatingMessageCatalog,$(@:$(SvProductsDir)%.mo=%.po))) \
	$(call SvShellCommand,msgfmt -o $@ -f $(addprefix $(SvTemporaryDir),$(@:$(SvProductsDir)%.mo=%.po).tmp),$(call SvTextGeneratorCompilingMessageCatalog,$(@:$(SvProductsDir)%=%)))

install-catalogs: $(SvCompiledMessageCatalogFiles)
	@set -e; \
	for msgcat in $(^:$(SvProductsDir)%=%); do \
		lang=$$(basename $$msgcat .mo); \
		if [ ! -d "$(TARGET_ROOT)$(SvDataPrefix)/locale/$$lang/LC_MESSAGES" ]; then \
			$(call SvInstallDirectory,$(SvDataPrefix)/locale/$$lang/LC_MESSAGES,$(TARGET_ROOT)) \
		fi; \
		$(call SvInstallFile,$$msgcat,$(SvProductsDir),$(SvDataPrefix)/locale/$$lang/LC_MESSAGES/$(SvTextDomain).mo,$(TARGET_ROOT),SvTextGeneratorInstallingMessageCatalogFile) \
	done
