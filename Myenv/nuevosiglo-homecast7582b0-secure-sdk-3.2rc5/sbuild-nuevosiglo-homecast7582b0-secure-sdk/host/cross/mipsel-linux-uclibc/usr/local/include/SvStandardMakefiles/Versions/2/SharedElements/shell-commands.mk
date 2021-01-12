# Some shell abstractions are defined below, basically the goal is to
# support Linux (using GNU utils), Mac OS X (using BSD utils) and Win32 
# with MIN GW (using subset of GNU utils).
#
# The full list of required functionality is listed below:
# - print some text
# - make a directory with full path
# - copy a single file, setting permissions if possible
# - copy an entire tree to some location
# - remove an entire tree
# - remove a single file
# - run a command with output redirection to a file
# - check if a file is empty
# - touch a file
#
# I used the following commands to implement those:
SvEchoCommand=command printf "%b\n" "$1";
SvInstallDirectoryCommand=install -d "$1";
SvInstallFileCommand=install -m 0644 "$1" "$2";
SvInstallTreeCommand=tar -cC $1 . | tar -xC $2;
SvInstallExecutableFileCommand=install -m 0755 "$1" "$2";
SvRemoveFileCommand=rm -f "$1";
SvRemoveTreeCommand=rm -rf "$1";
SvCatCommand=cat "$1";
SvIsFileEmptyCommand=test -s "$1";
SvTemporaryFileGeneratorCommand=mktemp -t SvStandardMakefilesProxy.XXXXXXX;
SvTouchCommand=touch "$1";

# The functions above are used in the most important primitives used by
# everything else: SvShellCommand and SvPlainShellCommand

# If the user wants to see verbose commands then print the actual commands
# instead of localized descriptions.
ifndef SvVerboseCommands
# Plain shell command
# Args:
#  - $1 is the actuall command
#  - $2 is the localized description
define SvPlainShellCommand
$(call SvEchoCommand,$2) \
( $1 );
endef

# Smart shell command
# Args:
#  - $1 is the actuall comman
#  - $2 is the localized description
define SvShellCommand
$(call SvEchoCommand,$2) \
tmp=$$($(call SvTemporaryFileGeneratorCommand)); \
( $1 ) >$$tmp 2>&1; \
retval=$$?; \
if [ "x$$retval" != "x0" ]; then \
	$(call SvEchoCommand,$(call SvTextGeneratorCommandFailed,$$retval)) \
	$(call SvEchoCommand,$1) \
	$(call SvCatCommand,$$tmp) \
	$(call SvRemoveFileCommand,$$tmp) \
	exit $$retval; \
else \
	if $(call SvIsFileEmptyCommand,$$tmp) then \
		$(call SvEchoCommand,$(call SvTextGeneratorCommandLeftSomeOutput)) \
		$(call SvEchoCommand,$1) \
		$(call SvCatCommand,$$tmp) \
	fi; \
	$(call SvRemoveFileCommand,$$tmp) \
fi;
endef

else

# no SvVerboseCommands
define SvPlainShellCommand
( set -x; $1 );
endef
define SvShellCommand
( set -x; $1 );
endef

endif

# Some nice wrappers around basic shell commands
SvInstallDirectory=$(call SvShellCommand,$(call SvInstallDirectoryCommand,$2$1),$(call SvTextGeneratorCreatingDirectory,$1,$2))
SvInstallFile=$(call SvShellCommand,$(call SvInstallFileCommand,$2$1,$4$3),$(call $5,$1,$2,$3,$4))
SvInstallExecutableFile=$(call SvShellCommand,$(call SvInstallExecutableFileCommand,$2$1,$4$3),$(call $5,$1,$2,$3,$4))
SvInstallDirectoryTree=$(call SvShellCommand,$(call SvInstallTreeCommand,$2$1,$4$3),$(call SvTextGeneratorInstallingDirectoryTree,$1,$2,$3,$4))
SvTouchStamp=$(call SvShellCommand,$(call SvTouchCommand,$(call SvStampPath,$1)),$(call SvTextGeneratorTouchingStamp,$1))

SvStampPath=$(SvBuildDir)$1
