create or replace procedure demo_meta5_proc( a1 in integer,
a2 in char(10), a3 out integer)
as
begin
a3 := 1;
end;
/

create or replace function demo_meta5_func( a1 in integer,
a2 in char(10), a3 out integer)
return integer
as
begin
a3 := 1;
return 1;
end;
/
