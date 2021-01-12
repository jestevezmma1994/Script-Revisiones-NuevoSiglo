.PHONY: compile-executables

ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) $^ $(LDLIBS)
else ifeq ($(SvTargetPlatformOperatingSystemName),Linux)
 ifneq ($(filter 2.14 2.15 2.16 2.16.1 2.17,$(V_BINUTILS)),)
  SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) $^ $(LDLIBS)
 else
  SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) -Wl,--build-id $^ $(LDLIBS)
 endif
else ifeq ($(SvTargetPlatformOperatingSystemName),SunOS)
SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) -Wl,--build-id $^ $(LDLIBS)
else
# TODO: Add Win32 support (mingw)
# TODO: Add BSD support (should be idential to linux, right?)
$(error Standard Makefiles does not know how to link executables for your system)
endif

define ExecutableTemplate
$$(SvProductsDir)$1: $$(SvTemporaryDir)$(call SvObjectFileGenerator,$1)
	@mkdir -p $(dir $(SvProductsDir)$1)
	@$$(call SvShellCommand,$$(SvExecutableLinkCommand),$$(call SvTextGeneratorLinking,$1))
endef

$(foreach exec,$(SvExecutableFiles),$(eval $(call ExecutableTemplate,$(exec))))

compile-executables: $(addprefix $(SvProductsDir),$(SvExecutableFiles))
