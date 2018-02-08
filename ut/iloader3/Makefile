# $Id: Makefile 79821 2017-04-25 01:50:09Z kit.lee $
#

include ../utEnv.mk

DIRS=lib src

include $(ALTIDEV_HOME)/makefiles/moddep-common.mk

all: compile
.PHONY: compile $(DIRS)

$(DIRS):
	$(MAKE) -C $@ $(SUBDIR_TARGET)

compile:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=$(COMPILE)
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=$(COMPILE)
	
archive:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=archive;

clean:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=clean

alint:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=alint;

clearall:
	for i in $(DIRS); do (cd $$i; $(RM) core *.$(OBJEXT) *.p *.log .pure SunWS_cache;) done

ctags:
	find `pwd` \( -name "*.h" -o -name "*.cpp" -o -name "*.i" \) -exec ctags -f ../../tags -a {} \;
	-$(RM) tags; ln -s ../../tags tags
	-cd src; $(RM) tags; ln -s ../../../tags tags
	-cd include; $(RM) tags; ln -s ../../../tags tags

sure:

