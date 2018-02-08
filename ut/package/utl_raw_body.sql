/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body utl_raw is

/*----------------------------------------------------------------*/
/*  CONCAT                                                        */
/*----------------------------------------------------------------*/
--  Concatenate a set of 12 raws into a single raw.  If the
--    concatenated size exceeds 32K, an error is returned.

--  Input parameters:
--    r1....r12 are the raw items to concatenate.

--  Defaults and optional parameters: None

--  Return value:
--    raw - containing the items concatenated.

--  Errors:
--    VALUE_ERROR exception raised (to be revised in a future release)
--        If the sum of the lengths of the inputs exceeds the maximum
--        allowable length for a RAW.
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
r12 IN RAW(32767) DEFAULT NULL) RETURN RAW(32767)
as
    res raw(32767);
begin
    res := raw_concat(r1, r2);
    res := raw_concat(res, r3);
    res := raw_concat(res, r4);
    res := raw_concat(res, r5);
    res := raw_concat(res, r6);
    res := raw_concat(res, r7);
    res := raw_concat(res, r8);
    res := raw_concat(res, r9);
    res := raw_concat(res, r10);
    res := raw_concat(res, r11);
    res := raw_concat(res, r12);

    return res;
end concat;

/*----------------------------------------------------------------*/
/*  CAST_TO_RAW                                                   */
/*----------------------------------------------------------------*/
--  Cast a varchar2 to a raw
--    This function converts a varchar2 represented using N data bytes
--    into a raw with N data bytes.
--    The data is not modified in any way, only its datatype is recast
--    to a RAW datatype.

--  Input parameters:
--    c - varchar2 or nvarchar2 to be changed to a raw

--  Defaults and optional parameters: None

--  Return value:
--    raw - containing the same data as the input varchar2 and
--          equal byte length as the input varchar2 and without a
--          leading length field.
--    null - if c input parameter was null

--  Errors:
--     None
FUNCTION cast_to_raw(c IN VARCHAR(32767)) RETURN RAW(32767)
as
    res raw(32767);
begin
    res := to_raw(c);
    return res;
end cast_to_raw;

/*----------------------------------------------------------------*/
/*  CAST_TO_VARCHAR2                                              */
/*----------------------------------------------------------------*/
--  Cast a raw to a varchar2
--    This function converts a raw represented using N data bytes
--    into varchar2 with N data bytes.
--    NOTE: Once the value is cast to a varchar2, string
--          operations or server->client communication will assume
--          that the string is represented using the database's
--          character set.  "Garbage" results are likely if these
--          operations are applied to a string that is actually
--          represented using some other character set.

--  Input parameters:
--    r - raw (without leading length field) to be changed to a
--             varchar2)

--  Defaults and optional parameters: None

--  Return value:
--    varchar2 - containing having the same data as the input raw
--    null     - if r input parameter was null

--  Errors:
--     None

FUNCTION cast_to_varchar2(r IN RAW(32767)) RETURN VARCHAR(32767)
as
    res varchar(32767);
begin
    res := raw_to_varchar(r);
    return res;
end cast_to_varchar2;

/*----------------------------------------------------------------*/
/*  LENGTH                                                        */
/*----------------------------------------------------------------*/
--  Return the length in bytes of a raw r.

--  Input parameters:
--    r - the raw byte stream to be measured

--  Defaults and optional parameters: None

--  Return value:
--    number - equal to the current length of the raw.

--  Errors:
--     None
FUNCTION length(r IN RAW(32767)) RETURN NUMBER
as
    length NUMBER;
begin
    length := raw_sizeof( r );
    return length;
end length;

/*----------------------------------------------------------------*/
/*  SUBSTR                                                        */
/*----------------------------------------------------------------*/
--  Return a substring portion of raw r beginning at pos for len bytes.
--    If pos is positive, substr counts from the beginning of r to find
--    the first byte.  If pos is negative, substr counts backwards from the
--    end of the r.  The value pos cannot be 0.  If len is omitted,
--    substr returns all bytes to the end of r.  The value len cannot be
--    less than 1.

--  Input parameters:
--    r    - the raw byte-string from which a portion is extracted.
--    pos  - the byte pisition in r at which to begin extraction.
--    len  - the number of bytes from pos to extract from r (optional).

--  Defaults and optional parameters:
--    len - position pos thru to the end of r

