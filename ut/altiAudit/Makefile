# $Id: Makefile 20029 2007-01-20 03:02:25Z gskim $
#

DIRS=src

all: compile
.PHONY: compile $(DIRS)

$(DIRS):
	$(MAKE) -C $@ $(SUBDIR_TARGET)

compile:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=$(COMPILE)
	
clean:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=clean
