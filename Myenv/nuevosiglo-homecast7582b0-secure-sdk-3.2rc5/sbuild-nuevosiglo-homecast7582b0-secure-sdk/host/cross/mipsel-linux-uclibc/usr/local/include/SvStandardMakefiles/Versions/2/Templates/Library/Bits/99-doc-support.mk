.PHONY: install-doc-sources

SvHelp[install-doc-sources]=Installs source files for generating Doxygen documentation

SvDocVisibility ?= external
SvDocBaseDir := $(subst $(dir $(SRM_ROOT)),,$(SvNowDir))

ifeq ($(SvDocVisibility),internal)
  # only explicitly marked stuff is external
  # everything else is internal
  SvDocSourceFiles[internal] += $(SvDocSourceFiles)
else
  # only explicitly marked stuff is internal
  # everything else is external
  SvDocSourceFiles[external] += $(SvDocSourceFiles)
endif

install-doc-sources: install-doc-examples install-doc-images install-doc-sources-internal install-doc-sources-external

install-doc-examples:
	@set -e
	@for f in $(SvDocExampleFiles); do \
		if [ ! -d "$(SRM_ROOT)/docs/src/examples" ]; then install -d "$(SRM_ROOT)/docs/src/examples"; fi; \
		install -m 0644 $$f "$(SRM_ROOT)/docs/src/examples/"; \
	done

install-doc-images:
	@set -e
	@for f in $(SvDocImageFiles); do \
		if [ ! -d "$(SRM_ROOT)/docs/src/images" ]; then install -d "$(SRM_ROOT)/docs/src/images"; fi; \
		install -m 0644 $$f "$(SRM_ROOT)/docs/src/images/"; \
	done

install-doc-sources-%:
	@set -e
	@for f in $(SvDocSourceFiles[$*]); do \
		if [ ! -d "$(SRM_ROOT)/docs/src-$*/$(SvDocBaseDir)" ]; then install -d "$(SRM_ROOT)/docs/src-$*/$(SvDocBaseDir)"; fi; \
		install -m 0644 $$f "$(SRM_ROOT)/docs/src-$*/$(SvDocBaseDir)/"; \
	done
