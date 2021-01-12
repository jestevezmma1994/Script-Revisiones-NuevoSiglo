# Sanity checks on variables defined by the programmer

SvDescription           ?= $(error $(call SvUndefinedVariable,SvDescription))
SvExecutablesSourceFiles?= $(wildcard $(SvSourcesDir)*.[scm]) $(wildcard $(SvSourcesDir)*.cpp)
