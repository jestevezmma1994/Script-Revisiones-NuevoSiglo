all: $(addprefix refresh-,$(SvProjectComponents))
clean:
	@echo "Removing workspace for current target platform"
	@rm -rf $(SvWorkspaceBuildRoot)
