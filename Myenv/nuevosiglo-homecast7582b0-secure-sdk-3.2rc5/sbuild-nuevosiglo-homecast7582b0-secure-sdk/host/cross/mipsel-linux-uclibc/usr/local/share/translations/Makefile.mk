PREFIX:=$(CROSS_ROOT)/usr/local
TRANSLATIONS_PREFIX:=$(PREFIX)/share/translations/
TRANS_DIR=$(TRANSLATIONS_PREFIX)/

TARGETS=

ifdef QBTranslateSources
	TARGETS += translate-sources
endif

ifdef QBTranslateSettings
	TARGETS += translate-settings
endif

ifdef QBTranslateJSON
	TARGETS += translate-json
endif

ifdef QBTranslateHints
	TARGETS += translate-hints
endif

ifdef QBTranslatePODir
	TARGETS += translate-orig
endif

ifdef QBTranslatePartnerPolicies
        TARGETS += translate-partner-polices
endif

QBName ?= $(SvName)
QBName ?= $(kpLibraryName)
QBTranslationDomain ?= default

compile:

clean:

install-directory:
	@install -d $(TRANS_DIR)/orig
	@install -d $(TRANS_DIR)/templates

translate-sources: install-directory
	@echo "Generating translation templates of sources for $(QBName)"
	@xgettext --no-location --from-code utf-8 -s -d $(QBTranslationDomain) -o $(TRANS_DIR)/templates/$(QBName)_sources.pot $(QBTranslateSources)

translate-settings: install-directory
	@echo "Generating translation templates of settings for $(QBName)"
	@$(PREFIX)/share/translations/extract-xml-i18n-texts --no-location --sort-output -d $(QBTranslationDomain) -o $(TRANS_DIR)/templates/$(QBName)_settings.pot $(QBTranslateSettings)

translate-json: install-directory
	@echo "Generating translation templates of JSON files for $(QBName)"
	@$(PREFIX)/share/translations/extract-json-i18n-texts -o $(TRANS_DIR)/templates/$(QBName)_json.pot $(QBTranslateJSON)

translate-hints: install-directory
	@echo "Generating translation templates of JSON hints files for $(QBName)"
	@$(PREFIX)/share/translations/extract-hints-i18n-texts -o $(TRANS_DIR)/templates/$(QBName)_hints.pot $(QBTranslateHints)

translate-partner-polices: install-directory
	@echo "Generating translation templates of partner JSON files for $(QBName)"
	@$(PREFIX)/share/translations/extract-json-partner-polices-texts -o $(TRANS_DIR)/templates/$(QBName)_partners.pot $(QBTranslatePartnerPolicies)

translate-orig: install-directory
	@echo "Installing translations for $(QBName)"
	@install -d "$(TRANS_DIR)/"; \
	 domains=`find $(QBTranslatePODir) -maxdepth 1 -mindepth 1 -type d`; \
	 for domain in $$domains; do \
	    domainName=`basename $$domain`; \
	    install -d "$(TRANS_DIR)/$$domainName"; \
		for lang in `ls $(QBTranslatePODir)/$$domainName/*.po`; do \
			install -d "$(TRANS_DIR)/orig/$$domainName"; \
			install -m 0644 $$lang "$(TRANS_DIR)/orig/$$domainName/";\
		 done \
	 done

translate: $(TARGETS)

new-lang-%:
	@lang=$(subst new-lang-,,$@); \
	 echo -e "msgid \"\"\n msgstr \"\"" | msginit -l $$lang.UTF8 -o $$lang.pot --no-translator -i -

