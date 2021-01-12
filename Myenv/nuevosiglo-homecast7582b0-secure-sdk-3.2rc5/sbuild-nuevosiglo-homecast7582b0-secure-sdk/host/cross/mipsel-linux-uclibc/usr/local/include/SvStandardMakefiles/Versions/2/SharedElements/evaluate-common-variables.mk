# This part is to be included at the *very* last.
#
# This is an optimization only.
# Otherwise, those variables are being evaluated in each place they are used (and that many times),
# which can take significant amount of time.

ifeq ($(MAKECMDGOALS),compile)

CFLAGS := $(CFLAGS)
CPPFLAGS := $(CPPFLAGS)
LDFLAGS := $(LDFLAGS)
LDLIBS := $(LDLIBS)

endif
