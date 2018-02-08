/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

CREATE OR REPLACE PACKAGE utl_file AUTHID CURRENT_USER AS

invalid_path         EXCEPTION;
invalid_mode         EXCEPTION;
invalid_filehandle   EXCEPTION;
invalid_operation    EXCEPTION;
read_error           EXCEPTION;
write_error          EXCEPTION;
invalid_filename     EXCEPTION;
access_denied        EXCEPTION;
delete_failed        EXCEPTION;
rename_failed        EXCEPTION;

invalid_path_errcode        CONSTANT INTEGER:= 201237;
invalid_mode_errcode        CONSTANT INTEGER:= 201235;
invalid_filehandle_errcode  CONSTANT INTEGER:= 201238;
invalid_operation_errcode   CONSTANT INTEGER:= 201239;
read_error_errcode          CONSTANT INTEGER:= 201242;
write_error_errcode         CONSTANT INTEGER:= 201243;
access_denied_errcode       CONSTANT INTEGER:= 201236;
delete_failed_errcode       CONSTANT INTEGER:= 201240;
rename_failed_errcode       CONSTANT INTEGER:= 201241;

PRAGMA EXCEPTION_INIT(invalid_path,        201237);
PRAGMA EXCEPTION_INIT(invalid_mode,        201235);
PRAGMA EXCEPTION_INIT(invalid_filehandle,  201238);
PRAGMA EXCEPTION_INIT(invalid_operation,   201239);
PRAGMA EXCEPTION_INIT(read_error,          201239);
PRAGMA EXCEPTION_INIT(write_error,         201243);
PRAGMA EXCEPTION_INIT(access_denied,       201236);
PRAGMA EXCEPTION_INIT(delete_failed,       201240);
PRAGMA EXCEPTION_INIT(rename_failed,       201241);


FUNCTION fopen(location     IN VARCHAR(40),
filename     IN VARCHAR(256),
open_mode    IN VARCHAR(4),
max_linesize IN INTEGER DEFAULT NULL)
RETURN file_type;
PRAGMA RESTRICT_REFERENCES(fopen, WNDS, RNDS);


FUNCTION is_open(file IN file_type) RETURN BOOLEAN;
PRAGMA RESTRICT_REFERENCES(is_open, WNDS, RNDS);

PROCEDURE fclose(file IN OUT file_type);
PRAGMA RESTRICT_REFERENCES(fclose, WNDS, RNDS);

PROCEDURE fclose_all;
PRAGMA RESTRICT_REFERENCES(fclose_all, WNDS, RNDS);

PROCEDURE get_line(file   IN file_type,
buffer OUT VARCHAR(32768),
len    IN INTEGER DEFAULT NULL);
PRAGMA RESTRICT_REFERENCES(get_line, WNDS, RNDS);

PROCEDURE put(file   IN file_type,
buffer IN VARCHAR(32768));
PRAGMA RESTRICT_REFERENCES(put, WNDS, RNDS);

PROCEDURE new_line(file  IN file_type,
lines IN INTEGER DEFAULT 1);
PRAGMA RESTRICT_REFERENCES(new_line, WNDS, RNDS);

PROCEDURE put_line(file   IN file_type,
buffer IN VARCHAR(32768),
autoflush IN BOOLEAN DEFAULT FALSE);
PRAGMA RESTRICT_REFERENCES(put_line, WNDS, RNDS);


PROCEDURE fflush(file IN file_type);
PRAGMA RESTRICT_REFERENCES(fflush, WNDS, RNDS);

PROCEDURE fremove(location IN VARCHAR(40),
filename IN VARCHAR(256));
PRAGMA RESTRICT_REFERENCES(fremove, WNDS, RNDS);

PROCEDURE fcopy(src_location  IN VARCHAR(40),
src_filename  IN VARCHAR(256),
dest_location IN VARCHAR(40),
dest_filename IN VARCHAR(256),
start_line    IN INTEGER DEFAULT 1,
end_line      IN INTEGER DEFAULT NULL);
PRAGMA RESTRICT_REFERENCES(fcopy, WNDS, RNDS);

PROCEDURE frename(src_location   IN VARCHAR(40),
src_filename   IN VARCHAR(256),
dest_location  IN VARCHAR(40),
dest_filename  IN VARCHAR(256),
overwrite      IN BOOLEAN DEFAULT FALSE);
PRAGMA RESTRICT_REFERENCES(frename, WNDS, RNDS);
END utl_file;

/

CREATE OR REPLACE PUBLIC SYNONYM utl_file FOR sys.utl_file;
GRANT EXECUTE ON utl_file TO PUBLIC;

