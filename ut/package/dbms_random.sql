/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE dbms_random AUTHID CURRENT_USER AS

TYPE num_array IS TABLE OF NUMBER INDEX BY INTEGER;

PROCEDURE seed(val IN INTEGER);
PRAGMA restrict_references (seed, WNDS);

PROCEDURE seed(val IN VARCHAR(2000));
PRAGMA restrict_references (seed, WNDS);

FUNCTION value RETURN NUMBER;

FUNCTION value (low IN NUMBER, high IN NUMBER) RETURN NUMBER;

FUNCTION string (opt char(1), len NUMBER)
RETURN VARCHAR(4000);  

FUNCTION replay_random_number RETURN NUMBER;
PRAGMA restrict_references (replay_random_number, WNDS);

PROCEDURE initialize(val IN INTEGER);

FUNCTION random RETURN BIGINT;

PROCEDURE terminate;

END dbms_random;
/

CREATE OR REPLACE PUBLIC SYNONYM dbms_random FOR sys.dbms_random;
GRANT EXECUTE ON dbms_random TO public;
