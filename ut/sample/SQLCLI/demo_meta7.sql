drop table demo_meta7;
CREATE TABLE DEMO_META7 ( id char(8), name varchar(20), age integer,
birth date, sex smallint, etc numeric(10,3) );
alter table demo_meta7 add primary key (id, name);
