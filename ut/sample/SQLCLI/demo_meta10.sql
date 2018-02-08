drop table demo_meta10;
drop user demo_meta10 cascade;
create user demo_meta10 identified by demo_met;
create table demo_meta10 (i1 integer, i2 integer);
grant insert on demo_meta10 to demo_meta10;
grant select on demo_meta10 to demo_meta10 with grant option;
