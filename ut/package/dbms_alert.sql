/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package dbms_alert authid current_user as

procedure register( name in varchar(30) );

procedure remove_event( name in varchar(30) );

procedure removeall();

procedure set_defaults( poll_interval in integer );

procedure signal( name    in varchar(30),
                  message in varchar(1800) );

procedure waitany( name    out varchar(30),
                   message out varchar(1800),
                   status  out integer,
                   timeout in  integer default 86400000 );

procedure waitone( name    in  varchar(30),
                   message out varchar(1800),
                   status  out integer,
                   timeout in  integer default 86400000 );

end dbms_alert;
/

create public synonym dbms_alert for sys.dbms_alert;
grant execute on sys.dbms_alert to public;
