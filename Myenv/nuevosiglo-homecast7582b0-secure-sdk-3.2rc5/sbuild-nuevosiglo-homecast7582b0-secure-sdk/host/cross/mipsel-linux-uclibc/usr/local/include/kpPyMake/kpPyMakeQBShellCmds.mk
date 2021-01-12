WhitelistBuildDir := $(SRM_DESTDIR)/../QBShellCmdsWhitelists

kpQBShellCmdsWhitelists += $(filter $(addprefix $(WhitelistBuildDir)/,$(kpPyPkgDeps)),$(wildcard $(WhitelistBuildDir)/*))

ifneq ($(kpQBShellCmdsWhitelists),)

# Determine component name and type
ifneq ($(kpLibraryName),)
kpQBShellCmdsWhitelistName := $(strip $(kpLibraryName))
compile: generate-shell_cmds-whitelist
generate-shell_cmds-whitelist: $(WhitelistBuildDir)/$(kpQBShellCmdsWhitelistName)
else ifneq ($(kpExecName),)
kpQBShellCmdsWhitelistName := $(strip $(kpExecName))
compile: generate-shell_cmds-whitelist
install: install-shell_cmds-whitelist
generate-shell_cmds-whitelist: $(WhitelistBuildDir)/$(kpQBShellCmdsWhitelistName).final
else
  $(error Cannot determine component name and type)
endif

$(WhitelistBuildDir)/$(kpQBShellCmdsWhitelistName) $(WhitelistBuildDir)/$(kpQBShellCmdsWhitelistName).final: $(kpQBShellCmdsWhitelists)
	@mkdir -p "$(dir $@)"
	@# merge all whitelist from dependencies
	@echo -e "# $(notdir $@): $(notdir $^)" > "$@"
	@awk 'FNR==1&&NR!=1{print""}1' $^ >> "$@"

install-shell_cmds-whitelist: WhitelistInstallDir := $(SRM_DESTDIR)/etc/QBShellCmdsWhitelists
install-shell_cmds-whitelist:
	@install -m 700 -d "$(WhitelistInstallDir)"
	@cat "$(WhitelistBuildDir)/$(kpQBShellCmdsWhitelistName).final" | sed -e '/^#/d' -e '/^$$/d' | sort -u > "$(WhitelistInstallDir)/$(kpQBShellCmdsWhitelistName)"
	@chmod 0600 "$(WhitelistInstallDir)/$(kpQBShellCmdsWhitelistName)"

endif
