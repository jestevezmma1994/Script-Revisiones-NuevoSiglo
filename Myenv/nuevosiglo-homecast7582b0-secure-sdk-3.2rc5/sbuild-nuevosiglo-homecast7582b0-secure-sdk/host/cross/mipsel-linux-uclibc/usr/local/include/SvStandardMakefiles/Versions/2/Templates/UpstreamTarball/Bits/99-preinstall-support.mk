# PreInstall support
.PHONY: preinstall  install-pre  install-pre-all

preinstall:
# empty - silo fix -deffer 2009.05.22

SvPreinstallDir ?= $(SvBuildDir)/INSTALL_ROOT

install-pre: $(SvPreinstallTargets)

install-pre-all:
	@$(call SvPlainShellCommand,$(MAKE) DESTDIR=$(SvPreinstallDir) -C $(SvBuildDir)$(SvExtractedSourceDir)/$(SvRelevantSubDirectory) install,$(call SvTextGeneratorInstallingPackage,$(SvName)))
