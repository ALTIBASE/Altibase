
SECTION FILE;

PRODUCT_ID   = 82;
PRODUCT_NAME = "adapter";
PRODUCT_ABBR = "OAM";

LANGUAGES    = EN;

SECTION ERROR;

{
    ERR_ID = 1;
    KEY    = OAM_ERR_TEST;
    MSG_EN = "msg test error %1$s";
}

SECTION LOG;

{
    KEY    = OAM_MSG_TEST;
    MSG_EN = "msg test message %1$d";
}

{
    KEY    = OAM_MSG_TEST2;
    MSG_EN = "msg test message %1$s";
}

{
    KEY = OAM_MSG_ADAPTER_INITIALIZED;
    MSG_EN = "Adapter is initialized.";
}

{
    KEY = OAM_MSG_ADAPTER_FINALIZED;
    MSG_EN = "Adapter is finalized.";
}

{
    KEY = OAM_MSG_ADAPTER_READY;
    MSG_EN = "Adapter is ready to process logs.";
}

{
    KEY = OAM_MSG_DUMP_LOG;
    MSG_EN = "%1$s";
}

{
    KEY = OAM_MSG_STATISTICS_TICKS;
    MSG_EN = "Ticks : %1$s [%2$llu]";
}

/*
 * TODO: Following error messages have to be located SECTION ERROR. But now
 * some codes about error callback are need to do it.
 */

{
    KEY = OAM_ERR_ALA_LIBRARY;
    MSG_EN = "ALA library error: %1$s";
}

{
    KEY = OAM_ERR_LIBRARY;
    MSG_EN = "%1$s library error: %2$d %3$s";
}

{
    KEY = OAM_ERR_GET_ADAPTER_HOME_DIRECTORY;
    MSG_EN = "Getting adapter home directory is failed.";
}

{
    KEY    = OAM_ERR_MEM_CALLOC;
    MSG_EN = "Error: memory allocation is failed.";
}

{
    KEY = OAM_ERR_ALA_CLEAR_ERROR_MGR;
    MSG_EN = "ALA library error: ALA_ClearErrorMgr() failed." ;
}

{
    KEY = OAM_ERR_TABLE_INFO_NOT_EXIST;
    MSG_EN = "ALA Log converter error: Table information is not exist.";
}

{
    KEY = OAM_ERR_COLUMN_NOT_EXIST;
    MSG_EN = "ALA Log converter error: Column is not exist.";
}

{
    KEY = OAM_ERR_UNSUPPORTED_DATA_TYPE;
    MSG_EN = "ALA Log converter error: Unsupported data type.";
}

{
    KEY = OAM_ERR_UNSUPPORTED_LOG_TYPE;
    MSG_EN = "ALA Log converter error: Unsupported log type.";
}

{
    KEY = OAM_ERR_INITIALIZE;
    MSG_EN = "%1$s library error: Initialize() failed.";
}

{
    KEY = OAM_ERR_INITIALIZE_ENV;
    MSG_EN = "%1$s library error: EnvInit() failed.";
}

{
    KEY = OAM_ERR_CREATE_ENV;
    MSG_EN = "%1$s library error: EnvCreate() failed.";
}

{
    KEY = OAM_ERR_ALLOC_SERVICE_HANDLE;
    MSG_EN = "%1$s library error: Allocation service handle failed.";
}

{
    KEY = OAM_ERR_ALLOC_ERROR_HANDLE;
    MSG_EN = "%1$s library error: Allocation error handle failed.";
}

{
    KEY = OAM_ERR_APPLY_ERROR;
    MSG_EN = "Errors have incurred %1$d times when applying data from %2$s to Altibase.";
}

{
    KEY = OAM_ERR_RESTART_ERROR;
    MSG_EN = "Adaptor had restarted %1$d times due to the error occurred, but it failed.";
}

{
    KEY = OAM_MSG_JVM_NOT_CREATE;
    MSG_EN = "Failed to create the JAVA VM in JNI";
}

{
    KEY = OAM_MSG_NOT_SUPPORT_CHARSET;
    MSG_EN = "%1$s character set is not supported in JDBC adapter.";
}

{
    KEY = OAM_MSG_JNI_EXCEPTION;
    MSG_EN = "A JNI exception occurred in %1$s function.";
}

{
    KEY = OAM_MSG_NOT_FOUND_CLASS;
    MSG_EN = "%1$s class does not exist in the JAVA library.";
}

{
    KEY = OAM_MSG_NOT_FOUND_METHOD;
    MSG_EN = "%1$s method does not exist in the JAVA library.";
}

{
    KEY = OAM_ERR_LOGIN_FILE_OPEN;
    MSG_EN = "Opening oalogin.sql file failed: %1$s";
}

{
    KEY = OAM_ERR_LOGIN_FILE_READ;
    MSG_EN = "Reading oalogin.sql file failed: %1$s";
}

{
    KEY = OAM_ERR_LOGIN_SQL_MAX_SIZE;
    MSG_EN = "The SQL statement used in the oalogin.sql file exceeded 512KBytes.";
}

{
    KEY = OAM_ERR_OCI_ERR_SQL;
    MSG_EN = "OCI error sql: %1$s";
}
