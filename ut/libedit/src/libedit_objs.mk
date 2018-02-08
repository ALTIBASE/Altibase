# $Id: libedit_objs.mk 26403 2008-06-08 06:16:43Z jdlee $
#

LIBEDIT_SRCS = \
$(UT_DIR)/libedit/src/chared.c \
$(UT_DIR)/libedit/src/common.c \
$(UT_DIR)/libedit/src/el.c \
$(UT_DIR)/libedit/src/emacs.c \
$(UT_DIR)/libedit/src/fcns.c \
$(UT_DIR)/libedit/src/fgetln.c \
$(UT_DIR)/libedit/src/help.c \
$(UT_DIR)/libedit/src/hist.c \
$(UT_DIR)/libedit/src/history.c \
$(UT_DIR)/libedit/src/key.c \
$(UT_DIR)/libedit/src/map.c \
$(UT_DIR)/libedit/src/parse.c \
$(UT_DIR)/libedit/src/prompt.c \
$(UT_DIR)/libedit/src/read.c \
$(UT_DIR)/libedit/src/refresh.c \
$(UT_DIR)/libedit/src/search.c \
$(UT_DIR)/libedit/src/sig.c \
$(UT_DIR)/libedit/src/strlcat.c \
$(UT_DIR)/libedit/src/strlcpy.c \
$(UT_DIR)/libedit/src/term.c \
$(UT_DIR)/libedit/src/tokenizer.c \
$(UT_DIR)/libedit/src/tty.c \
$(UT_DIR)/libedit/src/unvis.c \
$(UT_DIR)/libedit/src/vi.c \
$(UT_DIR)/libedit/src/vis.c \
$(UT_DIR)/libedit/src/local_ctype.c

LIBEDIT_OBJS = $(LIBEDIT_SRCS:$(DEV_DIR)/%.c=$(TARGET_DIR)/%.$(OBJEXT))
