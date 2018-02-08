# $Id: Makefile 68602 2015-01-23 00:13:11Z sbjang $
#

DIRS = src


.PHONY: all $(DIRS) compile archive clean alint clearall sure

all: compile

$(DIRS):
	$(MAKE) -C $@ $(SUBDIR_TARGET)

compile:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=$(COMPILE)

archive:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=archive;

clean:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=clean

alint:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=alint;

clearall:
	for i in $(DIRS); do (cd $$i; $(RM) core *.$(OBJEXT) *.p *.log .pure SunWS_cache;) done

sure:
