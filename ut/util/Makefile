# $Id: Makefile 70397 2015-04-20 01:43:07Z heesuk.shin $
#

include ../utEnv.mk


ABC_HEADER = include/abc_*.h

DIRS = ute utt lib
include $(ALTIDEV_HOME)/makefiles/moddep-common.mk


.PHONY: all $(DIRS) compile install archive clean alint clearall sure pure dep

all: compile install

$(DIRS):
	$(MAKE) -C $@ $(SUBDIR_TARGET)

compile:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=$(COMPILE)

install:
	$(COPY) $(ABC_HEADER) $(ALTI_HOME)/include

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
