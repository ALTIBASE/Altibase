drop table demo_ex6;
CREATE TABLE DEMO_EX6 ( id integer, name varchar(20), birth date );

insert into demo_ex6 values(1, 'name1', to_date('1990-10-11 07:10:10','YYYY-MM-DD HH:MI:SS'));

insert into demo_ex6 values(2, 'name2', to_date('1980-04-21 12:10:10','YYYY-MM-DD HH:MI:SS'));

insert into demo_ex6 values(3, 'name3', to_date('1970-12-06 08:08:08','YYYY-MM-DD HH:MI:SS'));

insert into demo_ex6 values(4, 'name4', to_date('1960-11-10 05:05:05','YYYY-MM-DD HH:MI:SS'));

create or replace procedure demo_proc6(a1 in integer, a2 in varchar(20), a3 in date, a4 out integer)
as
begin
insert into demo_ex6 values(a1, a2, a3);
a4 := 1;
end;
/
