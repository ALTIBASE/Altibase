drop table demo_meta4;
CREATE TABLE DEMO_META4 ( id char(8), name varchar(20), age integer,
birth date, sex smallint, etc numeric(10,3) );
create index demo_meta4_idx on demo_meta4( id, name desc);

