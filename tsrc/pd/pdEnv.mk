include $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))/../../env.mk
include $(PD_DIR)/lib/pd_objs.mk

#########################################
# implicit rules for pd/tsrc only
#########################################
%.o: %.cpp
	$(COMPILE_IT)

%.o: %.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

%.o: %.s
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

%.o: %.S
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(CC_OUTPUT_FLAG)$@ $<

%_shobj.o: %.cpp
	mkdir -p $(dir $@)
	$(COMPILE.cc) $(INCLUDES) $(PIC) $(CC_OUTPUT_FLAG)$@ $<

%_shobj.o: %.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(INCLUDES) $(PIC) $(CC_OUTPUT_FLAG)$@ $<
