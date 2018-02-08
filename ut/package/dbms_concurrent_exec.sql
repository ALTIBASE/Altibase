/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE DBMS_CONCURRENT_EXEC AUTHID CURRENT_USER AS

FUNCTION INITIALIZE ( IN_DEGREE INTEGER DEFAULT NULL ) RETURN INTEGER;

FUNCTION GET_DEGREE_OF_CONCURRENCY                     RETURN INTEGER;

FUNCTION REQUEST    ( TEXT VARCHAR(8192) )             RETURN INTEGER;

FUNCTION WAIT_ALL                                      RETURN INTEGER;

FUNCTION WAIT_REQ   ( REQ_ID INTEGER )                 RETURN INTEGER;

FUNCTION FINALIZE                                      RETURN INTEGER;

FUNCTION GET_ERROR_COUNT                               RETURN INTEGER;

FUNCTION GET_ERROR  ( REQ_ID IN INTEGER,
                      TEXT OUT VARCHAR(8192),
                      ERR_CODE OUT INTEGER,
                      ERR_MSG OUT VARCHAR(8192) )      RETURN INTEGER;

FUNCTION GET_LAST_REQ_ID                               RETURN INTEGER;

FUNCTION GET_REQ_TEXT ( REQ_ID INTEGER )               RETURN VARCHAR(8192);

PROCEDURE PRINT_ERROR ( REQ_ID INTEGER );

END;
/

CREATE OR REPLACE PUBLIC SYNONYM DBMS_CONCURRENT_EXEC FOR SYS.DBMS_CONCURRENT_EXEC;
GRANT EXECUTE ON DBMS_CONCURRENT_EXEC TO PUBLIC;
