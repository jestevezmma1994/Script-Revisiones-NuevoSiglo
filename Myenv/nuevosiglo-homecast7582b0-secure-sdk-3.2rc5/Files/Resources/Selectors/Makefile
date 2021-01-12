# Support for installing OTA selector

S_INSTALL_SELECTOR?=$(SRM_PROJECT)

all: install

compile:

install: 
	@mkdir -p $(DESTDIR)/etc
	@if test -f files/$(S_INSTALL_SELECTOR); then\
		install -m 0644 files/$(S_INSTALL_SELECTOR) $(DESTDIR)/etc/SELECTOR;\
	else\
		echo "********************************************************" >&2;\
		echo "********************************************************" >&2;\
		echo "********************************************************" >&2;\
		echo "********* WARNING! Using default OTA selector! *********" >&2;\
		echo "********************************************************" >&2;\
		echo "********************************************************" >&2;\
		echo "********************************************************" >&2;\
		install -m 0644 files/default $(DESTDIR)/etc/SELECTOR;\
	fi

clean:
	@echo nothing to do for clean
