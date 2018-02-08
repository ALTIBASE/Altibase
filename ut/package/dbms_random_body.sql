/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE BODY dbms_random AS
mem        num_array;           -- big internal state hidden from the user
counter    INTEGER := 55;-- counter through the results
saved_norm NUMBER := NULL;      -- unused random normally distributed value
need_init  BOOLEAN := TRUE;     -- do we still need to initialize


-- Seed the random number generator with a binary_integer
PROCEDURE seed(val IN INTEGER) IS
    dummy integer;
BEGIN
    dummy := RANDOM(val);
END seed;


-- Seed the random number generator with a string.
PROCEDURE seed(val IN VARCHAR(2000)) IS
junk     VARCHAR2(2000);
piece    VARCHAR2(20);
randval  NUMBER;
mytemp   NUMBER;
j        INTEGER;
BEGIN
    need_init   := FALSE;
    saved_norm  := NULL;
    counter     := 0;
    junk        := val;

    FOR i IN 0 .. 54 LOOP
        piece   := SUBSTR(junk,1,19);
        randval := 0;
        j       := 1;

        -- convert 19 characters to a 38-digit number
        FOR j IN 1 .. 19 LOOP
            randval := 1e2*randval + NVL(ASCII(SUBSTR(piece,j,1)),0.0);
        END LOOP;

        -- try to avoid lots of zeros
        randval := randval*1e-38+i*.01020304050607080910111213141516171819;
        mem(i)  := randval - TRUNC(randval);

        -- we've handled these first 19 characters already; move on
        junk    := SUBSTR(junk,20);
    END LOOP;

    randval := mem(54);
    FOR j IN 0 .. 10 LOOP
        FOR i IN 0 .. 54 LOOP

            -- barrelshift mem(i-1) by 24 digits
            randval := randval * 1e24;
            mytemp  := TRUNC(randval);
            randval := (randval - mytemp) + (mytemp * 1e-38);

            -- add it to mem(i)
            randval := mem(i)+randval;
            IF (randval >= 1.0) THEN
                randval := randval - 1.0;
            END IF;

            -- record the result
            mem(i) := randval;
        END LOOP;
    END LOOP;
END seed;


FUNCTION replay_random_number RETURN NUMBER IS
    randval NUMBER;
    randmax NUMBER;
begin
    randmax := 1000000000;
    randval := mod(random(0) , randmax);
    randval := cast(randval as number) / cast((randmax + 1) as number);
    return randval;
end replay_random_number;

-- give values to the user
-- Delayed Fibonacci, pilfered from Knuth volume 2
FUNCTION value RETURN NUMBER    IS
    randval  NUMBER;
BEGIN

    randval := replay_random_number();  -- null if not in replay mode
    IF randval IS NOT NULL THEN
        RETURN randval;
    END IF;

    counter := counter + 1;
    IF counter >= 55 THEN

        -- initialize if needed
        IF (need_init = TRUE) THEN
            seed( TO_CHAR(SYSDATE,'MM-DD-YYYY HH24:MI:SS') || USER_name() || session_id());
        ELSE
        -- need to generate 55 more results
            FOR i IN 0 .. 30 LOOP
                randval := mem(i+24) + mem(i);
                IF (randval >= 1.0) THEN
                    randval := randval - 1.0;
                END IF;
                mem(i) := randval;
            END LOOP;

            FOR i IN 31 .. 54 LOOP
                randval := mem(i-31) + mem(i);
                IF (randval >= 1.0) THEN
                    randval := randval - 1.0;
                END IF;
                mem(i) := randval;
            END LOOP;
        END IF;
        counter := 0;
    END IF;

    //record_random_number(mem(counter));  -- no-op if not in recording

    RETURN mem(counter);
END value;


-- Random 38-digit number between LOW and HIGH.
FUNCTION value ( low in NUMBER, high in NUMBER) RETURN NUMBER
  is
BEGIN
    RETURN (value*(high-low))+low;
END value;

-- Random string.  Pilfered from Chris Ellis.

/* "opt" specifies that the returned string may contain:
 'u','U'  :  upper case alpha characters only
 'l','L'  :  lower case alpha characters only
 'a','A'  :  alpha characters only (mixed case)
 'x','X'  :  any alpha-numeric characters (upper)
 'p','P'  :  any printable characters
 */
FUNCTION string (opt char(1), len NUMBER)
RETURN VARCHAR(4000)   is
begin
    return random_string(opt, len);
end string;

-- For compatibility with 8.1
PROCEDURE initialize(val IN INTEGER) IS
BEGIN
    seed(to_char(val));
END initialize;


-- For compatibility with 8.1
-- Random binary_integer, -power(2,31) <= Random < power(2,31)
-- Delayed Fibonacci, pilfered from Knuth volume 2
FUNCTION random RETURN BIGINT IS
BEGIN
    RETURN TRUNC( value()*4294967296 )-2147483648;
END random;


-- For compatibility with 8.1
PROCEDURE terminate IS
BEGIN
    NULL;
END terminate;

END dbms_random;
/

