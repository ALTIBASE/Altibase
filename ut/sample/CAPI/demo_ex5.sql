drop table demo_ex5;
CREATE TABLE DEMO_EX5 ( id char(8), name varchar(20), age integer,
birth date, sex smallint, etc numeric(10,3) );

insert into demo_ex5 values('10000000', 'name1', 10, to_date('1990-10-11 07:10:10','YYYY-MM-DD HH:MI:SS'), 0, 100.22);

insert into demo_ex5 values('20000000', 'name2', 20, to_date('1980-04-21 12:10:10','YYYY-MM-DD HH:MI:SS'), 0, 200.22);

insert into demo_ex5 values('30000000', 'name3', 30, to_date('1970-12-06 08:08:08','YYYY-MM-DD HH:MI:SS'), 0, 300.22);

insert into demo_ex5 values('40000000', 'name4', 40, to_date('1960-11-10 05:05:05','YYYY-MM-DD HH:MI:SS'), 0, 400.22);

