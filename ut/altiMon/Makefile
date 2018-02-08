# $Id: Makefile 75190 2016-04-21 07:44:19Z bethy $
#

include ../utEnv.mk

include moddep.mk

DIRS = com.altibase.picl com.altibase.altimon
include $(ALTIDEV_HOME)/makefiles/moddep-common.mk


.PHONY: all $(DIRS) compile install archive clean alint clearall sure pure dep

all: compile

$(DIRS):
	$(MAKE) -C $@ $(SUBDIR_TARGET)

compile:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=$(COMPILE)

dist:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=dist

archive:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=archive

clean:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=clean

alint:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=alint

clearall:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=clean

sure:

pure:
	$(MAKE) INSTALL=install_pure
