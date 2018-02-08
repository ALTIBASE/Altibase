# Set the default build action
.PHONY: all
all:

# Build libraries last
.PHONY: lib
lib: $(filter-out lib,$(DIRS))

# Build msg before others in $(DIRS)
ifeq ($(filter msg,$(DIRS)),msg)
.PHONY: msg
$(filter-out msg,$(DIRS)): msg
endif
