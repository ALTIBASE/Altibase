/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE FUNCTION SYS_CONTEXT( NAMESPACE IN VARCHAR(30),
                                        PARAMETER IN VARCHAR(30) )
RETURN VARCHAR(256) AUTHID CURRENT_USER
AS
    RET_VAL         VARCHAR(256);
    UP_NAMESPACE    VARCHAR(30);
    UP_PARAMETER    VARCHAR(30);
    
BEGIN
    UP_NAMESPACE := UPPER(NAMESPACE);
    
    IF UP_NAMESPACE = 'USERENV' THEN
       
        UP_PARAMETER := UPPER(PARAMETER);

        IF UP_PARAMETER = 'CLIENT_INFO' THEN
            SELECT CLIENT_INFO INTO RET_VAL FROM V$SESSION where ID = SESSION_ID();

        ELSEIF UP_PARAMETER = 'IP_ADDRESS' THEN
            select substr(comm_name,5,instr(comm_name,':')-5) into RET_VAL from v$session where id = session_id();
        ELSEIF UP_PARAMETER = 'ISDBA' THEN
            select DECODE(SYSDBA_FLAG,1,'TRUE',0,'FALSE') into RET_VAL from v$session where id = session_id();
        ELSEIF UP_PARAMETER = 'LANGUAGE' THEN
            select nls_territory||'.'||NLS_CHARACTERSET into RET_VAL from v$session, v$nls_parameters where session_id = id and session_id = session_id();

        ELSEIF UP_PARAMETER = 'NLS_CURRENCY' THEN
            select nls_currency into RET_VAL from v$session where id = session_id();

        ELSEIF UP_PARAMETER = 'NLS_DATE_FORMAT' THEN
            select DEFAULT_DATE_FORMAT into RET_VAL from v$session where id = session_id();

        ELSEIF UP_PARAMETER = 'NLS_TERRITORY' THEN
            select NLS_TERRITORY into RET_VAL from v$session where id = session_id();

        ELSEIF UP_PARAMETER = 'ACTION' THEN
            SELECT ACTION INTO RET_VAL FROM V$SESSION WHERE ID = SESSION_ID();

        ELSEIF UP_PARAMETER = 'CURRENT_SCHEMA' THEN
            RET_VAL := USER_NAME();

        ELSEIF UP_PARAMETER = 'DB_NAME' THEN
            SELECT DB_NAME INTO RET_VAL FROM V$DATABASE;

        ELSEIF UP_PARAMETER = 'HOST' THEN
            RET_VAL := HOST_NAME();

        ELSEIF UP_PARAMETER = 'INSTANCE' THEN
            RET_VAL := '1';

        ELSEIF UP_PARAMETER = 'MODULE' THEN
            SELECT MODULE INTO RET_VAL FROM V$SESSION WHERE ID = SESSION_ID();

        ELSEIF UP_PARAMETER = 'SESSION_USER' THEN
            RET_VAL := USER_NAME();

        ELSEIF UP_PARAMETER = 'SID' THEN
            RET_VAL := SESSION_ID();

        ELSE
            RET_VAL := NULL;
        END IF;
    ELSE
        RET_VAL := NULL;

    END IF;

    RETURN RET_VAL;
END;
/

create or replace public synonym sys_context for sys.sys_context;
grant execute on sys.sys_context to public;
