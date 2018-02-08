include $(ALTIBASE_HOME)/install/altibase_env.mk

SRCS=
OBJS=$(SRCS:.cpp=.$(OBJEXT))

BINS=formout dataout datain dataout_cb datain_cb \
	 formout_mt datain_mt dataout_mt \
	 dataout_cb_1 datain_cb_1 dataout_cb_2 datain_cb_2
	 
all: $(BINS)

formout: formout.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)formout$(BINEXT) formout.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dataout: dataout.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)dataout$(BINEXT) dataout.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

datain: datain.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)datain$(BINEXT) datain.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dataout_cb: dataout_cb.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)dataout_cb$(BINEXT) dataout_cb.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

datain_cb: datain_cb.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)datain_cb$(BINEXT) datain_cb.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dataout_cb_1: dataout_cb_1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)dataout_cb_1$(BINEXT) dataout_cb_1.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

datain_cb_1: datain_cb_1.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)datain_cb_1$(BINEXT) datain_cb_1.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dataout_cb_2: dataout_cb_2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)dataout_cb_2$(BINEXT) dataout_cb_2.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

datain_cb_2: datain_cb_2.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)datain_cb_2$(BINEXT) datain_cb_2.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

formout_mt: formout_mt.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)formout_mt$(BINEXT) formout_mt.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT)  $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

datain_mt: datain_mt.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)datain_mt$(BINEXT) datain_mt.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT)  $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

dataout_mt: dataout_mt.$(OBJEXT)
	$(LD) $(LFLAGS) $(LDOUT)dataout_mt$(BINEXT) dataout_mt.$(OBJEXT) $(LIBOPT)iloader$(LIBAFT) $(LIBOPT)odbccli$(LIBAFT) $(LIBS)

clean:
	-$(RM) $(BINS) *.$(OBJEXT) core *.exe


