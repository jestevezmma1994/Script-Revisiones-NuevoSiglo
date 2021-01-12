# SILO/SRM support
#
# SILO and SRM define the variable SMR_DESTDIR that defaults to root
# directory of the target device file-system.
#
# Since almost everything puts shared libraries there it's a good idea to
# extend linker search path to those locations.
ifdef SRM_DESTDIR
LDFLAGS			+= -L$(SRM_DESTDIR)/usr/lib
LDFLAGS			+= -L$(SRM_DESTDIR)/usr/local/lib
endif

# workaround: silo does not provide DESTDIR variable in targets other than "preinstall" and "install".
# when invoking make with any other target (e.g. some sub target of "install") we might still need it.
# the value that DESTDIR is to be set to, is well know, though ;)
# -deffer 2009.05.22
ifdef SRM_DESTDIR
DESTDIR ?= $(SRM_DESTDIR)
endif
TARGET_ROOT := $(DESTDIR)

# SILO and SRM define the variable CROSS_ROOT (or SRM_CROSS_ROOT) to the root
# directory of a cross-compilation filesystem that is not installed on the
# device but is necessary for development.
#
# This place contains static libraries, header files and other similar items.
ifdef CROSS_ROOT
CPPFLAGS		+= -I${CROSS_ROOT}/usr/include
LDFLAGS			+= -L${CROSS_ROOT}/usr/lib
CPPFLAGS		+= -I${CROSS_ROOT}/usr/local/include
LDFLAGS			+= -L${CROSS_ROOT}/usr/local/lib
SvPkgConfigFlags+= --define-variable=CROSS_ROOT=$(CROSS_ROOT)

# SILO and SRM use different meaning for cross/host than typical GNU tools.
# GNU's model is stronger but irrelevant to our case (currently).
# GNU:
#  build  - tools are built here
#  host   - tools will be invoked here
#  target - tools generate code for this thing
# SILO/SRM:
#  host   - tools are build here
#  cross  - tools will be invoked here and work with this architecture
#
# GNU allows you to build on one thing then make use of the tools you just
# built on something totally different to actually generate code for third
# architecture. This is useful when host is limited and cannot normally build
# the compiler (like for example, windows)
export CROSS_COMPILING=yes
export SvHostTriplet=$(CROSS_ARCH)
export SvBuildTriplet=$(BUILD_ARCH)
endif

ifdef SRM_PROJECT
SvTargetProject=$(SRM_PROJECT)
endif