--  Return value:
--    Portion of r beginning at pos for len bytes long
--    null - if r input parameter was null

--  Errors:
--    VALUE_ERROR exception raised (to be revised in a future release)
--       when pos = 0
--       when len < 0
FUNCTION substr(r   IN RAW(32767),
                pos IN INTEGER,
                len IN INTEGER DEFAULT NULL) RETURN RAW(32767)
as
begin
    return subraw( r, pos, len );
end substr;

/*----------------------------------------------------------------*/
/*  CAST_TO_NUMBER                                                */
/*----------------------------------------------------------------*/
--  Perform a casting of the binary representation of the number (in RAW)
--  into a NUMBER.

--  Input parameters:
--    r - RAW value to be cast as a NUMBER

--  Return value:
--    number
--    null - if r input parameter was null

--  Errors:
--    None

FUNCTION cast_to_number(r IN RAW(40)) RETURN NUMBER
as
begin
    return raw_to_numeric( r );
end cast_to_number;

/*----------------------------------------------------------------*/
/*  CAST_FROM_NUMBER                                              */
/*----------------------------------------------------------------*/
--  Returns the binary representation of a NUMBER in RAW.

--  Input parameters:
--    n - NUMBER to be returned as RAW

--  Return value:
--    raw
--    null - if n input parameter was null

--  Errors:
--    None

FUNCTION cast_from_number(n IN NUMBER) RETURN RAW(40)
as
    res raw(40);
begin
    return to_raw(n);
end cast_from_number;

/*----------------------------------------------------------------*/
/*  CAST_TO_BINARY_INTEGER                                        */
/*----------------------------------------------------------------*/
--  Perform a casting of the binary representation of the raw
--  into a binary integer.  This function will handle PLS_INTEGER as well
--  as binary integer.
--  In the 4 byte, 2 byte, and 1 byte integer cases, here is how the
--  bytes are mapped into a RAW depending on the endianess of the platform:
--
--                   4-byte raw    2-byte raw   1-byte raw
--                     0 1 2 3          0 1           0
--
--     Big-endian      0 1 2 3      - - 0 1     - - - 0
--
--     Little-endian   3 2 1 0      - - 1 0     - - - 0
--
--  Machine-endian refers to the endianess of the database host and
--  can be either big or little endian.  In this case, the bytes are
--  copied straight across into the integer value without endianess
--  handling.
--
--  Machine-endian-lze refers to treating the raw as an unsigned
--  quantity.  This applies only in the 1,2,3-byte cases, as the
--  4-byte case is forced to be signed by our binary_integer datatype.
--  In the case of a 4-byte input and machine-endian-lze, we will treat
--  this as a signed 4-byte quantity - the same as machine-endian.
--
--  Input parameters:
--    r - RAW value to be cast to binary integer
--    endianess - PLS_INTEGER representing endianess.  Valid values are
--                big_endian,little_endian,machine_endian
--                Default=big_endian. --> machine_endian 으로처리한다.

--  Defaults and optional parameters: endianess defaults to big_endian

--  Return value:
--    binary_integer value
--    null - if r input parameter was null

--  Errors:
--    None
FUNCTION cast_to_binary_integer(r IN RAW(8),
                                endianess IN INTEGER DEFAULT 1)
RETURN INTEGER
as
begin
    return raw_to_integer(r);
end cast_to_binary_integer;


/*----------------------------------------------------------------*/
/*  CAST_FROM_BINARY_INTEGER                                      */
/*----------------------------------------------------------------*/
--  Return the RAW representation of a binary_integer value.  This function
--  will handle PLS_INTEGER as well as binary integer.  The mapping applies
--  as follows for little and big-endian platforms:
--
--                    4-byte int
--                     0 1 2 3
--
--     Big-endian      0 1 2 3
--
--     Little-endian   3 2 1 0
--

--  Input parameters:
--    n - binary integer to be returned as RAW
--    endianess - big or little endian PLS_INTEGER constant --> machine_endian으로처리한다.

--  Defaults and optional parameters: endianess defaults to big_endian

--  Return value:
--    raw
--    null - if n input parameter was null

--  Errors:
--    None

FUNCTION cast_from_binary_integer(n         IN INTEGER,
                                  endianess IN INTEGER DEFAULT 1)
RETURN RAW(8)
as
begin
    return to_raw(n);
end cast_from_binary_integer;

end utl_raw;
/

