#this makefile should support install and clean targets
ifeq (,${DESTDIR})
  $(error DESTDIR is not defined)
endif
ifeq (,${SPECFILE})
  $(error SPECFILE is not defined)
endif

install:
	install -m 0755 ${S_INSTALL_UPGRADE_PLATFORM_PROC} ${DESTDIR}/bin/${SPECFILE}
	echo "not implemented"

clean:
	echo "not implemented"
