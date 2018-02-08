include $(ALTIBASE_HOME)/install/altibase_env.mk

# for static library
CDBC_LIBS = $(foreach i, alticapi odbccli, $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)
# for shared library
CDBC_SHLIBS = $(foreach i, alticapi_sl, $(LIBOPT)$(i)$(LIBAFT)) $(LIBS)



#	    demo_meta3 demo_meta4 demo_meta5 \
#	    demo_meta6 demo_meta7 demo_meta9 demo_meta10 \
#	    demo_info1 demo_info2 \

DEMONAMES = demo_ex1 demo_ex2 demo_ex3 demo_ex4 demo_ex5 demo_ex6 demo_ex7 \
            demo_meta1 demo_meta2 demo_meta8 \
            demo_tran1 demo_tran2 demo_mt \
            demo_rtl \
            demo_clob demo_blob
OBJS = $(foreach i, $(DEMONAMES), $(i).$(OBJEXT))
BINS = $(foreach i, $(DEMONAMES), $(i)$(BINEXT))



all : build

build : compile

compile : $(DEMONAMES)

clean :
	$(RM) core SunWS_cache $(OBJS) $(BINS) *.ilk *.pdb *.manifest *.d



demo_ex1 : demo_ex1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_ex2 : demo_ex2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_ex3 : demo_ex3.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_ex4 : demo_ex4.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_ex5 : demo_ex5.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_ex6 : demo_ex6.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_ex7 : demo_ex7.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta1 : demo_meta1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta2 : demo_meta2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta3 : demo_meta3.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta4 : demo_meta4.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta5 : demo_meta5.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta6 : demo_meta6.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta7 : demo_meta7.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta8 : demo_meta8.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta9 : demo_meta9.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_meta10 : demo_meta10.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_tran1 : demo_tran1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_tran2 : demo_tran2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_mt : demo_mt.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS) $(PTHREAD_LIB)

demo_info1 : demo_info1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_info2 : demo_info2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_rtl : demo_ex2.$(OBJEXT)
	$(LD) $(RTL_FLAG) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_SHLIBS)

demo_clob : demo_clob.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

demo_blob : demo_blob.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)$@$(BINEXT) $^ $(CDBC_LIBS)

