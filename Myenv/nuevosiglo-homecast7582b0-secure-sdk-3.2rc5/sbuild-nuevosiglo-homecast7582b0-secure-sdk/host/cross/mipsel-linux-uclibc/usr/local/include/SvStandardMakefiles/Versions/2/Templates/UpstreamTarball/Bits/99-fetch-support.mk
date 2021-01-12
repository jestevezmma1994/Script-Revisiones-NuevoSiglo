# Prepare sources
.PHONY: prebuild-fetch prebuild-extract prebuild-patch

ifeq ($(SRM_DOWNLOADS_DIR),)

SvTarballFilePath = $(SvBuildDir)$(SvTarballFile)

$(SvTarballFilePath): $(SvTarballFile)
	@set -e
	@install -d $(SvBuildDir)
	@$(call SvShellCommand,mv "$<" "$@",$(call SvTextGeneratorCopyingTarball,$(SvTarballFile)))

else

SvTarballFilePath = $(SRM_DOWNLOADS_DIR)/$(SvTarballFile)

endif

prebuild-fetch: $(SvTarballFilePath)

#  -----  unpack -----

$(SvBuildDir)$(SvExtractedSourceDir): $(SvTarballFilePath)
	@set -e
	@install -d $(SvBuildDir)
ifeq ($(SvTarballFileExt),.tar.gz)
	@$(call SvShellCommand,tar -xf $^ --use-compress-program=pigz -C $(SvBuildDir),$(call SvTextGeneratorExtractingTarball,$^))
else ifeq ($(SvTarballFileExt),.tgz)
	@$(call SvShellCommand,tar -xf $^ --use-compress-program=pigz -C $(SvBuildDir),$(call SvTextGeneratorExtractingTarball,$^))
else ifeq ($(SvTarballFileExt),.tar.bz2)
	@$(call SvShellCommand,tar -xf $^ --use-compress-program=pbzip2 -C $(SvBuildDir),$(call SvTextGeneratorExtractingTarball,$^))
else ifeq ($(SvTarballFileExt),.tar.xz)
	@$(call SvShellCommand,tar -Jxf $^ -C $(SvBuildDir),$(call SvTextGeneratorExtractingTarball,$^))
else
	$(error SvStandardMakefiles don\'t know how to extract $(SvTarballFileExt) archives!)
endif

prebuild-extract: $(SvBuildDir)$(SvExtractedSourceDir)

#  -----  patch -----

prebuild-patch:
	@set -e
	@if [ ! -z "$(SvPatches)" ]; then \
		for f in $$(cat $(SvPatches) | awk '/^(\+\+\+|---) / { print $$2 }' | cut -d/ -f2- | uniq); do \
			if [ -f $(SvBuildDir)/$(SvExtractedSourceDir)/$$f ]; then \
				chmod u+w $(SvBuildDir)/$(SvExtractedSourceDir)/$$f; \
			fi; \
		done; \
	fi
	@$(foreach patch,$(SvPatches),$(call SvShellCommand,patch -d $(SvBuildDir)/$(SvExtractedSourceDir) --quiet -p1 < $(patch),$(call SvTextGeneratorApplyingPatch,$(patch))))
