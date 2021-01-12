# External dependencies are tracked with the pkg-conifg tool.
# I want three separate set of options from pkg-config: 
# CFLAGS  - C compiler and preprocessor options (which is usually CPPFLAGS used
#           to pass -I around)
# LDFLAGS - Linker options, usually -Lpathname but sometimes various arcane
# -rdynamic and other things that don't start with -L
# LDLIBS  - List of libraries something needs to link with: -lfoo -lbar
# 
# They are extracted to special variables to avoid calling $(shell) too often


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(SvRequires),)
ifneq ($(shell $(SvPkgConfig) --short-errors --print-errors $(SvRequires) && echo yes),yes)
$(error "Pkg-config detected dependency problem. Please analyze the set of required libraries to fix the problem")
endif
CFLAGS += $(filter-out -I%,$(shell $(SvPkgConfig) --cflags $(SvRequires)))
CPPFLAGS += $(shell $(SvPkgConfig) --cflags-only-I $(SvRequires))
LDFLAGS += $(shell $(SvPkgConfig) --libs-only-L $(SvRequires))
LDLIBS += $(shell $(SvPkgConfig) --libs-only-l $(SvRequires))
endif
endif

describe-dependencies:
	@echo "Tests depends on the following packages: $(SvRequires)"
	@for dep in $(SvRequires); do \
		if $(SvPkgConfig) --exists $$dep; then \
			echo "Package '$$dep' is available, version `$(SvPkgConfig) --modversion $$dep`"; \
			echo "CPPFLAGS=`$(SvPkgConfig) $$dep --cflags-only-I`"; \
			echo "CPPFLAGS+CFLAGS=`$(SvPkgConfig) $$dep --cflags`"; \
			echo "LDFLAGS=`$(SvPkgConfig) $$dep --libs-only-L`"; \
			echo "LDLIBS=`$(SvPkgConfig) $$dep --libs-only-l`"; \
		else \
			echo "Package '$$dep' is not available"; \
		fi \
	done
