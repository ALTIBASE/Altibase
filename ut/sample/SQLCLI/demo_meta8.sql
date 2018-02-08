drop table demo_meta8;
CREATE TABLE DEMO_META8 (
id char(8) primary key,
name varchar(20) not null,
age integer not null,
birth date,
sex smallint,
etc numeric(10,3)
);
