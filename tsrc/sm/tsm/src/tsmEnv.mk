include $(ALTIBASE_DEV)/env.mk
include $(SM_DIR)/lib/sm_objs.mk

# SM侩 何啊可记 贸府
INCLUDES += $(foreach i,$(MT_DIR)/include $(ST_DIR)/include $(SM_DIR)/include $(SM_DIR)/smu,$(IDROPT)$(i))
LFLAGS   += $(foreach i,$(ALTI_HOME)/lib $(PD_DIR)/lib $(ID_DIR)/lib $(SM_DIR)/lib $(MT_DIR)/lib ,$(LDROPT)$(i))

INCLUDES += $(IDROPT)$(DEV_DIR)/tsrc/sm/tsm/src/lib $(IDROPT).
LFLAGS   += $(LDROPT)$(DEV_DIR)/tsrc/sm/tsm/src/lib

TSM_DIR=$(DEV_DIR)/tsrc/sm/tsm
TSMLIB=$(TSM_DIR)/src/lib/libtsm.$(LIBEXT)

DEPLIBS =  $(TSMLIB) $(SMLIB) $(ALTI_HOME)/lib/libid.$(LIBEXT)

#SMLIB = $(LIBOPT)sm$(LIBAFT)
SMLIB = $(ALTI_HOME)/lib/$(LIBPRE)sm.$(LIBEXT)

ID_LIB = $(foreach i,id pd,$(LIBOPT)$(i)$(LIBAFT))

#########################################
# implicit rules for sm/tsm only
#########################################
%.$(OBJEXT): %.cpp
	$(COMPILE_IT)

%.$(OBJEXT): %.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

%.$(OBJEXT): %.s
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

%.$(OBJEXT): %.S
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

%_shobj.$(OBJEXT): %.cpp
	mkdir -p $(dir $@)
	$(COMPILE.cc) $(INCLUDES) $(PIC) $(CC_OUTPUT_FLAG)$@ $<

%_shobj.$(OBJEXT): %.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(PIC) $(CC_OUTPUT_FLAG)$@ $<

