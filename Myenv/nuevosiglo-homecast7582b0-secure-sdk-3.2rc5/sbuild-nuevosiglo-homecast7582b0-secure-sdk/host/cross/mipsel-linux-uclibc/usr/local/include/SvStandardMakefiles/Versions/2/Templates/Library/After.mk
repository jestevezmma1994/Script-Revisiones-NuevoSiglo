# Sanity checks on variables defined by the programmer

# TODO: check for a yes/no value
#SvCompileUnitTests      ?= no
#SvCompileStaticLibrary  ?= no
#SvCompileSharedLibrary  ?= yes

ifeq ($(SvStandardMakefilesIgnoreLibraryVersion),yes)
SvMajorVersion := 1
SvMinorVersion := 0
SvRevision     := 0
endif

SvName                  ?= $(error $(call SvUndefinedVariable,SvName))
SvDescription           ?= $(error $(call SvUndefinedVariable,SvDescription))
SvMajorVersion          ?= $(error $(call SvUndefinedVariable,SvMajorVersion))
SvMinorVersion          ?= $(error $(call SvUndefinedVariable,SvMinorVersion))
SvRevision              ?= $(error $(call SvUndefinedVariable,SvRevision))

SvUnitTestRequires	+= $(SvName)-uninstalled


ifeq ($(SvDirectoryLayoutConvention),custom)

SvLibrarySourceFiles    ?= $(error You selected custom directory layout convention but did not define SvLibrarySourceFiles)
SvLibraryHeaderFiles    ?= $(error You selected custom directory layout convention but did not define SvLibraryHeaderFiles)
SvUnitTestSourceFiles   ?= $(error You selected custom directory layout convention but did not define SvUnitTestSourceFiles)
SvMessageCatalogFiles   ?=

else

ifeq ($(SvDirectoryLayoutConvention),mac)
SvSourcesDir            ?= Sources/
SvHeadersDir            ?= Headers/
SvTestsDir              ?= Tests/
SvTranslationsDir       ?= Translations/
else ifeq ($(SvDirectoryLayoutConvention),unix)
SvSourcesDir            ?= src/
SvHeadersDir            ?= include/
SvTestsDir              ?= test/
SvTranslationsDir       ?= po/
else
$(error Invalid value of SvDirectoryLayoutConvention, allowed values are 'mac', 'unix' and 'custom')
endif

SvLibraryHeaderFiles    ?= $(shell find $(SvHeadersDir) -type f)
SvLibrarySourceFiles    ?= $(shell find $(SvSourcesDir) -type f -a \( -name '*\.[csm]' -o -name '*\.cpp' -o -name '*\.sx' -o -name '*\.cc' \))
SvUnitTestSourceFiles   ?= $(wildcard $(SvTestsDir)*.[scm]) $(wildcard $(SvTestsDir)*.cpp)
SvMessageCatalogFiles   ?= $(wildcard $(SvTranslationsDir)*.po)

endif

SvSanityCheckHeaderFiles ?= $(SvLibraryHeaderFiles) $(shell find $(SvSourcesDir) -type f -a \( -name '*.h' -o -name '*.hpp' \))
SvSanityCheckSourceFiles ?= $(SvLibrarySourceFiles)

SvTextDomain            ?= $(SvName)
