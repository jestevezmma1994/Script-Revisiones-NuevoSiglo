ifeq (,${DESTDIR})
  $(error DESTDIR is not defined)
endif
ifeq (,${SPECFILE})
  $(error SPECFILE is not defined)
endif

S_INSTALL_UPGRADE_USR_BASE_PART_NO?=5
S_INSTALL_ENCRYPT_NVRAM ?= no

USE_QB_IMAGE=$(shell mapreader.pl $(S_MAP_DIR)/all.maps attr name=nvram | grep -o "qbimage")

ifneq ($(USE_QB_IMAGE),)
    CFLAGS+=-I$(CROSS_ROOT)/usr/local/include -DENCRYPT_NVRAM -DUSE_QB_BINARY_IMAGE -DCONFIG_ENCRYPT_USE_OPENSSL
    LDFLAGS += -lcrypto
    QB_OBJS = $(BUILDDIR)/QBBinaryImage.o
    LOCAL_FLAGS=-I$(BUILD_ROOT)/include/ -DENCRYPT_NVRAM -DUSE_QB_BINARY_IMAGE -DCONFIG_ENCRYPT_USE_OPENSSL -lcrypto -ldl
    LOCAL_SRC=$(DESTDIR)/../qbimage/QBBinaryImage.c
    LOCAL_INC=$(DESTDIR)/../qbimage/QBBinaryImage.h
else
  ifeq (yes,$(S_INSTALL_ENCRYPT_NVRAM))
    CFLAGS+=-DENCRYPT_NVRAM
    LDFLAGS+=-lcrypto
    LOCAL_FLAGS=-DENCRYPT_NVRAM -lcrypto
  endif
endif

$(BUILDDIR)/QBBinaryImage.o: $(DESTDIR)/../qbimage/QBBinaryImage.c $(DESTDIR)/../qbimage/QBBinaryImage.h
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILDDIR)/crc32.o: crc32.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILDDIR)/setnv.o: setnv.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILDDIR)/setnv: $(QB_OBJS) $(BUILDDIR)/setnv.o $(BUILDDIR)/crc32.o

compile: $(BUILDDIR)/setnv $(TOOLSDIR)/nvimage

clean:

$(TOOLSDIR)/nvimage: nvimage.c crc32.c $(LOCAL_SRC) $(LOCAL_INC)
	srm-local-run gcc -static -o $@ nvimage.c crc32.c $(LOCAL_SRC) $(LOCAL_FLAGS)

install:
	install -m 0755 ${S_INSTALL_UPGRADE_PLATFORM_PROC} ${DESTDIR}/bin/${SPECFILE}
	sed -e "s/_UPGRADE_USR_BASE_PART_NO_/$(S_INSTALL_UPGRADE_USR_BASE_PART_NO)/g" \
		-e "s/_UPGRADE_FILES_/$(UPGRADE_FILES)/g" \
		-i ${DESTDIR}/bin/${SPECFILE}
	mkdir -p $(DESTDIR)/usr/local/bin
	install -m 0755 $(BUILDDIR)/setnv $(DESTDIR)/usr/local/bin/
	srm-local-run install -s -m 0755 $(TOOLSDIR)/nvimage $(BUILD_ROOT)/usr/bin/
