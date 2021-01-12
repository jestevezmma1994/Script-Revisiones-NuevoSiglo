SvStandardMakefiles = $(CROSS_ROOT)/usr/local/include/SvStandardMakefiles
SvMakefileTemplate = Executable
include $(SvStandardMakefiles)/Versions/2/Before.mk

SvName = CubiTV-test
SvDescription = Tests for internal CubiTV components
SvBinPrefix = /usr/local/tests/

SvRequires = check CubiTV QBPlatformHAL

SvExecutableSourceFiles = $(shell find . -name '*.c')
NoSourceFiles := $(shell find ./General -name '*.c')
SvExecutableSourceFiles := $(filter-out $(NoSourceFiles),$(SvExecutableSourceFiles))

include $(SvStandardMakefiles)/Versions/2/After.mk
