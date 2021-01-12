# This file contains english text generators
SvTextGeneratorRemovingRecursively=$(SvHilightColor)Removing: $(SvNormalColor)$1$(SvWarningColor) (recursive)$(SvNormalColor)
SvTextGeneratorRemoving=$(SvHilightColor)Removing: $(SvNormalColor)$1
SvTextGeneratorCompilingC=$(SvHilightColor)Compiling: $(SvNormalColor)$1
SvTextGeneratorCompilingCxx=$(SvTextGeneratorCompilingC)
SvTextGeneratorCompilingObjc=$(SvTextGeneratorCompilingC)
SvTextGeneratorCompilingAsm=$(SvTextGeneratorCompilingC)
SvTextGeneratorCreatingPkgConfig=$(SvHilightColor)Creating pkg-config file: $(SvNormalColor)$1
SvTextGeneratorCreatingStaticLibraryFrom=$(SvHilightColor)Creating static library: $(SvNormalColor)$1$(SvHilightColor) from: $(SvNormalColor)$2
SvTextGeneratorCreatingSharedLibraryFrom=$(SvHilightColor)Creating shared library: $(SvNormalColor)$1$(SvHilightColor) from: $(SvNormalColor)$2
SvTextGeneratorLinking=$(SvHilightColor)Linking: $(SvNormalColor)$1
SvTextGeneratorRunningExecutable=$(SvHilightColor)Running: $(SvNormalColor)$1
SvTextGeneratorRunningExecutableViaGdb=$(SvHilightColor)Running: $(SvNormalColor)$1$(SvHilightColor) via debugger$(SvNormalColor)
SvTextGeneratorCommandFailed=$(SvErrorColor)Shell command exited with non-zero code: $1$(SvNormalColor)
SvTextGeneratorCommandLeftSomeOutput=$(SvWarningColor)Shell command left some output$(SvNormalColor)
SvTextGeneratorConfiguringPackageWithOptions=$(SvHilightColor)Configuring package: $(SvNormalColor)$1$(SvHilightColor) with options: $(SvNormalColor)$2
SvTextGeneratorBuildingPackage=$(SvHilightColor)Building package: $(SvNormalColor)$1
SvTextGeneratorInstallingPackage=$(SvHilightColor)Installing package: $(SvNormalColor)$1
SvTextGeneratorCleaningPackage=$(SvHilightColor)Cleaning package: $(SvNormalColor)$1
SvTextGeneratorPatchingFileWithPathPatch=$(SvHilightColor)Patching file: $(SvNormalColor)$1$(SvHilightColor) with path-patch.pl$(SvNormalColor)
SvTextGeneratorTouchingStamp=$(SvHilightColor)Touching stamp file: $(SvNormalColor)$1

SvTextGeneratorCopyingTarball=$(SvHilightColor)Using archive file: $(SvNormalColor)$1$(SvHilightColor) provided by Silo$(SvNormalColor)
SvTextGeneratorExtractingTarball=$(SvHilightColor)Extracting archive file: $(SvNormalColor)$1$(SvHilightColor)...$(SvNormalColor)
SvTextGeneratorApplyingPatch=$(SvHilightColor)Applying patch: $(SvNormalColor)$1

ifeq ($(SvShowsDESTDIR),yes)
SvTextGeneratorCreatingDirectory=$(SvHilightColor)Creating directory: $(SvNormalColor)$1$(SvHilightColor) in: $(SvNormalColor)$2
SvTextGeneratorInstallingPkgConfigFile=$(SvHilightColor)Installing pkg-config file: $(SvNormalColor)$1$(SvHilightColor) to: $(SvNormalColor)$3$(SvHilightColor) in: $(SvNormalColor)$4
SvTextGeneratorInstallingHeaderFile=$(SvHilightColor)Installing header file: $(SvNormalColor)$1$(SvHilightColor) to: $(SvNormalColor)$3$(SvHilightColor) in: $(SvNormalColor)$4
SvTextGeneratorInstallingStaticLibraryFile=$(SvHilightColor)Installing static library file: $(SvNormalColor)$1$(SvHilightColor) to: $(SvNormalColor)$3$(SvHilightColor) in: $(SvNormalColor)$4
SvTextGeneratorInstallingSharedLibraryFile=$(SvHilightColor)Installing shared library file: $(SvNormalColor)$1$(SvHilightColor) to: $(SvNormalColor)$3$(SvHilightColor) in: $(SvNormalColor)$4
SvTextGeneratorInstallingExecutableFile=$(SvHilightColor)Installing executable file: $(SvNormalColor)$1$(SvHilightColor) to: $(SvNormalColor)$3$(SvHilightColor) in: $(SvNormalColor)$4
SvTextGeneratorInstallingDirectoryTree=$(SvHilightColor)Installing directory tree: $(SvNormalColor)$1$(SvHilightColor) to: $(SvNormalColor)$3$(SvHilightColor) in: $(SvNormalColor)$4
else
SvTextGeneratorCreatingDirectory=$(SvHilightColor)Creating directory: ...$(SvNormalColor)$1
SvTextGeneratorInstallingPkgConfigFile=$(SvHilightColor)Installing pkg-config file: $(SvNormalColor)$1$(SvHilightColor) to: ...$(SvNormalColor)$3
SvTextGeneratorInstallingHeaderFile=$(SvHilightColor)Installing header file: $(SvNormalColor)$1$(SvHilightColor) to: ...$(SvNormalColor)$3
SvTextGeneratorInstallingStaticLibraryFile=$(SvHilightColor)Installing static library file: $(SvNormalColor)$1$(SvHilightColor) to: ...$(SvNormalColor)$3
SvTextGeneratorInstallingSharedLibraryFile=$(SvHilightColor)Installing shared library file: $(SvNormalColor)$1$(SvHilightColor) to: ...$(SvNormalColor)$3
SvTextGeneratorInstallingExecutableFile=$(SvHilightColor)Installing executable file: $(SvNormalColor)$1$(SvHilightColor) to: ...$(SvNormalColor)$3
SvTextGeneratorInstallingDirectoryTree=$(SvHilightColor)Installing directory tree: $(SvNormalColor)$1$(SvHilightColor) to: ...$(SvNormalColor)$3
endif
SvTextGeneratorRefreshingComponent=$(SvHilightColor)Refreshing component: $(SvNormalColor)$1

SvTextGeneratorFormatingMessageCatalog=$(SvHilightColor)Converting message catalog: $(SvNormalColor)$1
SvTextGeneratorCompilingMessageCatalog=$(SvHilightColor)Compiling message catalog: $(SvNormalColor)$1
SvTextGeneratorInstallingMessageCatalogFile=$(SvHilightColor)Installing message catalog file: $(SvNormalColor)$1$(SvHilightColor) to: ...$(SvNormalColor)$3

SvTextGeneratorCheckingHeaderFilesSanity=$(SvHilightColor)Checking header files sanity$(SvNormalColor)
SvTextGeneratorCheckingSourceFilesSanity=$(SvHilightColor)Checking source files sanity$(SvNormalColor)
SvTextGeneratorCheckingClassDeclarations=$(SvHilightColor)Checking SvFoundation class declarations$(SvNormalColor)
SvTextGeneratorCheckingSharedLibrarySanity=$(SvHilightColor)Checking shared library sanity$(SvNormalColor)
SvTextGeneratorCheckingMakefileSanity=$(SvHilightColor)Checking makefile sanity$(SvNormalColor)
