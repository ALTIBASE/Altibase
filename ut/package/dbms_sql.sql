/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package dbms_sql authid current_user 
as
native constant integer := 0;

function  open_cursor() return integer;

function  is_open(c in integer) return boolean;

procedure parse(c             in integer,
                sql           in varchar2(32000),
                language_flag in integer);

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in varchar2(32000));

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in char(32000));

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in integer);

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in bigint);

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in smallint);

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in double);

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in real);

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in numeric(38));

procedure bind_variable(c     in integer,
                        name  in varchar2(128),
                        value in date);

function  execute_cursor(c in integer) return bigint;

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in varchar2(32000));

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in char(32000));

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in integer);

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in bigint);

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in smallint);

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in double);

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in real);

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in numeric(38));

procedure define_column(c            in integer,
                        position     in integer,
                        column_value in date);

procedure define_column_char(c            in integer,
                             position     in integer,
                             column_value in char(32000),
                             column_size  in integer);

function fetch_rows(c integer) return integer;

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out varchar2(32000));

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out char(32000));

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out integer);

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out bigint);

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out smallint);

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out double);

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out real);

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out numeric(38));

procedure column_value(c             in integer,
                       position      in integer,
                       column_value  out date);

procedure close_cursor(c in out integer);

function  last_error_position() return integer;

end dbms_sql;
/

create public synonym dbms_sql for sys.dbms_sql;
grant execute on sys.dbms_sql to public;
