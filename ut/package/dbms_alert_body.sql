/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body dbms_alert as

procedure register( name in varchar(30) )
as
a integer;
begin
    a := sp_register( name );
end register;

procedure remove_event( name in varchar(30) )
as
a integer;
begin
    a := sp_remove( name );
end remove_event;

procedure removeall()
as
a integer;
begin
    a := sp_removeall();
end removeall;

procedure set_defaults( poll_interval in integer )
as
a integer;
begin
    a := sp_set_defaults( poll_interval );
end set_defaults;

procedure signal( name    in varchar(30),
                  message in varchar(1800) )
as
a integer;
begin
    a := sp_signal( name, message );
end signal;

procedure waitany( name    out varchar(30),
                   message out varchar(1800),
                   status  out integer,
                   timeout in  integer default 86400000 )
as
a integer;
begin
    a := sp_waitany( name, message, status, timeout );
end waitany;

procedure waitone( name    in varchar(30),
                   message out varchar(1800),
                   status  out integer,
                   timeout in integer default 86400000 )
as
a integer;
begin
    a := sp_waitone( name, message, status, timeout );
end waitone;

end dbms_alert;
/
