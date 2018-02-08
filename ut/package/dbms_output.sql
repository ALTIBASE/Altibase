/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package dbms_output authid current_user as

procedure put(a VARCHAR(65534));
pragma restrict_references(put,WNDS,RNDS);

procedure put_line(a VARCHAR(65533));
pragma restrict_references(put_line,WNDS,RNDS);

procedure new_line;
pragma restrict_references(new_line,WNDS,RNDS);

end;
/

create or replace public synonym dbms_output for dbms_output;
grant execute on dbms_output to public;

