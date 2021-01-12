# Workspace support
#
# Workspace is a environment that can be used to build native libraries and
# applications without the necessity of root access or any other 3rd party
# tools like silo or srm.
#
# Workspaces take care of pkg-config configuration *outside* the scope of
# standard makefiles.
#
# Workspaces are tested on Mac OS X 10.5.2, Ubuntu 7.10 and Ubuntu 8.04
SvWorkspaceRoot	?=
ifneq ($(SvWorkspaceRoot),)
CPPFLAGS		+= -I$(DESTDIR)$(SvIncludePrefix)
LDFLAGS			+= -L$(DESTDIR)$(SvLibPrefix)
endif
