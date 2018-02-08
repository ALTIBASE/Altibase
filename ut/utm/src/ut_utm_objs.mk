# $Id: ut_utm_objs.mk 78292 2016-12-15 05:50:02Z bethy $
#

UT_UTM_SRCS = $(UT_DIR)/utm/src/aexport.cpp \
              $(UT_DIR)/utm/src/utmProgOption.cpp \
              $(UT_DIR)/utm/src/utmPropertyMgr.cpp \
              $(UT_DIR)/utm/src/utmPassword.cpp \
              $(UT_DIR)/utm/src/utmManager.cpp \
              $(UT_DIR)/utm/src/utmTable.cpp \
              $(UT_DIR)/utm/src/utmMisc.cpp \
              $(UT_DIR)/utm/src/utmSQLApi.cpp \
              $(UT_DIR)/utm/src/utmPartition.cpp \
              $(UT_DIR)/utm/src/utmObjMode.cpp \
              $(UT_DIR)/utm/src/utmSeqSym.cpp \
              $(UT_DIR)/utm/src/utmQueue.cpp \
              $(UT_DIR)/utm/src/utmReplDbLink.cpp \
              $(UT_DIR)/utm/src/utmConstr.cpp \
              $(UT_DIR)/utm/src/utmObjPriv.cpp \
              $(UT_DIR)/utm/src/utmUser.cpp \
              $(UT_DIR)/utm/src/utmTbs.cpp \
              $(UT_DIR)/utm/src/utmViewProc.cpp \
              $(UT_DIR)/utm/src/utmDbStats.cpp \
              $(UT_DIR)/utm/src/utmScript4Ilo.cpp


UT_UTM_OBJS = $(UT_UTM_SRCS:$(DEV_DIR)/%.cpp=$(TARGET_DIR)/%.$(OBJEXT))

