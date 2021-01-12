.PHONY: manifest
manifest:
	@echo "Name: $(SvName)"
	@echo "Description: $(SvDescription)"
	@echo "Requires: $(SvRequires)"
	@echo "-----------------------------------------"
	@echo "Following options are active during compilation of this executable:"
	@echo "CFLAGS: $(CFLAGS)" 
	@echo "CPPFLAGS: $(CPPFLAGS)" 
	@echo "LDFLAGS: $(LDFLAGS)" 
	@echo "LDLIBDS: $(LDLIBS)" 
