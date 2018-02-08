/***********************************************************************
 * Copyright 1999-2017, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE STANDARD AUTHID CURRENT_USER AS
  TYPE SYS_REFCURSOR IS REF CURSOR;
END;
/

CREATE PUBLIC SYNONYM STANDARD FOR SYS.STANDARD;
GRANT EXECUTE ON SYS.STANDARD TO PUBLIC;
