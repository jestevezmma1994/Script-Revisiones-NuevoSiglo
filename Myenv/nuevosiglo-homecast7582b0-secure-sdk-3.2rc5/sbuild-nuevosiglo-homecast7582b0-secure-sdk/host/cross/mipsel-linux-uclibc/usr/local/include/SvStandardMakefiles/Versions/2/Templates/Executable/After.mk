# Sanity checks on variables defined by the programmer

SvName                  ?= $(error $(call SvUndefinedVariable,SvName))
SvDescription           ?= $(error $(call SvUndefinedVariable,SvDescription))
SvTextDomain			?= $(SvName)

ifeq ($(SvDirectoryLayoutConvention),sentivision)
SvSourcesDir            ?= Sources/
SvHeadersDir            ?= Headers/
SvTranslationsDir       ?= Translations/
SvExecutableSourceFiles?= $(wildcard $(SvSourcesDir)*.[scm]) $(wildcard $(SvSourcesDir)*.cpp)
SvMessageCatalogFiles   ?= $(wildcard $(SvTranslationsDir)*.po)
else ifeq ($(SvDirectoryLayoutConvention),unix)
SvSourcesDir            ?= src/
SvHeadersDir            ?= include/
SvTranslationsDir       ?= po/
SvExecutableSourceFiles	?= $(wildcard $(SvSourcesDir)*.[scm]) $(wildcard $(SvSourcesDir)*.cpp)
SvMessageCatalogFiles   ?= $(wildcard $(SvTranslationsDir)*.po)
else ifeq ($(SvDirectoryLayoutConvention),custom)
SvExecutableSourceFiles	?= $(error You selected custom directory layout convention but did not define SvExecutableSourceFiles)
SvMessageCatalogFiles   ?=
else ifeq ($(SvDirectoryLayoutConvention),flat)
SvSourcesDir            ?= .
SvHeadersDir            ?= .
SvExecutableSourceFiles ?= $(shell find $(SvSourcesDir) -type f -a \( -name '*\.[csm]' -o -name '*\.cpp' -o -name '*\.sx' -o -name '*\.cc' \))
SvMessageCatalogFiles   ?= $(wildcard *.po)
else
$(error Invalid value of SvDirectoryLayoutConvention, allowed values are 'sentivision', 'unix', 'flat' and 'custom')
endif

SvSanityCheckSourceFiles ?= $(SvExecutableSourceFiles)

ifeq ($(filter -static,$(LDFLAGS)),-static)
SvPkgConfigFlags	+= --static
endif
