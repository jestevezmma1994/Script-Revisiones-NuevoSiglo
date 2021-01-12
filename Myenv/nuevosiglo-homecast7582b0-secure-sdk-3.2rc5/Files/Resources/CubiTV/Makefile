###############################################################
#    !!!  DO NOT CUSTOMIZE THIS FILE FOR YOUR PLATFORM !!!    #
#       use variables described below for that purpose        #
###############################################################

# Variables
# DESTDIR - defines the target's destination directory e.g. 
# Q_RESOURCES_CUBITV_USE_GENERIC - defines if to use the generic setup before using customer
#                                specified setup. In this mode (when yes) the following will be done:
#                              1) copy generic files to target directory
#                              2) copy specific files to target directory (ovewriting if necessary)
#   e.g.
#      Q_RESOURCES_CUBITV_USE_GENERIC=yes
#      Q_RESOURCES_CUBITV_USE_GENERIC=no
#      Q_RESOURCES_CUBITV_USE_GENERIC= (yes will be used)
#
# Q_RESOURCES_CUBITV_CUSTOMER - defines customer's name for specific icons selection
#   e.g. 
#      Q_RESOURCES_CUBITV_CUSTOMER=qb
#      Q_RESOURCES_CUBITV_CUSTOMER=welho
#      Q_RESOURCES_CUBITV_CUSTOMER=iptvamericas
#      Q_RESOURCES_CUBITV_CUSTOMER=telio
#
# Q_RESOURCES_CUBITV_FORMAT - defines graphics format for specific icons selection
#   e.g
#      Q_RESOURCES_CUBITV_FORMAT=ntsc
#      Q_RESOURCES_CUBITV_FORMAT=pal
#      Q_RESOURCES_CUBITV_FORMAT=720p
#
# Note that these variables are used to create a directory name under which
# the files are located:
# NEW: added Q_RESOURCES_CUBITV_CUSTOMER_IMAGES - overrides Q_RESOURCES_CUBITV_CUSTOMER to choose a different
# set of icons.... this is because Q_RESOURCES_CUBITV_CUSTOMER conflicts with App/Applications/CubiTV...
#
#      Q_RESOURCES_CUBITV_SPEC_DIRECTORY=data/$(Q_RESOURCES_CUBITV_CUSTOMER)/$(Q_RESOURCES_CUBITV_FORMAT)
#
# - If the directory is found then all the files in that directory will overwrite the generic
#   files at the destination
# - If such directory is not found then error is returned
#
# Note that is is advisable that these variables are explicitly set, otherwise some default
# values will be used

.EXPORT_ALL_VARIABLES:

# Default configuration 
DESTDIR ?= $(CROSS_ROOT)
RESOURCES_DESTDIR=$(DESTDIR)/usr/local/share/CubiTV/data

Q_RESOURCES_CUBITV_USE_GENERIC?=no
Q_RESOURCES_CUBITV_CUSTOMER?=qb
Q_RESOURCES_CUBITV_FORMAT?=720p

ifneq (${Q_RESOURCES_CUBITV_CUSTOMER_IMAGES},)
Q_RESOURCES_CUBITV_CUSTOMER=${Q_RESOURCES_CUBITV_CUSTOMER_IMAGES}
endif

PROGRESS_RESOURCES_DESTDIR=$(DESTDIR)/usr/local/share/progress

# Initializing temporary variables
Q_RESOURCES_CUBITV_GENERIC_DIRECTORY=generic/$(Q_RESOURCES_CUBITV_FORMAT)
Q_RESOURCES_CUBITV_SPEC_DIRECTORY=$(Q_RESOURCES_CUBITV_CUSTOMER)/$(Q_RESOURCES_CUBITV_FORMAT)

Q_RESOURCES_CUBITV_SPEC_DIRECTORY_NOT_AVAILABLE:=$(shell if [ -d "$(Q_RESOURCES_CUBITV_SPEC_DIRECTORY)" ]; then echo 0; else echo 1; fi;)

install:
ifeq ($(Q_RESOURCES_CUBITV_SPEC_DIRECTORY_NOT_AVAILABLE),1)
	@echo ""
	@echo "********************* CUBITV RESOURCES WARNING *****************"
	@echo "$(Q_RESOURCES_CUBITV_SPEC_DIRECTORY) directory is not available."
	@echo "Please choose another valid configuration if you want your"
	@echo "application to work correctly."
	@echo "**************************************************************"
	@echo ""
endif
	mkdir -p $(RESOURCES_DESTDIR)
#	mkdir -p $(RESOURCES_DESTDIR)/osd
#	mkdir -p $(RESOURCES_DESTDIR)/xml

ifeq ($(Q_RESOURCES_CUBITV_USE_GENERIC),yes)
#	install -m 0644 $(Q_RESOURCES_CUBITV_GENERIC_DIRECTORY)/*.png $(RESOURCES_DESTDIR)/
#	install -m 0644 $(Q_RESOURCES_CUBITV_GENERIC_DIRECTORY)/osd/*.png $(RESOURCES_DESTDIR)/osd
#	install -m 0644 $(Q_RESOURCES_CUBITV_GENERIC_DIRECTORY)/xml/*.xml $(RESOURCES_DESTDIR)/xml
	cp -af $(Q_RESOURCES_CUBITV_GENERIC_DIRECTORY) $(RESOURCES_DESTDIR)
endif

#	install -m 0644 $(Q_RESOURCES_CUBITV_SPEC_DIRECTORY)/*.png $(RESOURCES_DESTDIR)/
#	install -m 0644 $(Q_RESOURCES_CUBITV_SPEC_DIRECTORY)/osd/*.png $(RESOURCES_DESTDIR)/osd
#	install -m 0644 $(Q_RESOURCES_CUBITV_SPEC_DIRECTORY)/xml/*.xml $(RESOURCES_DESTDIR)/xml
	cp -af $(Q_RESOURCES_CUBITV_SPEC_DIRECTORY) $(RESOURCES_DESTDIR)
	install -d $(PROGRESS_RESOURCES_DESTDIR)
	ln -fs ../CubiTV/data/720p $(PROGRESS_RESOURCES_DESTDIR)/data
	find $(RESOURCES_DESTDIR) -type f -exec chmod 0644 '{}' \;
	find $(RESOURCES_DESTDIR) -type d -exec chmod 0755 '{}' \;

compile: 

clean:
