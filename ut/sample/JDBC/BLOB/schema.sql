--###########################################
--# TABLE 
--###########################################

DROP TABLE TEST_IMAGE;
CREATE TABLE TEST_IMAGE
(
   IMAGE_ID    INTEGER,
   IMAGE_NAME  VARCHAR(128),
   IMAGE_URL   VARCHAR(256),
   IMAGE       BLOB,
   IMAGE_SIZE INTEGER 
);

DROP TABLE TEST_SAMPLE_IMAGE;
CREATE TABLE TEST_SAMPLE_IMAGE
( 
   ID    INTEGER,
   IMAGE BLOB
);
