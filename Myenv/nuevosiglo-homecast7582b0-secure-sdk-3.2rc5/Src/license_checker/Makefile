scripts_dir := $(BUILD_ROOT)/bin
configs_dir := $(BUILD_ROOT)/usr/local/share/license_checker


prebuild compile clean:
	@:

install: install-scripts install-configs

install-scripts: check_licenses.py
	@install -d -m 0755 $(scripts_dir)
	@install -m 0755 $^ $(scripts_dir)/

install-configs: db.json
	@install -d -m 0755 $(configs_dir)
	@install -m 0644 $^ $(configs_dir)/
