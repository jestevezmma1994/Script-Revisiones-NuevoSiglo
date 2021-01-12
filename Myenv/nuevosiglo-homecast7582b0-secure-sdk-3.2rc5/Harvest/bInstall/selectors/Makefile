# Sanity-checker for OTA Selector presence

all: install

install: 
ifeq ($(wildcard $(DESTDIR)/etc/SELECTOR),)
	@$(warning "*******************************************************************************")
	@$(warning "*******************************************************************************")
	@$(warning "*******************************************************************************")
	@$(warning "********* ERROR! No /etc/SELECTOR found in your target root!!!        *********")
	@$(warning "********* Make sure you include Files/Resources/Selectors/c.comp      *********")
	@$(warning "********* into your prj file as selector installation was moved there *********")
	@$(warning "*******************************************************************************")
	@$(warning "*******************************************************************************")
	@$(warning "*******************************************************************************")
	$(error "Please Fix your project file")
endif

clean:
	@echo nothing to do for clean
