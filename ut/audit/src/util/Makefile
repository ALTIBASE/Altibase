
include ../../../utEnv.mk

AUDIT_DIR=$(UT_DIR)/audit

INCLUDES+= $(foreach i, ../include $(ORACLE_HOME)/rdbms/demo $(ORACLE_HOME)/rdbms/public, $(IDROPT)$(i))
LIBDIRS += $(foreach i, ../atc ../ata ../ato $(ALTI_HOME)/lib, $(LDROPT)$(i))
LIB_OPT	+= $(foreach i,odbccli id pd atDb, $(LIBOPT)$(i)$(LIBAFT)) $(ID_LIBS)

DIRS = ../atc ../ata ../ato

#ifneq "$(OS_TARGET)" "INTEL_WINDOWS"
#ifeq "$(compile64)" "0"
## LIB_OPT+= -L$(ORACLE_HOME)/lib32 -L$(ORACLE_HOME)/lib -lclntsh `cat $(ORACLE_HOME)/lib/sysliblist`
# else
## LIB_OPT+= -L$(ORACLE_HOME)/lib64 -L$(ORACLE_HOME)/lib -lclntsh `cat $(ORACLE_HOME)/lib/sysliblist`
#endif
#endif # INTEL_WINDOWS



all: schema
	@$(RM) core*

.PHONY: $(DIRS)
$(DIRS):
	@$(MAKE) -C $@ $(SUBDIR_TARGET)


OBJS=$(SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

compile :
lib:
	$(MAKE) $(S) $(DIRS) SUBDIR_TARGET=compile

clean:
	@$(RM) core* *.d SunWS_cache *.o schema ../log/* *.log
	
schema: schema.o lib
	$(LD) $(LFLAGS) -latOra $(SOLFLAGS) $(LDOUT)$@ schema.o $(LIB_OPT)

cdemodt: cdemodt.o 
	$(LD) $(LFLAGS) $(SOLFLAGS) $(LDOUT)$@ cdemodt.o $(LIB_OPT)
	

ctags:
	ctags -R . ../include

check:
	@echo EXTRA_CXXOPT= $(EXTRA_CXXOPT)

-include $(OBJS:.$(OBJEXT)=.d)
