# Calculate dependencies of all source files using gcc's -MM option.
#
# There is an uglish hack for old make (pre 3.81) compatibility adjoins lines
# ending with double slashes and consumes leading whitespace on the following
# line.
#$(foreach src_file,$(SV_LIB_sources),$(eval $(shell $(CC) $(CFLAGS) -MM $(src_file) | sed -e ':a;$!N;s/[ ]*\\\n[ ]*/ /;ta')))
#$(foreach src_file,$(SvAllSourceFiles),$(shell $(CC) $(CPPFLAGS) $(SvExternalCPPFLAGS) -MM $(src_file) -MQ $(SvBuildDir)objects-static/$(src_file).o -o /tmp/foo.d))

#$(foreach src_file,$(SvAllSourceFiles),$(eval $(call SvDependencyRuleForSourceFile $(src_file))))

