/***********************************************************************
 * Copyright 1999-2015, ALTIBASE Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

create or replace package body utl_file is

FUNCTION fopen(location     IN VARCHAR(40),
               filename     IN VARCHAR(256),
               open_mode    IN VARCHAR(4),
               max_linesize IN INTEGER DEFAULT NULL)
RETURN file_type
as
file FILE_TYPE;
begin
    file := FILE_OPEN( location, filename, open_mode );
    return file;
end fopen;

FUNCTION is_open(file IN file_type) RETURN BOOLEAN
as
begin
if file is null then
    return false;
else
    return true;
end if;
end is_open;

PROCEDURE fclose(file IN OUT file_type)
as
begin
    file := FILE_CLOSE( file );
end fclose;

PROCEDURE fclose_all
as
    dummy BOOLEAN;
begin
    dummy := FILE_CLOSEALL( TRUE );
end fclose_all;

PROCEDURE get_line( file   IN file_type,
                    buffer OUT VARCHAR(32768),
                    len    IN INTEGER DEFAULT NULL )
as
begin
    buffer := FILE_GETLINE( file, len );
end get_line;

PROCEDURE put(file   IN file_type,
              buffer IN VARCHAR(32768))
as
    dummy BOOLEAN;
begin
    dummy := FILE_PUT( file, buffer, FALSE );
end put;

PROCEDURE new_line(file  IN file_type,
                   lines IN INTEGER DEFAULT 1)
as
    v1 INTEGER;
    dummy BOOLEAN;
BEGIN
    FOR v1 IN 1 .. lines LOOP
        dummy := FILE_PUT( file, CHR(10), FALSE );
    END LOOP;
END new_line;

PROCEDURE put_line(file   IN file_type,
buffer IN VARCHAR(32768),
autoflush IN BOOLEAN DEFAULT FALSE)
AS
    dummy BOOLEAN;
BEGIN
    dummy := FILE_PUT( file, buffer||CHR(10), autoflush );
END put_line;

PROCEDURE fflush(file IN file_type)
AS
    dummy BOOLEAN;
BEGIN
    dummy := FILE_FLUSH( file );
END fflush;

PROCEDURE fremove(location IN VARCHAR(40),
                  filename IN VARCHAR(256))
AS
    dummy BOOLEAN;
BEGIN
dummy := FILE_REMOVE( location, filename );
END fremove;

PROCEDURE fcopy(src_location  IN VARCHAR(40),
                src_filename  IN VARCHAR(256),
                dest_location IN VARCHAR(40),
                dest_filename IN VARCHAR(256),
                start_line    IN INTEGER DEFAULT 1,
                end_line      IN INTEGER DEFAULT NULL)
as
    dummy BOOLEAN;
BEGIN
    dummy := FILE_COPY( src_location, src_filename, dest_location, dest_filename, start_line, end_line );
END fcopy;

PROCEDURE frename(src_location   IN VARCHAR(40),
                  src_filename   IN VARCHAR(256),
                  dest_location  IN VARCHAR(40),
                  dest_filename  IN VARCHAR(256),
                  overwrite      IN BOOLEAN DEFAULT FALSE)
AS
    dummy BOOLEAN;
BEGIN
    dummy := FILE_RENAME( src_location, src_filename, dest_location, dest_filename, overwrite );
END frename;

end utl_file;
/

