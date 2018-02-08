/***********************************************************************
 * Copyright 1999-2018, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE BODY utl_copyswap IS

--  Check Precondition
--  Input parameters:
--    source_user_name  : the owner name of the original table
--    source_table_name : the original table name
PROCEDURE check_precondition( source_user_name  IN VARCHAR(128),
                              source_table_name IN VARCHAR(128) )
AS
    source_user_id              INTEGER;
    source_table_id             INTEGER;
    user_name                   VARCHAR(128);
    is_autocommit               INTEGER;
    repl_mode                   INTEGER;
    repl_port_no                VARCHAR(256);
    repl_ddl_enable             VARCHAR(256);
    repl_allow_duplicate_hosts  VARCHAR(256);
    repl_check_fk_disable       VARCHAR(256);
    foreign_key_count           INTEGER;
    foreign_key_child_count     INTEGER;
    compressed_column_count     INTEGER;
    eager_sender_thread_count   INTEGER;
    eager_receiver_thread_count INTEGER;
BEGIN
    SELECT USER_ID INTO source_user_id FROM SYSTEM_.SYS_USERS_ WHERE USER_NAME = source_user_name;
    SELECT TABLE_ID INTO source_table_id FROM SYSTEM_.SYS_TABLES_ WHERE USER_ID = source_user_id AND TABLE_NAME = source_table_name;

    SELECT DB_USERNAME, AUTOCOMMIT_FLAG, REPLICATION_MODE
      INTO user_name, is_autocommit, repl_mode
      FROM V$SESSION WHERE ID = SESSION_ID();

    -- [권한]
    -- SYS 사용자이어야 한다.
    IF ( user_name != 'SYS' ) THEN
        PRINTLN( '[CONNECT] Only SYS user can use this package.' );
    END IF;

    -- [세션 프라퍼티]
    -- AUTOCOMMIT 세션 프라퍼티가 FALSE이어야 한다.
    IF ( is_autocommit != 0 ) THEN
        PRINTLN( '[SESSION PROPERTY] AUTOCOMMIT property value must be FALSE.' );
    END IF;

    -- REPLICATION 세션 프라퍼티가 TRUE이어야 한다.
    IF ( repl_mode != 0 ) THEN
        PRINTLN( '[SESSION PROPERTY] REPLICATION property value must be TRUE.' );
    END IF;

    -- [시스템 프라퍼티]
    -- REPLICATION_PORT_NO 시스템 프라퍼티가 0이 아니어야 한다.
    SELECT VALUE1 INTO repl_port_no FROM V$PROPERTY WHERE NAME = 'REPLICATION_PORT_NO';

    IF ( repl_port_no = '0' ) THEN
        PRINTLN( '[SYSTEM PROPERTY] REPLICATION_PORT_NO property value must be larger than 0.' );
    END IF;

    -- REPLICATION_DDL_ENABLE 시스템 프라퍼티가 1이어야 한다.
    SELECT VALUE1 INTO repl_ddl_enable FROM V$PROPERTY WHERE NAME = 'REPLICATION_DDL_ENABLE';

    IF ( repl_ddl_enable = '0' ) THEN
        PRINTLN( '[SYSTEM PROPERTY] REPLICATION_DDL_ENABLE property value must be 1.' );
    END IF;

    -- REPLICATION_ALLOW_DUPLICATE_HOSTS 시스템 프라퍼티가 1이어야 한다.
    SELECT VALUE1 INTO repl_allow_duplicate_hosts FROM V$PROPERTY WHERE NAME = 'REPLICATION_ALLOW_DUPLICATE_HOSTS';

    IF ( repl_allow_duplicate_hosts = '0' ) THEN
        PRINTLN( '[SYSTEM PROPERTY] REPLICATION_ALLOW_DUPLICATE_HOSTS property value must be 1.' );
    END IF;

    -- Foreign Key가 있는 경우, CHECK_FK_IN_CREATE_REPLICATION_DISABLE 프라퍼티가 1이어야 한다.
    SELECT MEMORY_VALUE1 INTO repl_check_fk_disable FROM X$PROPERTY WHERE NAME = 'CHECK_FK_IN_CREATE_REPLICATION_DISABLE';

    SELECT COUNT(*) INTO foreign_key_count
      FROM SYSTEM_.SYS_CONSTRAINTS_
     WHERE TABLE_ID = source_table_id AND CONSTRAINT_TYPE = 0;

    IF ( foreign_key_count > 0 ) AND ( repl_check_fk_disable = '0' ) THEN
        PRINTLN( '[SYSTEM PROPERTY] A foreign key exists. CHECK_FK_IN_CREATE_REPLICATION_DISABLE property value must be 1.' );
    END IF;

    SELECT COUNT(*) INTO foreign_key_child_count
      FROM SYSTEM_.SYS_CONSTRAINTS_
     WHERE REFERENCED_TABLE_ID = source_table_id AND CONSTRAINT_TYPE = 0;

    IF ( foreign_key_child_count > 0 ) AND ( repl_check_fk_disable = '0' ) THEN
        PRINTLN( '[SYSTEM PROPERTY] A referencing table exists. CHECK_FK_IN_CREATE_REPLICATION_DISABLE property value must be 1.' );
    END IF;

    -- [Replication 제약 조건]
    -- Compressed Column을 지원하지 않는다.
    SELECT COUNT(*) INTO compressed_column_count FROM SYSTEM_.SYS_COMPRESSION_TABLES_ WHERE TABLE_ID = source_table_id;

    IF ( compressed_column_count > 0 ) THEN
        PRINTLN( '[REPLICATION] This package does not support a compressed column.' );
    END IF;

    -- 관련 Eager Sender/Receiver Thread가 없어야 한다.
    SELECT COUNT(*) INTO eager_sender_thread_count
      FROM V$REPSENDER
     WHERE REP_NAME IN ( SELECT DISTINCT(A.REPLICATION_NAME)
                           FROM SYSTEM_.SYS_REPL_ITEMS_ A, SYSTEM_.SYS_REPLICATIONS_ B
                          WHERE A.LOCAL_USER_NAME = source_user_name AND
                                A.LOCAL_TABLE_NAME = source_table_name AND
                                A.REPLICATION_NAME = B.REPLICATION_NAME AND
                                B.REPL_MODE = 2 );

    IF ( eager_sender_thread_count > 0 ) THEN
        PRINTLN( '[REPLICATION] This package does not support a table using eager sender threads.' );
    END IF;

    SELECT COUNT(*) INTO eager_receiver_thread_count
      FROM V$REPRECEIVER
     WHERE REP_NAME IN ( SELECT DISTINCT(A.REPLICATION_NAME)
                           FROM SYSTEM_.SYS_REPL_ITEMS_ A, SYSTEM_.SYS_REPLICATIONS_ B
                          WHERE A.LOCAL_USER_NAME = source_user_name AND
                                A.LOCAL_TABLE_NAME = source_table_name AND
                                A.REPLICATION_NAME = B.REPLICATION_NAME AND
                                B.REPL_MODE = 2 );

    IF ( eager_receiver_thread_count > 0 ) THEN
        PRINTLN( '[REPLICATION] This package does not support a table using eager receiver threads.' );
    END IF;
END check_precondition;

--  Copy Table Schema
--  Input parameters:
--    target_user_name  : the owner name of the copy table
--    target_table_name : the copy table name
--    source_user_name  : the owner name of the original table
--    source_table_name : the original table name
PROCEDURE copy_table_schema( target_user_name   IN VARCHAR(128),
                             target_table_name  IN VARCHAR(128),
                             source_user_name   IN VARCHAR(128),
                             source_table_name  IN VARCHAR(128) )
AS
    sql_buffer      VARCHAR(1024);
    code            INTEGER;
    errm            VARCHAR(1024);
BEGIN
    sql_buffer := 'CREATE TABLE "' || target_user_name || '"."' || target_table_name || '" FROM TABLE SCHEMA "' ||
                                      source_user_name || '"."' || source_table_name || '" USING PREFIX __COPY_';
    EXECUTE IMMEDIATE sql_buffer;
EXCEPTION WHEN OTHERS THEN
    code := SQLCODE;
    errm := SUBSTRING( SQLERRM, 1, 1024 );
    PRINTLN( 'SQLCODE : ' || code );
    PRINTLN( 'SQLERRM : ' || errm );
END copy_table_schema;

--  Replicate Table
--  Input parameters:
--    replication_name  : the local replication name
--    target_user_name  : the owner name of the copy table
--    target_table_name : the copy table name
--    source_user_name  : the owner name of the original table
--    source_table_name : the original table name
--    sync_parallel_factor   : the parallel factor for initial synchronization
--    receiver_applier_count : the parallel factor for incremental synchronization
PROCEDURE replicate_table( replication_name         IN VARCHAR(35),
                           target_user_name         IN VARCHAR(128),
                           target_table_name        IN VARCHAR(128),
                           source_user_name         IN VARCHAR(128),
                           source_table_name        IN VARCHAR(128),
                           sync_parallel_factor     IN INTEGER DEFAULT 8,
                           receiver_applier_count   IN INTEGER DEFAULT 8 )
AS
    repl_port_no    VARCHAR(256);
    sql_buffer      VARCHAR(1024);
    code            INTEGER;
    errm            VARCHAR(1024);
BEGIN
    SELECT VALUE1 INTO repl_port_no FROM V$PROPERTY WHERE NAME = 'REPLICATION_PORT_NO';

    -- Local Replication을 생성한다.
    sql_buffer := 'CREATE REPLICATION "' || replication_name || '" OPTIONS LOCAL "' || replication_name || '_PEER"' ||
                  ' WITH ''127.0.0.1'', ' || repl_port_no ||
                  ' FROM "' || source_user_name || '"."' || source_table_name || '" TO "' || target_user_name || '"."' || target_table_name || '"';
    EXECUTE IMMEDIATE sql_buffer;

    sql_buffer := 'CREATE REPLICATION "' || replication_name || '_PEER" OPTIONS LOCAL "' || replication_name || '"' ||
                  ' WITH ''127.0.0.1'', ' || repl_port_no ||
                  ' FROM "' || target_user_name || '"."' || target_table_name || '" TO "' || source_user_name || '"."' || source_table_name || '"';
    EXECUTE IMMEDIATE sql_buffer;

    sql_buffer := 'ALTER REPLICATION "' || replication_name || '" SET PARALLEL ' || TO_CHAR( receiver_applier_count );
    EXECUTE IMMEDIATE sql_buffer;

    -- Table을 복제한다.
    sql_buffer := 'ALTER REPLICATION "' || replication_name || '" SYNC PARALLEL ' || TO_CHAR( sync_parallel_factor );
    EXECUTE IMMEDIATE sql_buffer;

    sql_buffer := 'ALTER REPLICATION "' || replication_name || '" FLUSH';
    EXECUTE IMMEDIATE sql_buffer;
EXCEPTION WHEN OTHERS THEN
    code := SQLCODE;
    errm := SUBSTRING( SQLERRM, 1, 1024 );
    PRINTLN( 'DDL : ' || sql_buffer );
    PRINTLN( 'SQLCODE : ' || code );
    PRINTLN( 'SQLERRM : ' || errm );
    PRINTLN( 'Execute finish() for cleaning.' );
END replicate_table;

--  Swap Table
--  Input parameters:
--    replication_name  : the local replication name
--    target_user_name  : the owner name of the copy table
--    target_table_name : the copy table name
--    source_user_name  : the owner name of the original table
--    source_table_name : the original table name
--    force_to_rename_encrypt_column : if the tables have a encrypt column, set it to TRUE.
--    ignore_foreign_key_child       : if the tables have a foreign key child, set it to TRUE.
PROCEDURE swap_table( replication_name                  IN VARCHAR(35),
                      target_user_name                  IN VARCHAR(128),
                      target_table_name                 IN VARCHAR(128),
                      source_user_name                  IN VARCHAR(128),
                      source_table_name                 IN VARCHAR(128),
                      force_to_rename_encrypt_column    IN BOOLEAN DEFAULT FALSE,
                      ignore_foreign_key_child          IN BOOLEAN DEFAULT FALSE )
AS
    sql_buffer      VARCHAR(1024);
    sql_buffer_temp VARCHAR(1024);
    code            INTEGER;
    errm            VARCHAR(1024);
BEGIN
    sql_buffer := 'ALTER REPLICATION "' || replication_name || '" FLUSH';
    EXECUTE IMMEDIATE sql_buffer;

    -- 마지막으로 남은 DML을 반영하고 원본과 교체하기 전에, X Lock을 획득한다.
    sql_buffer := 'LOCK TABLE "' || source_user_name || '"."' || source_table_name || '" IN EXCLUSIVE MODE UNTIL NEXT DDL';
    EXECUTE IMMEDIATE sql_buffer;

    sql_buffer := 'ALTER REPLICATION "' || replication_name || '" FLUSH';
    EXECUTE IMMEDIATE sql_buffer;

    sql_buffer := 'ALTER TABLE "' || target_user_name || '"."' || target_table_name || '" REPLACE "' ||
                                     source_user_name || '"."' || source_table_name || '" USING PREFIX __COPY_';

    IF ( force_to_rename_encrypt_column = TRUE ) THEN
        sql_buffer_temp := sql_buffer || ' RENAME FORCE';
    ELSE
        sql_buffer_temp := sql_buffer;
    END IF;

    IF ( ignore_foreign_key_child = TRUE ) THEN
        sql_buffer := sql_buffer_temp || ' IGNORE FOREIGN KEY CHILD';
    ELSE
        sql_buffer := sql_buffer_temp;
    END IF;

    EXECUTE IMMEDIATE sql_buffer;
EXCEPTION WHEN OTHERS THEN
    code := SQLCODE;
    errm := SUBSTRING( SQLERRM, 1, 1024 );
    PRINTLN( 'SQLCODE : ' || code );
    PRINTLN( 'SQLERRM : ' || errm );
    PRINTLN( 'Check the error, and retry swap_table().' );
END swap_table;

--  Finish
--  Input parameters:
--    replication_name  : the local replication name
--    target_user_name  : the owner name of the copy table
--    target_table_name : the copy table name
--    print_all_errors  : if it is set to TRUE, all replication-related errors are printed.
PROCEDURE finish( replication_name     IN VARCHAR(35),
                  target_user_name     IN VARCHAR(128),
                  target_table_name    IN VARCHAR(128),
                  print_all_errors     IN BOOLEAN DEFAULT FALSE )
AS
    sql_buffer      VARCHAR(1024);
    code            INTEGER;
    errm            VARCHAR(1024);
BEGIN
    -- 실패해도 다음 단계를 진행한다.
    sql_buffer := 'ALTER REPLICATION "' || replication_name || '" STOP';
    BEGIN
        EXECUTE IMMEDIATE sql_buffer;
    EXCEPTION WHEN OTHERS THEN
        IF ( print_all_errors = TRUE ) THEN
            code := SQLCODE;
            errm := SUBSTRING( SQLERRM, 1, 1024 );
            PRINTLN( 'DDL : ' || sql_buffer );
            PRINTLN( 'SQLCODE : ' || code );
            PRINTLN( 'SQLERRM : ' || errm );
        END IF;
    END;

    -- 실패해도 다음 단계를 진행한다.
    sql_buffer := 'DROP REPLICATION "' || replication_name || '"';
    BEGIN
        EXECUTE IMMEDIATE sql_buffer;
    EXCEPTION WHEN OTHERS THEN
        IF ( print_all_errors = TRUE ) THEN
            code := SQLCODE;
            errm := SUBSTRING( SQLERRM, 1, 1024 );
            PRINTLN( 'DDL : ' || sql_buffer );
            PRINTLN( 'SQLCODE : ' || code );
            PRINTLN( 'SQLERRM : ' || errm );
        END IF;
    END;

    -- 실패해도 다음 단계를 진행한다.
    sql_buffer := 'DROP REPLICATION "' || replication_name || '_PEER"';
    BEGIN
        EXECUTE IMMEDIATE sql_buffer;
    EXCEPTION WHEN OTHERS THEN
        IF ( print_all_errors = TRUE ) THEN
            code := SQLCODE;
            errm := SUBSTRING( SQLERRM, 1, 1024 );
            PRINTLN( 'DDL : ' || sql_buffer );
            PRINTLN( 'SQLCODE : ' || code );
            PRINTLN( 'SQLERRM : ' || errm );
        END IF;
    END;

    sql_buffer := 'DROP TABLE "' || target_user_name || '"."' || target_table_name || '"';
    BEGIN
        EXECUTE IMMEDIATE sql_buffer;
    EXCEPTION WHEN OTHERS THEN
        code := SQLCODE;
        errm := SUBSTRING( SQLERRM, 1, 1024 );
        PRINTLN( 'DDL : ' || sql_buffer );
        PRINTLN( 'SQLCODE : ' || code );
        PRINTLN( 'SQLERRM : ' || errm );
    END;
END finish;

END utl_copyswap;
/

