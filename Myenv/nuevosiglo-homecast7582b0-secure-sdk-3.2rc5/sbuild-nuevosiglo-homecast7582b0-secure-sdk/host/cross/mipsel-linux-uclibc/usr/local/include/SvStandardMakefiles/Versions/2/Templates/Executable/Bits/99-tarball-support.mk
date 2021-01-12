.PHONY: tarball 

tarball:
	set -e
	mkdir -p "$(SvDistDir)"
	$(MAKE) install DESTDIR=$(SvDistDir)
	tar -C "$(SvDistDir)" -zcf $(SvName).tar.gz .
