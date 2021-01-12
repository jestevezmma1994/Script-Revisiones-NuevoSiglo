# redirect to kelpiePyMake

ifneq ($(SvStandardMakefilesUseKelpiePy),no)

  SvMakefileTemplateOrig := $(SvMakefileTemplate)

  ifeq ($(SvMakefileTemplate),Library)
    SvMakefileTemplate := kpPy
  endif
  ifeq ($(SvMakefileTemplate),Executable)
    SvMakefileTemplate := kpPy
  endif
  ifeq ($(SvMakefileTemplate),Tests)
    SvMakefileTemplate := kpPy
  endif

endif
