prebuild:

compile:

WARNING=\n\
*******\n\
*******\n\
SRM_DRIVER_PACK variable seems to be empty! It means that driver pack in your base is not defined, likely the base was created before\n\
support for driver packs was introduced (2016/07/13). Still you need to stay vigilant:\n\
- make sure your sbuild actually contains all necessary drivers for your respective platform\n\
- consult with Cubiware if you sbuild/SDK shall have the driver pack defined anyway\n\
*******\n\
*******\n

install:
ifneq ($(SRM_PACKAGE),)
	tar xzf $(SRM_PACKAGE) -C $(SRM_ROOT) --overwrite --overwrite-dir
else
	@echo -e "$(WARNING)"
endif

clean:
	rm -rf $(SRM_PACKAGE)
