# The compile rule, yeah easy
.PHONY: compile
SvHelp[compile]=Builds all products and their dependencies (library, pkg-config files, unit tests)
compile: $(SvCompileTargets)
