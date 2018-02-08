/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE utl_raw AUTHID CURRENT_USER IS

big_endian         CONSTANT INTEGER := 1;
little_endian      CONSTANT INTEGER := 2;
machine_endian     CONSTANT INTEGER := 3;

FUNCTION concat(r1  IN RAW(32767) DEFAULT NULL,
r2  IN RAW(32767) DEFAULT NULL,
r3  IN RAW(32767) DEFAULT NULL,
r4  IN RAW(32767) DEFAULT NULL,
r5  IN RAW(32767) DEFAULT NULL,
r6  IN RAW(32767) DEFAULT NULL,
r7  IN RAW(32767) DEFAULT NULL,
r8  IN RAW(32767) DEFAULT NULL,
r9  IN RAW(32767) DEFAULT NULL,
r10 IN RAW(32767) DEFAULT NULL,
r11 IN RAW(32767) DEFAULT NULL,
r12 IN RAW(32767) DEFAULT NULL) RETURN RAW(32767);
pragma RESTRICT_REFERENCES(concat, WNDS, RNDS);

FUNCTION cast_to_raw(c IN VARCHAR(32767)) RETURN RAW(32767);
pragma RESTRICT_REFERENCES(cast_to_raw, WNDS, RNDS);

FUNCTION cast_to_varchar2(r IN RAW(32767)) RETURN VARCHAR(32767);
pragma RESTRICT_REFERENCES(cast_to_varchar2, WNDS, RNDS);

FUNCTION length(r IN RAW(32767)) RETURN NUMBER;
pragma RESTRICT_REFERENCES(length, WNDS, RNDS);

FUNCTION substr(r   IN RAW(32767),
pos IN INTEGER,
len IN INTEGER DEFAULT NULL) RETURN RAW(32767);
pragma RESTRICT_REFERENCES(substr, WNDS, RNDS);

FUNCTION cast_to_number(r IN RAW(40)) RETURN NUMBER;
pragma RESTRICT_REFERENCES(cast_to_number, WNDS, RNDS);

FUNCTION cast_from_number(n IN NUMBER) RETURN RAW(40);
pragma RESTRICT_REFERENCES(cast_from_number, WNDS, RNDS);

FUNCTION cast_to_binary_integer(r IN RAW(8),
endianess IN INTEGER
DEFAULT 1)
RETURN INTEGER;
pragma RESTRICT_REFERENCES(cast_to_binary_integer, WNDS, RNDS);

FUNCTION cast_from_binary_integer(n IN INTEGER,
endianess IN INTEGER
DEFAULT 1)
RETURN RAW(8);
pragma RESTRICT_REFERENCES(cast_from_binary_integer,WNDS,RNDS);

END UTL_RAW;
/

CREATE OR REPLACE PUBLIC SYNONYM utl_raw FOR sys.utl_raw;
GRANT EXECUTE ON utl_raw TO PUBLIC;

