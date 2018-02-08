LIBS_JNI = -ljdbccli_sl
SH_LIBS  = -ldl -lpthread -lrt

LIBS_JNI += $(SH_LIBS)

JAVA_INCLUDE = $(JAVA_HOME)/include

INCLUDES += -I$(JAVA_HOME)/include/linux

JAVAC    = $(JAVA_HOME)/bin/javac
JAVA     = $(JAVA_HOME)/bin/java
JAR      = $(JAVA_HOME)/bin/jar
JAVALIB  = $(JAVA_HOME)/lib

COMMON_ADAPTER_SRCS = oaMain.c \
                      oaConfig.c \
                      oaLog.c \
                      oaAlaReceiver.c \
                      oaAlaLogConverter.c \
                      oaApplierInterface.c \
                      oaPerformance.c \
                      oaLogRecord.c

ALTIBASE_ADAPTER_SRCS = $(COMMON_ADAPTER_SRCS) oaAltiApplier.c

JDBC_ADAPTER_SRCS = $(COMMON_ADAPTER_SRCS) oaJDBCApplier.c \
                                           oaJNIInterface.c

ORA_ADAPTER_SRCS = $(COMMON_ADAPTER_SRCS) oaOciApplier.c

ALTIBASE_ADAPTER_OBJS = $(ALTIBASE_ADAPTER_SRCS:.c=.$(OBJEXT)) msg/oaMsg.$(OBJEXT)

JDBC_ADAPTER_OBJS = $(JDBC_ADAPTER_SRCS:.c=.$(OBJEXT)) msg/oaMsg.$(OBJEXT)

ORA_ADAPTER_OBJS = $(ORA_ADAPTER_SRCS:.c=.$(OBJEXT)) msg/oaMsg.$(OBJEXT)


