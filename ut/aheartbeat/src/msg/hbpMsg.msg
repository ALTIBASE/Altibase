
SECTION FILE;

PRODUCT_ID   = 100;
PRODUCT_NAME = "aheartbeat";
PRODUCT_ABBR = "HBM";

LANGUAGES    = EN;

SECTION ERROR;

{
    ERR_ID = 1;
    KEY    = HBM_ERR_TEST;
    MSG_EN = "msg test error %1$s";
}

SECTION LOG;

{
    KEY    = HBM_MSG_FOR_BANNER;
    MSG_EN = "%1$s";
}

{
    KEY    = HBM_MSG_STATE_CHANGE;
    MSG_EN = "ID : %1$d, STATE : %2$s -> %3$s.";
}

{
    KEY    = HBM_MSG_SUSPEND_RECEIVED;
    MSG_EN = "SUSPEND message is arrived from ID : %1$d.";
}

{
    KEY    = HBM_MSG_STOP_RECEIVED;
    MSG_EN = "Received STOP message.";
}

{
    KEY = HBM_MSG_HBP_INITIALIZED;
    MSG_EN = "aheartbeat is initialized. Listener Thread T-%1$ld.";
}

{
    KEY = HBM_MSG_HBP_LISTENER_INITIALIZED;
    MSG_EN = "aheartbeat Listener is initialized.";
}

{
    KEY = HBM_MSG_HBP_FINALIZED;
    MSG_EN = "aheartbeat is finalized.";
}

{
    KEY = HBM_MSG_HBP_DETECT_SERVER_ERROR;
    MSG_EN = "aheartbeat cannot execute select statement on any IP addresses of server %1$d.";
}

{
	KEY = HBM_MSG_HBP_CANNOT_CONNECT_TO_ALTIBASE_AT_BEGIN_ERROR;
	MSG_EN = "[ERROR] aheartbeat cannot connect to altibase server.";
}
{
    KEY = HBM_MSG_HBP_EXCEED_HIGHWATER_ERROR;
    MSG_EN = "aheartbeat has detected server %1$d failure.";
}

{
    KEY = HBM_MSG_PREPARE_SQL_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to prepare sql.";
}

{
    KEY = HBM_MSG_BIND_COL_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to bind column.";
}

{
    KEY = HBM_MSG_EXE_SQL_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to execute sql.";
}

{
    KEY = HBM_MSG_FETCH_SQL_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to fetch sql.";
}

{
    KEY = HBM_MSG_CLOSE_CURSOR_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to close cursor.";
}

{
    KEY = HBM_MSG_CONNECT_DRIVER_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to execute SQLDriverConnect.";
}

{
    KEY = HBM_MSG_FAILOVER1_EXECUTE;
    MSG_EN = "aheartbeat is running failover for altibase failure. My ID : %1$d.";
}

{
    KEY = HBM_MSG_FAILOVER2_EXECUTE;
    MSG_EN = "aheartbeat is running failover for remote node failure. ID : %1$d.";
}

{
    KEY = HBM_MSG_FAILOVER1_EXECUTE_ERROR;
    MSG_EN = "[ERROR] Failed to run failover for altibase failure. My ID : %1$d.";
}

{
    KEY = HBM_MSG_FAILOVER2_EXECUTE_ERROR;
    MSG_EN = "[ERROR] Failed to run failover for remote node failure. ID : %1$d.";
}

{
    KEY = HBM_MSG_HBP_DETECT_ONE_SERVER_ERROR;
    MSG_EN = "[ERROR] aheartbeat cannot execute select statement on server ID: %1$d, IP : %2$s.";
}

{
    KEY    = HBM_MSG_ALLOC_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to allocate memory.";
}

{
    KEY    = HBM_MSG_ALLOC_ENV_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to allocate environment handle.";
}

{
    KEY    = HBM_MSG_ALLOC_CONNECT_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to allocate connection.";
}

{
    KEY    = HBM_MSG_ALLOC_STMT_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to allocate statement.";
}

{
    KEY    = HBM_MSG_CONNECT_DB_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to connect DB on IP = %2$s.";
}

{
    KEY    = HBM_MSG_ALLOC_DB_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to allocate DB Info.";
}

{
    KEY    = HBM_MSG_RECV_ACK_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Ack message is invalid : %2$d.";
}

{
    KEY    = HBM_MSG_HANDSHAKE_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to handshake with ID : %2$d.";
}

{
    KEY    = HBM_MSG_SOCK_OPEN_WITH_IP_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to open socket with IP : %2$s Port : %3$d.";
}

{
    KEY    = HBM_MSG_SOCK_CONNECT_WITH_IP_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to establish a connection to IP : %2$s Port : %3$d.";
}

{
    KEY    = HBM_MSG_STR_TO_ADDR_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to excute InetStrToAddr function. IP : %2$s.";
}

{
    KEY    = HBM_MSG_BIND_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Socket Bind Error.";
}

{
    KEY    = HBM_MSG_SETOPT_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to execute setopt function.";
}

{
    KEY    = HBM_MSG_GET_ADDR_INFO_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to get address info.";
}

{
    KEY    = HBM_MSG_LISTEN_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to call socket listen function.";
}
{
    KEY    = HBM_MSG_BLOCK_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to change socket block mode.";
}
{
    KEY    = HBM_MSG_ACCEPT_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to accept a new connection on socket.";
}
{
    KEY    = HBM_MSG_SOCK_CLOSE_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to close socket.";
}
{
    KEY    = HBM_MSG_SEND_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to send data.";
}

{
    KEY    = HBM_MSG_RECV_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to receive data.";
}

{
    KEY    = HBM_MSG_SOCK_OPEN_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to open & connect socket.";
}

{
    KEY    = HBM_MSG_THREAD_CREATE_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to create thread.";
}

{
    KEY    = HBM_MSG_MUTEX_CREATE_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to create mutex.";
}

{
    KEY    = HBM_MSG_MUTEX_LOCK_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to lock mutex.";
}

{
    KEY    = HBM_MSG_MUTEX_UNLOCK_ERROR;
    MSG_EN = "[ERROR] [%1$s] : Failed to unlock mutex.";
}

{
	KEY    = HBM_MSG_UNDEFINED_PROTOCOL;
	MSG_EN = "[ERROR] Undefined protocol: %1$d.";
}

{
	KEY    = HBM_MSG_EXCEPTIONAL_CASE;
	MSG_EN = "[ERROR] [%1$s] Unexpected action has been detected.";
}
