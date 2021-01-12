.PHONY: extract-rtti

SvRTTIFiles = $(addprefix $(CROSS_ROOT)/usr/local/share/qbr/$(SvName)/,$(patsubst %.c,%.astz,$(filter %.c,$(SvLibrarySourceFiles))))

extract-rtti: $(SvRTTIFiles)

$(CROSS_ROOT)/usr/local/share/qbr/$(SvName)/%.astz: %.c
	@mkdir -p $(dir $@)
	@qbr-dump-ast -m $(SvName) -z -o $@ $(CPPFLAGS) $<
