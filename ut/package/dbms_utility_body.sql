/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body dbms_utility as

function format_call_stack return varchar(65534)
as
begin
return sp_format_call_stack();
end format_call_stack;

function format_error_backtrace return varchar(65534)
as
begin
return sp_format_error_backtrace();
end format_error_backtrace;

function format_error_stack return varchar(65534)
as
begin
return sp_format_error_stack();
end format_error_stack;

end dbms_utility;
/
