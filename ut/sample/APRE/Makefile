include $(ALTIBASE_HOME)/install/altibase_env.mk

BINS=connect1 connect2 free select insert update delete                  \
	 cursor1 cursor2 dynamic1 dynamic2 dynamic3 dynamic4 psm1 psm2       \
	 date varchar binary arrays1 arrays2 argument indicator              \
	 runtime_error_check whenever mc1 mc2 mc3 mt1 mt2 pointer declare_stmt     
BINS2=cparsefull macro
SRCS=connect1.sc connect2.sc free.sc                                       \
	 select.sc insert.sc update.sc delete.sc                               \
	 cursor1.sc cursor2.sc dynamic1.sc dynamic2.sc dynamic3.sc dynamic4.sc \
	 psm1.sc psm2.sc date.sc varchar.sc binary.sc                          \
	 arrays1.sc arrays2.sc argument.sc indicator.sc                        \
	 runtime_error_check.sc whenever1.sc whenever2.sc                      \
	 mc1.sc mc2.sc mc3.sc mt1.sc mt2.sc pointer.sc declare_stmt.sc         \
	 cparsefull.sc macro.sc

APRES=$(SRCS:.sc=.cpp)
OBJS=$(SRCS:.cpp=.$(OBJEXT))

%.cpp : %.sc
	apre -t cpp $^

all: $(BINS) parsefull

connect1: connect1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

connect2: connect2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

free: free.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

select: select.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

insert: insert.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

update: update.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

delete: delete.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

cursor1: cursor1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

cursor2: cursor2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dynamic1: dynamic1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dynamic2: dynamic2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dynamic3: dynamic3.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dynamic4: dynamic4.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

psm1: psm1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

psm2: psm2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

date: date.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

varchar: varchar.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

binary: binary.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

arrays1: arrays1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

arrays2: arrays2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

argument: argument.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

indicator: indicator.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

runtime_error_check: runtime_error_check.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

whenever: whenever1.$(OBJEXT) whenever2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

mc1: mc1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

mc2: mc2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

mc3: mc3.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

mt1: mt1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

mt2: mt2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

pointer: pointer.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

declare_stmt: declare_stmt.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)apre$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

parsefull:
	make -f Makefile_cparse

clean:
	$(Q) $(RM) $(BINS) $(BINS2) $(APRES) *.$(OBJEXT) core *.d

