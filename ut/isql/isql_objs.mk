# Makefile for isql DBC, TYPE library
#
# SVN Info : $Id$
#

# utISPApi srcs
ISQL_DBC_SRCS   =   $(UT_DIR)/isql/dbc/isqlColumns.cpp            \
                    $(UT_DIR)/isql/dbc/isqlFloat.cpp              \
                    $(UT_DIR)/isql/dbc/dbcAdmin.cpp               \
                    $(UT_DIR)/isql/dbc/dbcAttr.cpp                \
                    $(UT_DIR)/isql/dbc/dbcBind.cpp                \
                    $(UT_DIR)/isql/dbc/dbcConnection.cpp          \
                    $(UT_DIR)/isql/dbc/dbcDiagnostic.cpp          \
                    $(UT_DIR)/isql/dbc/dbcFetch.cpp               \
                    $(UT_DIR)/isql/dbc/dbcMeta.cpp                \
                    $(UT_DIR)/isql/dbc/dbcStmt.cpp                \
                    $(UT_DIR)/isql/dbc/dbcTransaction.cpp

ISQL_TYPE_SRCS  =   $(UT_DIR)/isql/type/isqlType.cpp              \
                    $(UT_DIR)/isql/type/isqlBit.cpp               \
                    $(UT_DIR)/isql/type/isqlBlob.cpp              \
                    $(UT_DIR)/isql/type/isqlBytes.cpp             \
                    $(UT_DIR)/isql/type/isqlChar.cpp              \
                    $(UT_DIR)/isql/type/isqlClob.cpp              \
                    $(UT_DIR)/isql/type/isqlDate.cpp              \
                    $(UT_DIR)/isql/type/isqlDouble.cpp            \
                    $(UT_DIR)/isql/type/isqlInteger.cpp           \
                    $(UT_DIR)/isql/type/isqlLong.cpp              \
                    $(UT_DIR)/isql/type/isqlNull.cpp              \
                    $(UT_DIR)/isql/type/isqlNumeric.cpp           \
                    $(UT_DIR)/isql/type/isqlReal.cpp              \
                    $(UT_DIR)/isql/type/isqlVarchar.cpp
