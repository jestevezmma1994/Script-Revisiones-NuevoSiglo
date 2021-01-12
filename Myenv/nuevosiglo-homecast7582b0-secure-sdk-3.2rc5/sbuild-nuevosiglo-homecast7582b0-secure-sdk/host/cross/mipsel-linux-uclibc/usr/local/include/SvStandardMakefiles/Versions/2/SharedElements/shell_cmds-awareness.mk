.PHONY: generate-shell_cmds-whitelist install-shell_cmds-whitelist

SvHelp[generate-shell_cmds-whitelist] = Merge shell_cmds whitelists from dependencies
SvHelp[install-shell_cmds-whitelist] = Install shell_cmds whitelist

WhitelistBuildDir := $(SRM_DESTDIR)/../QBShellCmdsWhitelists
SvQBShellCmdsWhitelists += $(filter $(addprefix $(WhitelistBuildDir)/,$(SvRequires)),$(wildcard $(WhitelistBuildDir)/*))

ifneq ($(SvQBShellCmdsWhitelists),)

# Determine component name
ifneq ($(SvProvides),)
  SvQBShellCmdsWhitelistName := $(strip $(SvProvides))
else ifneq ($(SvName),)
  SvQBShellCmdsWhitelistName := $(strip $(SvName))
else ifneq ($(kpLibraryName),)
  SvQBShellCmdsWhitelistName := $(strip $(kpLibraryName))
else ifneq ($(kpExecName),)
  SvQBShellCmdsWhitelistName := $(strip $(kpExecName))
else
  $(error Cannot determine component name)
endif

# Determine component name
ifneq ($(SvMakefileTemplateOrig),)
  QBShellCmdsComponentType := $(SvMakefileTemplateOrig)
else ifneq ($(SvMakefileTemplate),)
  QBShellCmdsComponentType := $(SvMakefileTemplate)
else ifneq ($(kpLibraryName),)
  QBShellCmdsComponentType := Library
else ifneq ($(kpExecName),)
  QBShellCmdsComponentType := Executable
else
  $(error Cannot determine component type)
endif

ifneq ($(filter $(QBShellCmdsComponentType),Executable Test),)
compile: generate-shell_cmds-whitelist
install: install-shell_cmds-whitelist
generate-shell_cmds-whitelist: $(WhitelistBuildDir)/$(SvQBShellCmdsWhitelistName).final
else ifeq ($(QBShellCmdsComponentType),Library)
compile: generate-shell_cmds-whitelist
generate-shell_cmds-whitelist: $(WhitelistBuildDir)/$(SvQBShellCmdsWhitelistName)
endif

$(WhitelistBuildDir)/$(SvQBShellCmdsWhitelistName) $(WhitelistBuildDir)/$(SvQBShellCmdsWhitelistName).final: $(SvQBShellCmdsWhitelists)
	@mkdir -p "$(dir $@)"
	@# merge all whitelist from dependencies
	@echo -e "# $(notdir $@): $(notdir $^)" > "$@"
	@awk 'FNR==1&&NR!=1{print""}1' $^ >> "$@"

install-shell_cmds-whitelist: WhitelistInstallDir := $(SRM_DESTDIR)/etc/QBShellCmdsWhitelists
install-shell_cmds-whitelist:
	@install -m 700 -d "$(WhitelistInstallDir)"
	@cat "$(WhitelistBuildDir)/$(SvQBShellCmdsWhitelistName).final" | sed -e '/^#/d' -e '/^$$/d' | sort -u > "$(WhitelistInstallDir)/$(SvQBShellCmdsWhitelistName)"
	@chmod 0600 "$(WhitelistInstallDir)/$(SvQBShellCmdsWhitelistName)"

endif
