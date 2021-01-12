SvStandardMakefiles = $(CROSS_ROOT)/usr/local/include/SvStandardMakefiles
SvMakefileTemplate = Executable
include $(SvStandardMakefiles)/Versions/2/Before.mk

SvName = QBTextSubsManager-unit-test
SvDescription = Unit test for QBTextSubsManager library

SvRequires = check SvCore SvFoundation QBTextSubsManager
SvBinPrefix = /usr/local/tests/$(SvName)/
SvInstallTargets += install-data-tests

DataTestsFullPath = $(DESTDIR)$(SvBinPrefix)
#SvExecutableSourceFiles = $(shell find . -name '*.c')
SvExecutableSourceFiles = $(wildcard *.c)

include $(SvStandardMakefiles)/Versions/2/After.mk

install-data-tests:
	install -d $(DataTestsFullPath)
	cp -r data $(DataTestsFullPath)
