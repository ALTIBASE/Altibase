INCLUDES = $(IDROPT)$(ALTIBASE_HOME)/include $(IDROPT). $(IDROPT)$(ALTIBASE_HOME)/../src/pd/include
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


%_shobj.$(OBJEXT): %.cpp
	$(Q) echo " CC $@"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(SHCOMPILE.cc) $(INCLUDES) $(PIC) $(VERDEFINES) $(CC_OUTPUT_FLAG)$@ $<

%_shobj.$(OBJEXT): %.c
	$(Q) echo " CC $@"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(SHCOMPILE.c) $(INCLUDES) $(PIC) $(VERDEFINES) $(CC_OUTPUT_FLAG)$@ $<
