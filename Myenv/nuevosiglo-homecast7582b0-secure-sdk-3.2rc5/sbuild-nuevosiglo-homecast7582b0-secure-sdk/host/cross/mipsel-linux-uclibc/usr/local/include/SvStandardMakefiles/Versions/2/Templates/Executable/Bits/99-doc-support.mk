# The doc rule, y
doc:
	doxygen SvName=$(SvName) $(SvStandardMakefilesInstance)Files/Library.doxyfile
