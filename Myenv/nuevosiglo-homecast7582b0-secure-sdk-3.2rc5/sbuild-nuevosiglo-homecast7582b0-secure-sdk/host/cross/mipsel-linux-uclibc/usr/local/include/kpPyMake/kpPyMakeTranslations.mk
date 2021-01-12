
kpCompiledMessageCatalogFiles=$(foreach f,$(basename $(kpMessageCatalogFiles)),$(kpTmpDir)/$(f).mo)

compile-catalogs: $(kpCompiledMessageCatalogFiles)

$(kpTmpDir)/%.mo : %.po
	install -d $(dir $@)
	msgconv -t UTF-8 $^ | msgfmt -o $@ -f - 

install-translation-catalogs : $(kpCompiledMessageCatalogFiles)
	@set -e; \
	for f in $^; do\
		lang=$$(basename $$f .mo) ;\
		if [ ! -d "$(TARGET_ROOT)$(kpDataPrefix)/usr/local/share/locale/$$lang/LC_MESSAGES" ]; then \
			install -d "$(TARGET_ROOT)$(kpDataPrefix)/usr/local/share/locale/$$lang/LC_MESSAGES" ;\
		fi; \
		install -m 0644 $$f $(TARGET_ROOT)/$(kpDataPrefix)/usr/local/share/locale/$$lang/LC_MESSAGES/$(kpTextDomain).mo; \
    done
