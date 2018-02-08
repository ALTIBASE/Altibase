INCLUDES = $(IDROPT)$(ALTIBASE_HOME)/include $(IDROPT).
LIBDIRS = $(LDROPT)$(ALTIBASE_HOME)/lib
LFLAGS += $(LIBDIRS)

########################
#### common rules
########################
%.$(OBJEXT): %.cpp
	$(COMPILE.cc) $(INCLUDES) $(CCOUT)$@ $<

%.p: %.cpp
	$(CXX) $(EFLAGS) $(DEFINES) $(INCLUDES) $< > $@

%.s: %.cpp
	$(CXX) $(SFLAGS) $(DEFINES) $(INCLUDES) $< > $@

%.$(OBJEXT): %.c
	$(COMPILE.c) $(INCLUDES) $(CCOUT)$@ $<

