PACKAGES_DIR=../../packages
LWM2M_DIR=./AwaLWM2M
LMC_DIR =./LetMeCreateIoT
LIBOBJECTS_DIR = ./libobjects
CONTIKI_PROJECT=weather-station-weather-click-app
VERSION?=$(shell git describe --abbrev=4 --dirty --always --tags)
CONTIKI=../../constrained-os/contiki
CONTIKI_WITH_IPV6 = 1
CONTIKI_WITH_RPL = 0

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -DVERSION='\"$(VERSION)\"'
CFLAGS += -Wall -Wno-pointer-sign -DLWM2M_CLIENT
CFLAGS += -I$(LWM2M_DIR)/api/include

PROJECT_SOURCEFILES += errors-helper.c

ifneq (,$(filter  $(TARGET),seedeye mikro-e))
  CFLAGS += -fno-short-double
  LDFLAGS += -Wl,--defsym,_min_heap_size=64000
endif

APPS += er-coap
APPS += rest-engine
APPS += client
APPS += common
APPS += jsmn
APPS += b64
APPS += hmac
APPS += libobjects

ifeq ($(TARGET),minimal-net)
  APPS += xml
endif

APPDIRS += $(CONTIKI)/platform/mikro-e/dev
APPDIRS += $(LWM2M_DIR)/lib
APPDIRS += $(LWM2M_DIR)/core/src

all: $(CONTIKI_PROJECT)
	xc32-bin2hex $(CONTIKI_PROJECT).$(TARGET)

distclean: cleanall

cleanall:
	rm -f $(CONTIKI_PROJECT).hex
	rm -f symbols.*
	rm -rf $(LWM2M_DIR)
	rm -rf $(LIBOBJECTS_DIR)
	rm -rf $(LMC_DIR)

installDependencies:
	git clone --branch 0.2.1 https://github.com/FlowM2M/AwaLWM2M.git $(LWM2M_DIR)
	git clone https://github.com/CreatorKit/libobjects.git $(LIBOBJECTS_DIR)
	
	# change to master once weather click is merged into master
	git clone --branch dev https://github.com/mtusnio/LetMeCreateIoT.git $(LMC_DIR)
	./LetMeCreateIoT/install.sh $(CONTIKI)

include $(CONTIKI)/Makefile.include
