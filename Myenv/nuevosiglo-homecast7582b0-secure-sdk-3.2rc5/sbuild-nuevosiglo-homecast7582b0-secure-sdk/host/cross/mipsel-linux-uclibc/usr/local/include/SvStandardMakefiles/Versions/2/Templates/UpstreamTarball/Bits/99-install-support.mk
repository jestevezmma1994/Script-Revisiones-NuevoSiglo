# Install support
.PHONY: install install-all install-pkg-config dist dist-view install-cross install-cross-libraries install-cross-executables install-cross-headers install-cross-pkg-config install-target install-target-libraries install-target-executables install-runtime-data

install: $(SvInstallTargets)

# install selected components from "install-pre"'s directory to final destdirs
install-all: install-cross install-target

# FIXME: use variable hiding /lib/pkgconfig
install-pkg-config:
	@set -e
	@$(call SvInstallDirectory,$(SvPrefix)/lib/pkgconfig/,$(CROSS_ROOT))
	@$(call SvInstallFile,$(SvPkgConfigFile),$(SvTemporaryDir),$(SvPrefix)/lib/pkgconfig,$(CROSS_ROOT))

install-cross: install-cross-libraries install-cross-executables install-cross-headers install-cross-pkg-config

install-target: install-target-libraries install-target-executables install-runtime-data


define SvInstallFiles
if [ ! -d "$3/$(dir $2)" ]; then \
	$(call SvInstallDirectory,$(dir $2),$3) \
fi; \
for file in $$(find $(SvPreinstallDir)/$(dir $2) -name $(notdir $2)); do \
	if [ -h $$file ]; then \
		$(call SvPlainShellCommand,ln -sf $$(readlink $$file) $3/$(dir $2)/$$(basename $$file),$4) \
	else \
		$(call SvPlainShellCommand,install -m $1 $$file $3/$(dir $2),$4) \
	fi; \
done;
endef


install-cross-libraries:
	@set -e
	@$(foreach base_name,$(SvTargetLibrariesBaseNames),$(call SvInstallFiles,755,$(base_name)$(SvSharedLibrarySuffixPattern),$(CROSS_ROOT),Installing dynamic library to cross-root))
	@$(foreach base_name,$(SvTargetLibrariesBaseNames),$(call SvInstallFiles,644,$(base_name)$(SvStaticLibrarySuffixPattern),$(CROSS_ROOT),Installing static library to cross-root))

install-cross-executables:
	@set -e
	@$(foreach base_name,$(SvTargetExecutablesBaseNames),$(call SvInstallFiles,755,$(base_name)$(SvExecutableSuffixPattern),$(CROSS_ROOT),Installing executable to cross-root))

install-cross-headers:
	@set -e
	@$(call SvInstallDirectory,/usr/include,$(CROSS_ROOT))
	@for file in $$(find $(SvPreinstallDir) -name '*.[ch]' -type f -printf '%P\n'); do \
		$(call SvPlainShellCommand,install -m 0644 -D $(SvPreinstallDir)/$$file $(CROSS_ROOT)/$$file,Installing header file /$$file) \
	done
	@for file in $$(find $(SvPreinstallDir) -name '*.[ch]' -type l -printf '%P\n'); do \
		$(call SvPlainShellCommand,ln -sf $$(readlink $(SvPreinstallDir)/$$file) $(CROSS_ROOT)/$$file,Installing header file /$$file) \
	done

install-cross-pkg-config:
	@set -e
	@$(call SvInstallDirectory,/usr/lib/pkgconfig,$(CROSS_ROOT))
	@for file in $$(find $(SvPreinstallDir) -name '*.pc' -type f -printf '%P\n'); do \
		$(call SvPlainShellCommand,install -m 0644 -D $(SvPreinstallDir)/$$file $(CROSS_ROOT)/$$file,Installing pkg-config file /$$file) \
	done


install-target-libraries:
ifneq ($(SvSkipInstallToTarget),yes)
	@set -e
	@$(foreach base_name,$(SvTargetLibrariesBaseNames),$(call SvInstallFiles,755,$(base_name)$(SvSharedLibrarySuffixPattern),$(DESTDIR),Installing dynamic library to target-root))
endif

install-target-executables:
ifneq ($(SvSkipInstallToTarget),yes)
	@set -e
	@$(foreach base_name,$(SvTargetExecutablesBaseNames),$(call SvInstallFiles,755,$(base_name)$(SvExecutableSuffixPattern),$(DESTDIR),Installing executable to target-root))
endif

define SvInstallRuntimeFiles
if [ ! -d "$2/$(dir $1)" ]; then \
	$(call SvInstallDirectory,$(dir $1),$2) \
fi; \
$(call SvPlainShellCommand,tar -C $(SvPreinstallDir)/$(dir $1) -cp $(notdir $1) | tar -C $2/$(dir $1) -xp,$3)
endef

install-runtime-data:
	@set -e
	@$(foreach base_name,$(SvRuntimeDataBaseNames),$(call SvInstallRuntimeFiles,$(base_name),$(DESTDIR),Installing runtime data files))
