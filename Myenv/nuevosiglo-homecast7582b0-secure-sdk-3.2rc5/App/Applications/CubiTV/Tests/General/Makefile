
kpName = CubiTV-tests
kpExecName = General-test

kpLibsDep += check CubiTV QBServices
kpBinPrefix := /usr/local/tests/$(kpName)
kpStrictWarnings=yes
kpInstallExtraTargets += install_scripts
install_scripts:
	install -m 0744 ./src/scripts/server-qbaccessmanagertest.py $(TARGET_ROOT)$(kpBinPrefix)

CFLAGS += -fprofile-arcs -ftest-coverage
LDFLAGS += -lgcov -coverage

include $(CROSS_ROOT)/usr/local/include/kpmake/inc.mk

