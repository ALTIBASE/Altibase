include $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))/../../env.mk
include $(ID_DIR)/lib/id_objs.mk


#########################################
# implicit rules for id/tsrc only
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
