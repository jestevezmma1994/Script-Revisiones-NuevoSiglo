
# Adding this is harmless, since it can always be stripped out (which release builds do).
ifeq ($(SvTargetPlatformOperatingSystemName),Darwin)
  CFLAGS		+= -g
else
  CFLAGS		+= -ggdb
endif

# You have to explicitly set SvDebugBuild=yes to disable optimizations for all targets.
# Of course, any single makefile can add -Os/1/2/3 independently by itself.
ifneq ($(SvDebugBuild),yes)
# This is the default.
# Since we're embedded let's always try to optimize for code size in builds.
CFLAGS			+= -Os
endif

ifeq ($(SvKeepStack),yes)
CFLAGS		    += -fno-omit-frame-pointer -funwind-tables -fno-exceptions
endif
