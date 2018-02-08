/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package dbms_lock authid current_user is

nl_mode  constant integer := 1;
ss_mode  constant integer := 2;
sx_mode  constant integer := 3;
s_mode   constant integer := 4;
ssx_mode constant integer := 5;
x_mode   constant integer := 6;

maxwait  constant integer := 32767;

badseconds_num NUMBER := -38148;

function  request(id in integer,
lockmode in integer default x_mode,
timeout in integer default maxwait,
release_on_commit in boolean default FALSE)
return integer;
function release(id in integer) return integer;
procedure sleep(seconds in number);

end dbms_lock;
/

create or replace public synonym dbms_lock for sys.dbms_lock;
grant execute on dbms_lock to PUBLIC;

