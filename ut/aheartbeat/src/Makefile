include ../../utEnv.mk 
.SUFFIXES: .c .$(OBJEXT)

# for ALTICORE
HBP_INCLUDES += $(ALTICORE_INCLUDES) $(IDROPT)./msg $(IDROPT)./

HBP_INCLUDES += $(IDROPT)$(UL_DIR)/include
ifeq "$(OS_TARGET)" "INTEL_WINDOWS"
HBP_INCLUDES += $(IDROPT)$(UL_DIR)/include/windows-odbc
else
HBP_INCLUDES += $(IDROPT)$(UL_DIR)/include/unix-odbc
endif

# for valgrind option.
ifeq "$(ALTI_HBP_VALGRIND_FLAG)" "ON"
CFLAGS += $(DEFOPT)_VALGRIND_
endif

ifeq "$(ALTI_CFG_OS)" "LINUX"
CFLAGS += -Werror
endif

OUT = aheartbeat$(BINEXT)

SRCS= hbpDaemonize.c \
	  hbpParser.c \
	  hbpServerCheck.c \
	  hbpSock.c hbpListener.c \
	  hbpMain.c

LIBS += $(ALTICORE_STATIC_LIB) $(ODBC_LIBS)

OBJS=$(SRCS:.c=.$(OBJEXT)) ./msg/hbpMsg.$(OBJEXT)


.PHONY: all
all: compile

.PHONY: compile
compile: install

$(OBJS): | build_msg

build: $(OUT)
$(OUT): $(OBJS)
		$(LD) $(LFLAGS) $(OBJS) $(LDOUT)$(OUT) $(LIBOPT)$(ODBCCLI_LIB)$(LIBAFT) $(LIBS)
	cp -f $(OUT) $(ALTIDEV_HOME)/ut/aheartbeat/src/bin/
	cp -f altibaseFailureEvent.sh $(ALTIDEV_HOME)/ut/aheartbeat/src/bin/
	cp -f remoteNodeFailureEvent.sh $(ALTIDEV_HOME)/ut/aheartbeat/src/bin/

clean: clean_msg
	-$(RM) $(OUT) *.$(OBJEXT) $(ALTIDEV_HOME)/ut/aheartbeat/src/bin/$(OUT) 
	-$(RM) $(ALTI_HOME)/aheartbeat/bin/$(OUT)
	-$(RM) $(ALTI_HOME)/aheartbeat/msg/msgEN100.mdl

.PHONY: build_msg
build_msg:
	$(MAKE) -C msg build

clean_msg: 
	$(MAKE) -C msg clean

.c.$(OBJEXT):
	$(COMPILE.c) $(CFLAGS) $(HBP_INCLUDES) $(CCOUT)$@ $<

install: build
	cp -f bin/$(OUT)  $(ALTI_HOME)/aheartbeat/bin/
	cp -f bin/altibaseFailureEvent.sh    $(ALTI_HOME)/aheartbeat/bin/
	cp -f bin/altibaseFailureEvent.bat   $(ALTI_HOME)/aheartbeat/bin/
	cp -f bin/remoteNodeFailureEvent.sh  $(ALTI_HOME)/aheartbeat/bin/
	cp -f bin/remoteNodeFailureEvent.bat $(ALTI_HOME)/aheartbeat/bin/
	cp -f conf/aheartbeat.settings       $(ALTI_HOME)/aheartbeat/conf/
	cp -f conf/.altibaseConnection.info  $(ALTI_HOME)/aheartbeat/conf/
	cp -f msg/msgEN100.mdl    $(ALTI_HOME)/aheartbeat/msg/
	cp -f msg/aheartbeat.ban  $(ALTI_HOME)/aheartbeat/msg/

-include $(SRCS:.c=.d)
