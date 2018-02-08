DIRS=lib src

all: compile
include moddep.mk
.PHONY: compile $(DIRS)

$(DIRS):
	$(MAKE) -C $@ $(SUBDIR_TARGET)

compile:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=$(COMPILE)

clean:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=clean
