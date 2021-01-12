###############################################
# Env variables to set before including this:
###############################################
#
# All variables that can be lseft uset - note that it is *very* different than setting to "".
#
# kpPyLibraryName
#   or
# kpPyExecName
#   or
# kpPyCustomExecNames (list of exec names to be compiled)
#      and
#    kpPyCustomExecSources_NAME (can be left unset, then "NAME.c" is assumed)
#
# kpPyHeadersBaseDir (defaults to "include/")
# kpPyHeaders (must be prefixed with $kpPyHeadersBaseDir, can be left unset)
#
# kpPySrcBaseDir (defaults to "src/")
# kpPySrcFiles (must be prefixed with $kpPySrcBaseDir, can be left unset)
#
# kpPyPkgDeps (pkg deps, passed to our own .pc)
# kpPyPkgDepsPriv (more pkg deps, NOT passed to our own .pc)
#
# kpPyExportedFlags (passed as CFlags to our .pc, to use them in compilation as well, you have to pass them to C{,XX,PP}Flags)
#
# kpPyExtraTargets (extra makefile targets to be done at the end)
#
# CFLAGS
# CXXFLAGS
# CPPFLAGS
# LDLIBS
# LDFLAGS
#
# kpPyTmpDir (directory to put compilation temps and products)
#
# CROSS_ROOT
# TARGET_ROOT
#
# CC   (defaults to gcc)
# CXX  (defaults to g++)
#
# and, of course:
#
# PATH
# LD_LIBRARY_PATH
#
###############################################

#BLA := $(shell date +%S.%N 1>&2 &&  echo start 1>&2)

.EXPORT_ALL_VARIABLES:

all:
	@kpPyMake $(MAKECMDGOALS)


clean: all

clean-force: all

compile: all

install: all $(kpPyExtraTargets)
	@true
#	@date +%S.%N 1>&2 &&  echo make end 1>&2
