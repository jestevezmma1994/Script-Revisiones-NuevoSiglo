.PHONY: compile-executable
compile-executable: $(SvProductsDir)$(SvExecutableFile)

SvExecutableObjects=$(call SvTurnSourcesToObjects,$(SvExecutableSourceFiles))

ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) $^ $(LDLIBS)
else ifeq ($(SvTargetPlatformOperatingSystemName),Linux)
 ifneq ($(filter 2.14 2.15 2.16 2.16.1 2.17,$(V_BINUTILS)),)
  SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) $^ $(LDLIBS)
 else
  SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) -Wl,--build-id $^ $(LDLIBS)
 endif
else ifeq ($(SvTargetPlatformOperatingSystemName),SunOS)
SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) $^ $(LDLIBS)
else ifeq ($(SvTargetPlatformOperatingSystemName),FreeBSD)
SvExecutableLinkCommand=$(LINK.o) $(OUTPUT_OPTION) $^ $(LDLIBS)
else
# TODO: Add Win32 support (mingw)
# TODO: Add BSD support (should be idential to linux, right?)
$(error Standard Makefiles does not know how to link executables for your system)
endif

$(SvProductsDir)$(SvExecutableFile): $(SvExecutableObjects)
	@set -e
	@mkdir -p `dirname $@`
	@$(call SvShellCommand,$(SvExecutableLinkCommand),$(call SvTextGeneratorLinking,$(SvExecutableFile)))
