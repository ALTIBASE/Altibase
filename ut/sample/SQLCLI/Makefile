include $(ALTIBASE_HOME)/install/altibase_env.mk

TARGETS = demo_ex1 demo_ex2 demo_ex3 demo_ex4 demo_ex5 demo_ex6 demo_ex7 demo_ex8\
		  demo_meta1 demo_meta2 demo_meta3 demo_meta4 \
		  demo_meta5 demo_meta6 demo_meta7 demo_meta8 \
		  demo_meta9 demo_meta10 \
		  demo_tran1 demo_tran2 demo_mt demo_info1 demo_info2 \
		  demo_sl demo_plan demo_cpool
BINS = $(addsuffix $(BINEXT),$(TARGETS))

all: $(TARGETS)

demo_ex1: demo_ex1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_ex2: demo_ex2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_ex3: demo_ex3.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_ex4: demo_ex4.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_ex5: demo_ex5.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_ex6: demo_ex6.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_ex7: demo_ex7.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_ex8: demo_ex8.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta1: demo_meta1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta2: demo_meta2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta3: demo_meta3.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta4: demo_meta4.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta5: demo_meta5.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta6: demo_meta6.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta7: demo_meta7.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta8: demo_meta8.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta9: demo_meta9.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_meta10: demo_meta10.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_tran1: demo_tran1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_tran2: demo_tran2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_dead: demo_dead.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_mt: demo_mt.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_info1: demo_info1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_info2: demo_info2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_plan: demo_plan.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

demo_sl: demo_ex2.$(OBJEXT)
	$(LD) $(RTL_FLAG) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli_sl$(LIBAFT) $(SH_LIBS)

demo_cpool: demo_cpool.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	-$(RM) $(BINS) *.$(OBJEXT) core *.exe *.d

