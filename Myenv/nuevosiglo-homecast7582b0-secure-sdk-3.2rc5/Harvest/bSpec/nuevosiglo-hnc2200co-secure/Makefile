SRM_ENV?=/dev/null

clean compile:
	@echo nothing to do here

install-hotplug-filters:
	install -d $(DESTDIR)/etc/hotplug_filters/block/
	install -m 0664 hotplug_filter_usb_block  $(DESTDIR)/etc/hotplug_filters/block/welho

install: install-hotplug-filters
	mkdir -p $(DESTDIR)/../bSpec/
	cp -a * $(DESTDIR)/../bSpec/
	ln -sf /usr/share/zoneinfo/UTC $(DESTDIR)/../bSpec/localtime
	echo "S_INSTALL_BSPEC=$(DESTDIR)/../bSpec/" >> $(SRM_ENV)
