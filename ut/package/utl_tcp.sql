/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE utl_tcp AUTHID CURRENT_USER AS

CRLF CONSTANT VARCHAR(2) := chr(13)||chr(10);

FUNCTION open_connection(remote_host     IN VARCHAR(64),
remote_port     IN INTEGER,
local_host      IN VARCHAR(64)    DEFAULT NULL,
local_port      IN INTEGER DEFAULT NULL,
in_buffer_size  IN INTEGER DEFAULT NULL,
out_buffer_size IN INTEGER DEFAULT NULL,
charset         IN VARCHAR(16)    DEFAULT NULL,
newline         IN VARCHAR(2)    DEFAULT CRLF,
tx_timeout      IN INTEGER DEFAULT NULL,
wallet_path     IN VARCHAR(256)    DEFAULT NULL,
wallet_password IN VARCHAR    DEFAULT NULL)
RETURN CONNECT_TYPE;

FUNCTION write_raw(c    IN CONNECT_TYPE ,
data IN            RAW(65534),
len  IN            INTEGER DEFAULT NULL)
RETURN INTEGER;


FUNCTION is_connect( c IN CONNECT_TYPE) RETURN INTEGER;


PROCEDURE close_connection(c IN CONNECT_TYPE);

PROCEDURE close_all_connections;

END;
/

CREATE OR REPLACE PUBLIC SYNONYM utl_tcp FOR sys.utl_tcp;
GRANT EXECUTE ON sys.utl_tcp TO PUBLIC;

