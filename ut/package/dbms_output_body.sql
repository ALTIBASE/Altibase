/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body dbms_output is

procedure put(a VARCHAR(65534))
as
    dummy integer;

begin
    dummy := nvl2(print_out( a ), 0, -1);
end;

procedure put_line(a VARCHAR(65533))
as
    dummy integer;
begin
    dummy := nvl2(print_out ( a || chr(10)), 0, -1);
end;

procedure new_line
as
    dummy integer;
begin
    dummy := nvl2(print_out( chr(10)), 0, -1);
end;

end;

/

