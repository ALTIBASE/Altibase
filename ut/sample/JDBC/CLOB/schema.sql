--###########################################
--# TABLE 
--###########################################

DROP TABLE TEST_TEXT;
CREATE TABLE TEST_TEXT 
(
   TEXT_ID         INTEGER,
   TEXT_TITLE      VARCHAR(128),
   TEXT_CONTENTS   CLOB,
   TEXT_SIZE       INTEGER 
);

DROP TABLE TEST_SAMPLE_TEXT;
CREATE TABLE TEST_SAMPLE_TEXT
( 
   ID    INTEGER,
   TEXT  CLOB
);
