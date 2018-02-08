/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package dbms_utility authid current_user as

function format_call_stack return varchar(65534);

function format_error_backtrace return varchar(65534);

function format_error_stack return varchar(65534);

end dbms_utility;
/

create public synonym dbms_utility for sys.dbms_utility;
grant execute on sys.dbms_utility to public;
