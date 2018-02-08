/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body utl_tcp is

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
                         RETURN CONNECT_TYPE
as
    ret CONNECT_TYPE;
begin
    ret := OPEN_CONNECT( remote_host, remote_port, NULL, out_buffer_size );
    return ret;
end;

FUNCTION write_raw(c    IN CONNECT_TYPE ,
data IN            RAW(65534),
len  IN            INTEGER DEFAULT NULL)
RETURN INTEGER
as
begin
    return write_raw ( c, data, len );
end;

FUNCTION is_connect( c IN CONNECT_TYPE) RETURN INTEGER
as
begin
    return is_connected( c );
end;

PROCEDURE close_connection(c IN CONNECT_TYPE)
as
    dummy integer;
begin
    dummy := close_connect(c);
end;

PROCEDURE close_all_connections
as
    dummy integer;
begin
    dummy := closeall_connect();
end;

end;
/

