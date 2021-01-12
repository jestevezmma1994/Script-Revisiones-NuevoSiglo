
################################################
# special target for installing seleted headers
# to predefined "doc" dir
################################################

kpDocBaseDir := $(subst $(dir $(SRM_ROOT)),,$(shell pwd))

kpDocVisibility ?= $(SvDocVisibility)

# Observation: executing an empty target takes as long as executing full target (with copying files etc).
# If we only execute target(s) that have actual files to process, it's N times faster!
# It actually _is_ noticable (cubitv empty run goes from 11s down to 7s).
kpDocExampleFiles ?= $(SvDocExampleFiles)
kpDocImageFiles ?= $(SvDocImageFiles)

kpDocTargets :=
ifneq ($(kpDocExampleFiles),)
  kpDocTargets += install-doc-examples
endif
ifneq ($(kpDocImageFiles),)
  kpDocTargets += install-doc-images
endif

kpDocSourceFiles ?= $(SvDocSourceFiles)
kpDocSourceFiles[internal] ?= $(SvDocSourceFiles[internal])
kpDocSourceFiles[external] ?= $(SvDocSourceFiles[external])

ifeq ($(kpDocVisibility),internal)
  # only explicitly marked stuff is external
  # everything else is internal
  kpDocSourceFiles[internal] += $(kpDocSourceFiles)
else
  # only explicitly marked stuff is internal
  # everything else is external
  kpDocSourceFiles[external] += $(kpDocSourceFiles)
endif

ifneq ($(kpDocSourceFiles[internal]),)
  kpDocTargets += install-doc-sources-internal
endif
ifneq ($(kpDocSourceFiles[external]),)
  kpDocTargets += install-doc-sources-external
endif


install-doc-sources: $(kpDocTargets)

install-doc-examples:
	@for f in $(kpDocExampleFiles); do \
	  if [ ! -d "$(SRM_ROOT)/docs/src/examples" ]; then install -d "$(SRM_ROOT)/docs/src/examples"; fi; \
	  true || echo -e installing [$$f] to [$(SRM_ROOT)/docs/src/examples/]; \
	  install -m 0644 $$f "$(SRM_ROOT)/docs/src/examples/"; \
	done

install-doc-images:
	@for f in $(kpDocImageFiles); do \
	  if [ ! -d "$(SRM_ROOT)/docs/src/images" ]; then install -d "$(SRM_ROOT)/docs/src/images"; fi; \
	  true || echo -e installing [$$f] to [$(SRM_ROOT)/docs/src/images/]; \
	  install -m 0644 $$f "$(SRM_ROOT)/docs/src/images/"; \
	done

install-doc-sources-%:
	@for f in $(kpDocSourceFiles[$*]); do \
	  if [ ! -d "$(SRM_ROOT)/docs/src-$*/$(kpDocBaseDir)" ]; then install -d "$(SRM_ROOT)/docs/src-$*/$(kpDocBaseDir)"; fi; \
	  true || echo -e installing [$$f] to [$(SRM_ROOT)/docs/src-$*/$(kpDocBaseDir)/]; \
	  install -m 0644 $$f "$(SRM_ROOT)/docs/src-$*/$(kpDocBaseDir)/"; \
	done
