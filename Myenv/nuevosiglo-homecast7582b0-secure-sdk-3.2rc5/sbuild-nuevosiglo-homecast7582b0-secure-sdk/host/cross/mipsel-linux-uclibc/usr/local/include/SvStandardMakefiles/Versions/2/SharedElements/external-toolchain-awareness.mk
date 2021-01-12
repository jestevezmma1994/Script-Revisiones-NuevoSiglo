# XXX: this is not used probably
SvToolchainRoot	?=
ifneq ($(SvToolchainRoot),)
CPPFLAGS		+= -I$(SvToolchainRoot)/$(SvHostSpec)/usr/include
LDFLAGS			+= -L$(SvToolchainRoot)/$(SvHostSpec)/usr/lib
CPPFLAGS		+= -I$(SvToolchainRoot)/$(SvHostSpec)/usr/local/include
LDFLAGS			+= -L$(SvToolchainRoot)/$(SvHostSpec)/usr/local/lib
endif
