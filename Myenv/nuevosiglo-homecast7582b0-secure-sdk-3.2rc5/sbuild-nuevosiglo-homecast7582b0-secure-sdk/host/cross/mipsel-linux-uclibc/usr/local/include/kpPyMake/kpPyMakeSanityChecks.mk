kpProjectName:=$(SRM_PROJECT)
kpComponentName:=$(notdir $(SRM_COMPONENT_FILE))
kpTmpDir ?= $(SRM_ROOT)/_tmp/$(kpComponentName)

kpSanityChecksTarges :=

kpDisableStaticAnalysisTools ?= $(SvDisableStaticAnalysisTools)
kpEnableSanityChecks ?= $(SvEnableSanityChecks)

ifneq ($(kpPyLibraryName),)

ifneq ($(kpPyHeadersBaseDir),)
kpSanityHeadersDir=$(kpPyHeadersBaseDir)
else
kpSanityHeadersDir=include/
endif

ifneq ($(kpPySrcBaseDir),)
kpSanitySrcDir=$(kpPySrcBaseDir)
else
kpSanitySrcDir=src/
endif

ifeq ($(kpName),)
kpName=$(kpPyLibraryName)
endif

kpMakefileName=$(firstword $(MAKEFILE_LIST))

check-header-sanity:
	@echo "--== check header sanity ==--"
	@CFLAGS="$(CPPFLAGS) $(CFLAGS)" check_header_sanity.sh -c $(kpName) -t $(kpTmpDir)/header_sanity/ $(kpSanityHeaders)

check-source-sanity:
	@echo "--== check source sanity ==--"
	@CFLAGS="$(CPPFLAGS) $(CFLAGS)" check_source_sanity.sh -c $(kpName) -t $(kpTmpDir)/source_sanity/ $(kpSanitySrcFiles)

check-class-declarations:
	@echo "--== check class declarations ==--"
	@CFLAGS="$(CPPFLAGS) $(CFLAGS)" check_class_declarations.py -c $(kpName) $(kpSanityCSrcFiles)

check-lib-sanity:
	@echo "--== check lib sanity ==--"
	@check_lib_sanity.sh $(CROSS_ROOT)/usr/local/lib/lib$(kpName).so -t $(kpTmpDir)/lib_sanity/

check-makefile-sanity:
	@echo "--== check makefile sanity ==--"
	@check_makefile_sanity.sh $(kpMakefileName) $(kpName)

check-shell-sanity:
	@echo "--== check shell sanity ==--"
	@CFLAGS="$(CPPFLAGS) $(CFLAGS)" check_shell_sanity.sh -c $(kpName) -t $(kpTmpDir)/shell_sanity/ $(kpSanityShellScripts)

# Conditionally enable sanity checks.
ifeq ($(kpEnableSanityChecks),yes)

disabledSanity:=$(findstring sanity_checker,$(kpDisableStaticAnalysisTools))

ifeq ($(disabledSanity),)
# sanity_checker is not disabled
ifneq ($(wildcard $(kpSanityHeadersDir)),)
kpSanityHeaders=$(shell find $(kpSanityHeadersDir) -type f -name '*.h' -o -name '*.hpp')
else
kpSanityHeaders=
endif
# check also internal headers in source directory
ifneq ($(wildcard $(kpSanitySrcDir)),)
kpSanityHeaders += $(shell find $(kpSanitySrcDir) -type f -name '*.h' -o -name '*.hpp')
endif
ifneq ($(kpSanityHeaders),)
kpSanityChecksTarges += check-header-sanity
endif

ifneq ($(wildcard $(kpSanitySrcDir)),)
kpSanitySrcFiles=$(shell find $(kpSanitySrcDir) -type f -name '*.c' -o -name '*.cpp' -o -name '*.s' -o -name '*.sx')
else
kpSanitySrcFiles=$(shell find . -type f -name '*.c' -o -name '*.cpp' -o -name '*.s' -o -name '*.sx')
endif
ifneq ($(kpSanitySrcFiles),)
kpSanityChecksTarges +=  check-source-sanity
endif

ifneq ($(wildcard $(kpSanitySrcDir)),)
kpSanityCSrcFiles=$(shell find $(kpSanitySrcDir) -type f -name '*.c')
else
kpSanityCSrcFiles=$(shell find . -type f -name '*.c')
endif
ifneq ($(kpSanityCSrcFiles),)
kpSanityChecksTarges += check-class-declarations
endif

kpSanityShellScripts=$(shell find -type f -exec file {} \; | grep -i 'shell script' | sed 's|:.*||')
ifneq ($(kpSanityShellScripts),)
kpSanityChecksTarges += check-shell-sanity
endif

kpSanityChecksTarges += check-lib-sanity
kpSanityChecksTarges += check-makefile-sanity

endif # disabledSanity

endif # enable sanity checks

endif # kpPyLibraryName

install-sanity-targets: $(kpSanityChecksTarges)
