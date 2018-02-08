drop table demo_plan;
CREATE TABLE DEMO_PLAN ( id char(8),
name varchar(20) not null,
age integer not null);

ALTER TABLE DEMO_PLAN ADD CONSTRAINT DEMO_PLAN_CONS1 PRIMARY KEY (ID);

insert into demo_plan values('10000000', 'name1', 18);
insert into demo_plan values('20000000', 'name2', 28);
insert into demo_plan values('30000000', 'name3', 38);
