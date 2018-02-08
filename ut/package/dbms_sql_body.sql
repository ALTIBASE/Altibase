/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body dbms_sql
as

function open_cursor() return integer
as
c integer;
begin
c := sp_open_cursor();
return c;
end open_cursor;

function is_open(c in integer) return boolean
as
a boolean;
begin
a := sp_is_open(c);
return a;
end is_open;

procedure parse(c             in integer,
                sql           in varchar2(32000),
                language_flag in integer)
as
a integer;
begin
a := sp_parse(c, sql);
end parse;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in varchar2(32000))
as
a integer;
begin
a := sp_bind_variable_varchar(c, name, value);
end bind_variable;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in char(32000))
as
a integer;
begin
a := sp_bind_variable_char(c, name, value);
end bind_variable;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in integer)
as
a integer;
begin
a := sp_bind_variable_integer(c, name, value);
end bind_variable;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in bigint)
as
a integer;
begin
a := sp_bind_variable_bigint(c, name, value);
end bind_variable;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in smallint)
as
a integer;
begin
a := sp_bind_variable_smallint(c, name, value);
end bind_variable;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in double)
as
a integer;
begin
a := sp_bind_variable_double(c, name, value);
end bind_variable;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in real)
as
a integer;
begin
a := sp_bind_variable_real(c, name, value);
end bind_variable;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in numeric(38))
as
a integer;
begin
a := sp_bind_variable_numeric(c, name, value);
end bind_variable;

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in date)
as
a integer;
begin
a := sp_bind_variable_date(c, name, value);
end bind_variable;

function execute_cursor(c in integer) return bigint
as
a bigint;
begin
a := sp_execute_cursor(c);
return a;
end execute_cursor;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in varchar2(32000))
as
begin
null;
end define_column;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in char(32000))
as
begin
null;
end define_column;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in integer)
as
begin
null;
end define_column;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in bigint)
as
begin
null;
end define_column;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in smallint)
as
begin
null;
end define_column;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in double)
as
begin
null;
end define_column;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in real)
as
begin
null;
end define_column;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in numeric(38))
as
begin
null;
end define_column;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in date)
as
begin
null;
end define_column;

procedure define_column_char(c            in integer,
                             position     in integer,
                             column_value in char(32000),
                             column_size  in integer)
as
begin
null;
end define_column_char;

function fetch_rows(c integer) return integer
as
a integer;
begin
a := sp_fetch_rows(c);
return a;
end fetch_rows;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out varchar2(32000))
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_varchar(c,
                             position,
                             column_value,
                             column_error,
                             actual_length);
end column_value;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out char(32000))
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_char(c,
                          position,
                          column_value,
                          column_error,
                          actual_length);
end column_value;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out integer)
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_integer(c,
                             position,
                             column_value,
                             column_error,
                             actual_length);
end column_value;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out bigint)
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_bigint(c,
                            position,
                            column_value,
                            column_error,
                            actual_length);
end column_value;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out smallint)
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_smallint(c,
                              position,
                              column_value,
                              column_error,
                              actual_length);
end column_value;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out double)
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_double(c,
                            position,
                            column_value,
                            column_error,
                            actual_length);
end column_value;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out real)
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_real(c,
                          position,
                          column_value,
                          column_error,
                          actual_length);
end column_value;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out numeric(38))
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_numeric(c,
                             position,
                             column_value,
                             column_error,
                             actual_length);
end column_value;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out date)
as
a integer;
column_error integer;
actual_length integer;
begin
a := sp_column_value_date(c,
                          position,
                          column_value,
                          column_error,
                          actual_length);
end column_value;

procedure close_cursor(c in out integer)
as
a integer;
begin
a := sp_close_cursor(c);
c := NULL;
end close_cursor;

function last_error_position() return integer
as
c integer;
begin
c := sp_last_error_position();
return c;
end last_error_position;

end dbms_sql;
/
