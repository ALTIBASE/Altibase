/***********************************************************************
 * Copyright 1999-2017, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE utl_smtp AUTHID CURRENT_USER AS

FUNCTION open_connection( host       IN VARCHAR(64),
                          port       IN INTEGER,
                          tx_timeout IN INTEGER DEFAULT NULL )
RETURN CONNECT_TYPE;

FUNCTION helo( c       IN CONNECT_TYPE,
               domain  IN VARCHAR(64) )
RETURN VARCHAR(512);

FUNCTION mail( c         IN CONNECT_TYPE,
               sender    IN VARCHAR(256),
               paramters IN VARCHAR DEFAULT NULL )
RETURN VARCHAR(512);

FUNCTION rcpt( c         IN CONNECT_TYPE,
               recipient IN VARCHAR(256),
               paramters IN VARCHAR DEFAULT NULL )
RETURN VARCHAR(512);

FUNCTION open_data( c IN CONNECT_TYPE )
RETURN VARCHAR(512);

PROCEDURE write_data( c    IN CONNECT_TYPE,
                      data IN VARCHAR(65534) );

PROCEDURE write_raw_data( c    IN CONNECT_TYPE,
                          data IN RAW(65534) );

FUNCTION close_data( c IN CONNECT_TYPE )
RETURN VARCHAR(512);

FUNCTION quit( c IN CONNECT_TYPE )
RETURN VARCHAR(512);
END;
/

CREATE OR REPLACE PUBLIC SYNONYM utl_smtp FOR sys.utl_smtp;

