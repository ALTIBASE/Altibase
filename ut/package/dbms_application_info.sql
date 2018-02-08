/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package dbms_application_info authid current_user is

procedure set_module(module_name varchar(128), action_name varchar(128));

procedure set_action(action_name varchar(128));

procedure read_module(module_name out varchar(128), action_name out varchar(128));

procedure set_client_info(client_info varchar(128));

procedure read_client_info(client_info out varchar(128));

end;
/

create public synonym dbms_application_info for sys.dbms_application_info;
grant execute on sys.dbms_application_info to public;

