drop table demo_meta3;
CREATE TABLE DEMO_META3 ( id char(8), name varchar(20), age integer,
birth date, sex smallint, etc numeric(10,3) );

ALTER TABLE DEMO_META3 ADD CONSTRAINT DEMO_META3_CONS1 PRIMARY KEY (ID,NAME);
