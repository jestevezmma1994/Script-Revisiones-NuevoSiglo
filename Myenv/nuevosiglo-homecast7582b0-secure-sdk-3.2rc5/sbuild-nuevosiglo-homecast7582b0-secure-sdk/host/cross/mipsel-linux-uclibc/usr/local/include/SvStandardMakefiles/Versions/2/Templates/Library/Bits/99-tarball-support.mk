.PHONY: tarball 
SvHelp[tarball]=Create a tarball of installed files
tarball:
	set -e
	mkdir -p "$(SvDistDir)"
	$(MAKE) install DESTDIR=$(SvDistDir)
	tar -C "$(SvDistDir)" -zcf $(SvName)-$(SvMajorVersion).$(SvMinorVersion).$(SvRevision).tar.gz .
