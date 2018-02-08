include ../../root.mk
include config.mk
include platform.mk

RUBYSRCS=checkruby.c
PYTSRCS=checkpython.c

ifeq ($(compile_bit),32)
CCFLAGS = $(M32_FLAGS)
else
CCFLAGS = $(M64_FLAGS)
endif

ifeq ($(ACG_CFG_OS), WINDOWS)
RUBY_TARGET=cruby.exe
PYTHON_TARGET=cpython.exe
else
RUBY_TARGET=cruby
PYTHON_TARGET=cpython
endif

clean:
	$(RM) $(RUBY_TARGET)
	$(RM) $(PYTHON_TARGET)

checkruby: $(RUBY_TARGET)
	@$(ECHO) "Done"

checkpython: $(PYTHON_TARGET)
	@$(ECHO) "Done"

$(RUBY_TARGET):
ifeq ($(ACG_CFG_OS), WINDOWS)
	$(CC) $(CCFLAGS) $(RUBYSRCS) $(ruby-includes) $(ruby-ldflags)
else
	$(CC) $(CCFLAGS) -o $(RUBY_TARGET) $(RUBYSRCS) $(ruby-includes) $(ruby-ldflags)
endif

$(PYTHON_TARGET):
ifeq ($(ACG_CFG_OS), WINDOWS)
	$(CC) $(CCFLAGS) $(PYTSRCS) $(python-includes) $(python-ldflags)
else
	$(CC) $(CCFLAGS) -o $(PYTHON_TARGET) $(PYTSRCS) $(python-includes) $(python-ldflags)
endif

