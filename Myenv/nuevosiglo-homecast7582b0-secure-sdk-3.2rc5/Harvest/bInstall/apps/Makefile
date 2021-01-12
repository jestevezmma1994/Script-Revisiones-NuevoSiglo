S_INSTALL_APPS_SCRIPT ?= generic

.PHONY: install_apps clean_apps

install_apps:
	mkdir -p $(DESTDIR)/opt
	ln -sf /tmp/apps $(DESTDIR)/opt/apps
	mkdir -p $(DESTDIR)/etc/init.d
	install -m 0755 scripts/$(S_INSTALL_APPS_SCRIPT) $(DESTDIR)/etc/init.d/apps

clean_apps:
	rm -f $(DESTDIR)/opt/apps/
	rm -f $(DESTDIR)/etc/init.d/apps

install: install_apps
clean: clean_apps
