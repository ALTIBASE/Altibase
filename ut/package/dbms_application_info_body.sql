/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body dbms_application_info is

procedure set_module( module_name varchar(128), action_name varchar(128) )
is
    dummy integer;
begin
    dummy := sp_set_module( module_name, action_name );
end set_module;

procedure set_action( action_name varchar(128) )
is
    dummy integer;
    module_name varchar(128);
begin
    select module into module_name from v$session where id = session_id();
    dummy := sp_set_module( module_name, action_name );
end set_action;

procedure read_module(module_name out varchar(128), action_name out varchar(128))
is
begin
    select module, action into module_name, action_name from v$session where id = session_id();
end read_module;

procedure set_client_info(client_info varchar(128))
is
    dummy integer;
begin
    dummy := sp_set_client_info( client_info );
end set_client_info;

procedure read_client_info(client_info out varchar(128))
is
begin
    select client_info into client_info from v$session where id = session_id();
end read_client_info;

end dbms_application_info;
/

