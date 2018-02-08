# $Id: utEnv.mk 79821 2017-04-25 01:50:09Z kit.lee $
#

include $(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST)))../env.mk
include $(DEV_DIR)/alticore.mk

INCLUDES += $(foreach i, $(SM_DIR)/include $(MT_DIR)/include $(ID_DIR)/include $(UL_DIR)/include $(UT_DIR)/util/include, $(IDROPT)$(i))

ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
INCLUDES += $(IDROPT)$(UL_DIR)/include/windows-odbc
else
INCLUDES += $(IDROPT)$(UL_DIR)/include/unix-odbc
endif

LFLAGS += $(foreach i, $(ALTI_HOME)/lib $(MT_DIR)/lib $(QP_DIR)/lib $(RP_DIR)/lib $(SM_DIR)/lib $(ID_DIR)/lib $(UL_DIR)/lib $(UT_DIR)/util/lib, $(LDROPT)$(i))

SHARDFLAGS:=$(DEFOPT)COMPILE_SHARDCLI

ODBCCLI_LIB = odbccli
UTIL_LIB = altiutil
ID_LIBS = $(foreach i, $(UTIL_LIB) id pd $(ODBCCLI_LIB), $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)

JAVAC   = $(JAVA_HOME)/bin/javac

JAVA    = $(JAVA_HOME)/bin/java

JAR     = $(JAVA_HOME)/bin/jar

JAVALIB = $(JAVA_HOME)/lib


#########################################
# implicit rules for UT only
#########################################
$(TARGET_DIR)/%.$(OBJEXT): $(DEV_DIR)/%.cpp
	$(COMPILE_IT)

$(TARGET_DIR)/%_sd.$(OBJEXT): $(DEV_DIR)/%.cpp
	$(COMPILE_IT) $(SHARDFLAGS)

$(TARGET_DIR)/%.$(OBJEXT): $(DEV_DIR)/%.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

$(TARGET_DIR)/%)_sd.$(OBJEXT): $(DEV_DIR)/%.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(SHARDFLAGS) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

$(TARGET_DIR)/%.$(OBJEXT): $(DEV_DIR)/%.s
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

$(TARGET_DIR)/%.$(OBJEXT): $(DEV_DIR)/%.S
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

$(TARGET_DIR)/%_shobj.$(OBJEXT): $(DEV_DIR)/%.cpp CPPFLAGS+=$(PIC)
	mkdir -p $(dir $@)
	$(COMPILE_IT)

$(TARGET_DIR)/%_shobj.$(OBJEXT): $(DEV_DIR)/%.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(PIC) $(CC_OUTPUT_FLAG)$@ $<

$(TARGET_DIR)/%_shobj.$(OBJEXT): $(DEV_DIR)/%.S
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(PIC) $(CC_OUTPUT_FLAG)$@ $<

