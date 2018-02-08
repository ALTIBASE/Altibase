include ../../utEnv.mk
include ../checkServer.mk

CHKSERV_SRCS = $(CHKSERV_DIR)/src/checkServer.$(CHKSERV_EXT)
KILLCHK_SRCS = $(CHKSERV_DIR)/src/killCheckServer.$(CHKSERV_EXT)
CHKSERV_OBJS = $(CHKSERV_SRCS:$(DEV_DIR)/%.$(CHKSERV_EXT)=$(TARGET_DIR)/%.$(OBJEXT))
KILLCHK_OBJS = $(KILLCHK_SRCS:$(DEV_DIR)/%.$(CHKSERV_EXT)=$(TARGET_DIR)/%.$(OBJEXT))
OBJS = $(CHKSERV_OBJS) $(KILLCHK_OBJS)
PGMS = checkServer killCheckServer
BINS = $(foreach i,$(PGMS),$(ALTI_HOME)/bin/$i$(BINEXT))

CHKSERV_DEF_LIBS = $(foreach i,$(CHKSERV_LIB_NAME),$(LIBOPT)$(i)$(LIBAFT)) $(LIBS)

cmd_ld = $(LD) $(LFLAGS) $(LDOUT)$(ALTI_HOME)/bin/$@$(BINEXT) $^ $(CHKSERV_DEF_LIBS)



all : build

build : compile

compile : $(PGMS)

clean :
	$(RM) core .dependency $(BINS) $(TARGET_DIR)/ut/checkServer/src *.d SunWS_cache



checkServer : $(CHKSERV_OBJS)
	$(cmd_ld)

killCheckServer : $(KILLCHK_OBJS)
	$(cmd_ld)

-include $(OBJS:.$(OBJEXT)=.d)
