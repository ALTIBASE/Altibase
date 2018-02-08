include $(ALTIBASE_HOME)/install/altibase_env.mk

BINS=insertObject$(BINEXT) selectObject$(BINEXT) updateObject$(BINEXT) deleteObject$(BINEXT) 
SRCS=insertObject.cpp selectObject.cpp updateObject.cpp deleteObject.cpp
OBJS=$(SRCS:.cpp=.$(OBJEXT) )

all : compile

compile: $(BINS) 

insertObject$(BINEXT) : insertObject.$(OBJEXT)  insertObject.cpp
	$(LD) $(LFLAGS) $(LDOUT)$@ insertObject.$(OBJEXT) $(LIBOPT)odbccli$(LIBAFT) $(LIBOPT)acs$(LIBAFT) $(LIBS) 

selectObject$(BINEXT) : selectObject.$(OBJEXT)  selectObject.cpp 
	$(LD) $(LFLAGS) $(LDOUT)$@ selectObject.$(OBJEXT) $(LIBOPT)odbccli$(LIBAFT) $(LIBOPT)acs$(LIBAFT) $(LIBS) 

updateObject$(BINEXT) : updateObject.$(OBJEXT)  updateObject.cpp
	$(LD) $(LFLAGS) $(LDOUT)$@ updateObject.$(OBJEXT) $(LIBOPT)odbccli$(LIBAFT) $(LIBOPT)acs$(LIBAFT) $(LIBS) 

deleteObject$(BINEXT) : deleteObject.$(OBJEXT)  deleteObject.cpp
	$(LD) $(LFLAGS) $(LDOUT)$@ deleteObject.$(OBJEXT) $(LIBOPT)odbccli$(LIBAFT) $(LIBOPT)acs$(LIBAFT) $(LIBS) 

clean:
	$(Q) $(RM) $(BINS) *.$(OBJEXT) core *.class *.exe
