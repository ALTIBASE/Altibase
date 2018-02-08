------------------------------------------------------------------
CONNECT sys/manager;

-- create user for SPATIAL_META
CREATE USER sto IDENTIFIED BY sto;

GRANT CREATE PUBLIC SYNONYM TO sto;
GRANT DROP PUBLIC SYNONYM TO sto;

------------------------------------------------------------------
CONNECT sto/sto;

-- create table
CREATE TABLE sto_spatial_ref_sys (
	 srid integer not null,
	 auth_name varchar(256),
	 auth_srid integer, 
	 srtext varchar(2048),
         proj4text varchar(2048)
);


CREATE TABLE sto_geometry_columns (
	f_table_schema varchar(40) not null,
	f_table_name varchar(40) not null,
	f_geometry_column varchar(40) not null,
	coord_dimension integer not null,
	srid integer not null
);

------------------------------------------------------------------

-- create PK,FK and CONSTRAINT

ALTER TABLE sto_spatial_ref_sys ADD CONSTRAINT sto_spatial_ref_sys_pk PRIMARY KEY (srid);

ALTER TABLE sto_geometry_columns ADD CONSTRAINT sto_geometry_columns_pk 
PRIMARY KEY (f_table_schema, f_table_name, f_geometry_column);

ALTER TABLE sto_geometry_columns ADD CONSTRAINT sto_geometry_columns_fk 
FOREIGN KEY (srid) REFERENCES sto_spatial_ref_sys (srid);

------------------------------------------------------------------

DROP PUBLIC SYNONYM spatial_ref_sys;
DROP PUBLIC SYNONYM geometry_columns;

-- grant privilege

GRANT SELECT ON sto_spatial_ref_sys TO PUBLIC;
GRANT SELECT, INSERT, DELETE ON sto_geometry_columns TO PUBLIC;

CREATE PUBLIC SYNONYM spatial_ref_sys FOR sto_spatial_ref_sys;
CREATE PUBLIC SYNONYM geometry_columns FOR sto_geometry_columns;

------------------------------------------------------------------

-- insert SRID

-- EPSG
INSERT INTO sto_spatial_ref_sys VALUES ( 1, 'EPSG', 32651, 'PROJCS [ "WGS 84 / UTM zone 51N", GEOGCS [ "WGS_1984", DATUM [ "WGS_1984", SPHEROID [ "WGS_1984", 6378137, 298.257223563]], PRIMEM [ "Greenwich", 0.000000], UNIT [ "Decimal Degree", 0.01745329251994328]], PROJECTION [ "Transverse_Mercator"], PARAMETER [ "Latitude_Of_Origin", 0], PARAMETER [ "Central_Meridian", 123], PARAMETER [ "Scale_Factor", 0.9996], PARAMETER [ "False_Easting", 500000], PARAMETER [ "False_Northing", 0], UNIT [ "Meter", 1]]', 'Proj4js.defs["EPSG:32651"] = "+proj=utm +zone=51 +ellps=WGS84 +datum=WGS84 +units=m +no_defs";' );

INSERT INTO sto_spatial_ref_sys VALUES ( 2, 'EPSG', 32652, 'PROJCS [ "WGS 84 / UTM zone 52N", GEOGCS [ "WGS_1984", DATUM [ "WGS_1984", SPHEROID [ "WGS_1984", 6378137, 298.257223563]], PRIMEM [ "Greenwich", 0.000000], UNIT [ "Decimal Degree", 0.01745329251994328]], PROJECTION [ "Transverse_Mercator"], PARAMETER [ "Latitude_Of_Origin", 0], PARAMETER [ "Central_Meridian", 129], PARAMETER [ "Scale_Factor", 0.9996], PARAMETER [ "False_Easting", 500000], PARAMETER [ "False_Northing", 0], UNIT [ "Meter", 1]]', 'Proj4js.defs["EPSG:32652"] = "+proj=utm +zone=52 +ellps=WGS84 +datum=WGS84 +units=m +no_defs";' );

INSERT INTO sto_spatial_ref_sys VALUES ( 3, 'EPSG', 2097, 'PROJCS [ "Korean 1985 / Korea Central Belt", GEOGCS [ "Korean 1985", DATUM [ "Korean Datum 1985 (EPSG ID 6162)", SPHEROID [ "Bessel_1841", 6377397.155, 299.1528128], TOWGS84 [ 146,-507,-687]], PRIMEM [ "Greenwich", 0.000000 ], UNIT [ "Decimal Degree", 0.01745329251994328]], PROJECTION [ "Transverse_Mercator"], PARAMETER [ "Latitude_Of_Origin", 38], PARAMETER [ "Central_Meridian", 127.00288889], PARAMETER [ "Scale_Factor", 1], PARAMETER [ "False_Easting", 200000], PARAMETER [ "False_Northing", 500000], UNIT ["Meter", 1]]', 'Proj4js.defs["EPSG:2097"] = "+proj=tmerc +lat_0=38 +lon_0=127 +k=1 +x_0=200000 +y_0=500000 +ellps=bessel +units=m +no_defs";' );

INSERT INTO sto_spatial_ref_sys VALUES ( 4, 'EPSG', 2096, 'PROJCS [ "Korean 1985 / Korea East Belt", GEOGCS [ "Korean 1985", DATUM [ "Korean Datum 1985 (EPSG ID 6162)", SPHEROID [ "Bessel_1841", 6377397.155, 299.1528128], TOWGS84 [ 146,-507,-687]], PRIMEM [ "Greenwich", 0.000000 ], UNIT [ "Decimal Degree", 0.01745329251994328]], PROJECTION [ "Transverse_Mercator"], PARAMETER [ "Latitude_Of_Origin", 38], PARAMETER [ "Central_Meridian", 129.00288889], PARAMETER [ "Scale_Factor", 1], PARAMETER [ "False_Easting", 200000], PARAMETER [ "False_Northing", 500000], UNIT [ "Meter", 1]]', 'Proj4js.defs["EPSG:2096"] = "+proj=tmerc +lat_0=38 +lon_0=129 +k=1 +x_0=200000 +y_0=500000 +ellps=bessel +units=m +no_defs";' );

INSERT INTO sto_spatial_ref_sys VALUES ( 5, 'EPSG', 2098, 'PROJCS [ "Korean 1985 / Korea West Belt", GEOGCS [ "Korean 1985", DATUM [ "Korean Datum 1985 (EPSG ID 6162)", SPHEROID [ "Bessel_1841", 6377397.155, 299.1528128], TOWGS84 [ 146,-507,-687]], PRIMEM [ "Greenwich", 0.000000 ], UNIT [ "Decimal Degree", 0.01745329251994328]], PROJECTION [ "Transverse_Mercator"], PARAMETER [ "Latitude_Of_Origin", 38], PARAMETER [ "Central_Meridian", 125.00288889], PARAMETER [ "Scale_Factor", 1], PARAMETER [ "False_Easting", 200000], PARAMETER [ "False_Northing", 500000], UNIT [ "Meter", 1]]', 'Proj4js.defs["EPSG:2098"] = "+proj=tmerc +lat_0=38 +lon_0=125 +k=1 +x_0=200000 +y_0=500000 +ellps=bessel +units=m +no_defs";' );

--OTHERS 

INSERT INTO sto_spatial_ref_sys VALUES ( 100, NULL, NULL, 'PROJCS [ "Korean 1985 / Korea Jeju Belt", GEOGCS [ "Korean 1985", DATUM [ "Korean Datum 1985 (EPSG ID 6162)", SPHEROID [ "Bessel_1841", 6377397.155, 299.1528128], TOWGS84 [ 146,-507,-687]], PRIMEM [ "Greenwich", 0.000000 ], UNIT [ "Decimal Degree", 0.01745329251994328]], PROJECTION [ "Transverse_Mercator"], PARAMETER [ "Latitude_Of_Origin", 38], PARAMETER [ "Central_Meridian", 127.00288889], PARAMETER [ "Scale_Factor", 1], PARAMETER [ "False_Easting", 200000], PARAMETER [ "False_Northing", 550000], UNIT [ "Meter", 1]]', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 101, NULL, NULL, 'PROJCS [ "Korean 1985 / Korea EastSea Belt", GEOGCS [ "Korean 1985", DATUM [ "Korean Datum 1985 (EPSG ID 6162)", SPHEROID ["Bessel_1841", 6377397.155, 299.1528128], TOWGS84 [ 146,-507,-687]], PRIMEM [ "Greenwich", 0.000000 ], UNIT [ "Decimal Degree", 0.01745329251994328]], PROJECTION [ "Transverse_Mercator"], PARAMETER [ "Latitude_Of_Origin", 38], PARAMETER [ "Central_Meridian", 131.00288889], PARAMETER [ "Scale_Factor", 1], PARAMETER [ "False_Easting", 200000], PARAMETER [ "False_Northing", 500000], UNIT [ "Meter", 1]]', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 102, NULL, NULL, 'PROJCS [ "GRS80 / Korea Central Belt", GEOGCS [ "Korea GRS80", DATUM [ "Korean Datum GRS80", SPHEROID [ "GRS_80", 6378137, 298.257222101]], PRIMEM [ "Greenwich", 0.000000 ], UNIT [ "Decimal Degree", 0.01745329251994328]], PROJECTION [ "Transverse_Mercator"], PARAMETER [ "Latitude_Of_Origin", 38], PARAMETER [ "Central_Meridian", 127], PARAMETER [ "Scale_Factor", 1], PARAMETER [ "False_Easting", 200000], PARAMETER [ "False_Northing", 500000], UNIT [ "Meter", 1]]', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 103, NULL, NULL, 'PROJCS [ "GRS80 / Korea East Belt", GEOGCS [ "Korea GRS80", DATUM ["Korean Datum GRS80", SPHEROID ["GRS_80", 6378137, 298.257222101]], PRIMEM [ "Greenwich", 0.000000 ], UNIT ["Decimal Degree", 0.01745329251994328]], PROJECTION ["Transverse_Mercator"], PARAMETER ["Latitude_Of_Origin", 38], PARAMETER ["Central_Meridian", 129], PARAMETER ["Scale_Factor", 1], PARAMETER ["False_Easting", 200000], PARAMETER ["False_Northing", 500000], UNIT ["Meter", 1]]', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 104, NULL, NULL, 'GEOGCS["Korea /  GRS80", DATUM ["Korean Datum GRS80", SPHEROID ["GRS_80", 6378137, 298.257222101]], PRIMEM [ "Greenwich", 0.000000 ], UNIT ["Decimal Degree", 0.01745329251994328]], PROJECTION ["Transverse_Mercator"], PARAMETER ["Latitude_Of_Origin", 38], PARAMETER ["Central_Meridian", 125], PARAMETER ["Scale_Factor", 1], PARAMETER ["False_Easting", 200000], PARAMETER ["False_Northing", 500000], UNIT ["Meter", 1]] ', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 105, NULL, NULL, 'PROJCS["GRS80 / Korea Jeju Belt", GEOGCS [ "Korea GRS80", DATUM ["Korean Datum GRS80", SPHEROID ["GRS_80", 6378137, 298.257222101]], PRIMEM [ "Greenwich", 0.000000 ], UNIT ["Decimal Degree", 0.01745329251994328]], PROJECTION ["Transverse_Mercator"], PARAMETER ["Latitude_Of_Origin", 38], PARAMETER ["Central_Meridian", 127], PARAMETER ["Scale_Factor", 1], PARAMETER ["False_Easting", 200000], PARAMETER ["False_Northing", 550000], UNIT ["Meter", 1]]', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 106, NULL, NULL, 'PROJCS["GRS80 / Korea EastSea Belt", GEOGCS [ "Korea GRS80", DATUM ["Korean Datum GRS80", SPHEROID ["GRS_80", 6378137, 298.257222101]], PRIMEM [ "Greenwich", 0.000000 ], UNIT ["Decimal Degree", 0.01745329251994328]], PROJECTION ["Transverse_Mercator"], PARAMETER ["Latitude_Of_Origin", 38], PARAMETER ["Central_Meridian", 131], PARAMETER ["Scale_Factor", 1], PARAMETER ["False_Easting", 200000], PARAMETER ["False_Northing", 500000], UNIT ["Meter", 1]]', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 107, NULL, NULL, 'PROJCS["GRS80 / UTM-K", GEOGCS [ "Korean GRS80", DATUM ["Korean Datum GRS80", SPHEROID ["GRS_80", 6378137, 298.257222101]], PRIMEM [ "Greenwich", 0.000000 ], UNIT ["Decimal Degree", 0.01745329251994328]], PROJECTION ["Transverse_Mercator"], PARAMETER ["Latitude_Of_Origin", 38], PARAMETER ["Central_Meridian", 127.5], PARAMETER ["Scale_Factor", 0.9996], PARAMETER ["False_Easting", 2000000], PARAMETER ["False_Northing", 1000000], UNIT ["Meter", 1]]', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 108, NULL, NULL, 'GEOGCS["Longitude / Latitude (BESSEL 1841)", DATUM ["Bessel_1841", SPHEROID ["Bessel_1841", 6377397.155, 299.1528128],TOWGS84[146,-507,-687]], PRIMEM [ "Greenwich", 0.000000 ], UNIT ["Decimal Degree", 0.01745329251994330]]
GEOGCS["Longitude / Latitude (WGS 84)", DATUM ["WGS_1984", SPHEROID ["WGS_1984", 6378137, 298.257223563]], PRIMEM [ "Greenwich", 0.000000 ], UNIT ["Decimal Degree", 0.01745329251994330]],', NULL );

INSERT INTO sto_spatial_ref_sys VALUES ( 109, NULL, NULL, 'GEOGCS["Longitude / Latitude (GRS80)", DATUM ["Korean Datum GRS80", SPHEROID ["GRS_80", 6378137, 298.257222101]], PRIMEM [ "Greenwich", 0.000000 ], UNIT ["Decimal Degree", 0.01745329251994328]]', NULL);


-- if you want add your srid please write here ---

-- insert into sto_spatial_ref_sys value ( id , auth_name , auth_code , srtext, proj4text);




--######################################################
--# Add Geometry Column to Meta Table
--#
--# SchemaName   :     User Name
--# TableName    :     Table Name
--# ColumnName   :     Geometry Column Name
--# SRID         :     Spatial Reference System ID
--######################################################



CREATE OR REPLACE PROCEDURE StoAddGeometryColumns( SchemaName VARCHAR(40), TableName VARCHAR(40), ColumnName VARCHAR(40), srid INTEGER )
AS
	type_name VARCHAR(40);
BEGIN	
	
	SELECT (SELECT type_name FROM v$datatype WHERE data_type = c.data_type) INTO type_name
	FROM system_.sys_users_ u, system_.sys_tables_ t, system_.sys_columns_ c
	WHERE u.user_id = t.user_id AND
  		t.table_id = c.table_id AND
		u.user_name = SchemaName AND
		t.table_name = TableName AND
		c.column_name = ColumnName
	limit 1;

	IF type_name <> 'GEOMETRY' THEN
		RAISE_APPLICATION_ERROR(990999,'Only geometry column can be added.');
	END IF;

	INSERT INTO sto.sto_geometry_columns VALUES ( SchemaName, TableName, ColumnName, 2, srid );
EXCEPTION
	WHEN OTHERS THEN
		IF SQLCODE = 200823 THEN
			RAISE_APPLICATION_ERROR(990998,'This value of SRID is out of range.');
		ELSEIF SQLCODE = 201066 THEN
			RAISE_APPLICATION_ERROR(990997,'It is unable to add nonexistent column.');
		ELSEIF SQLCODE = 69720 THEN
			RAISE_APPLICATION_ERROR(990996,'This column is already added.');
		ELSE
			RAISE;
		END IF;
END;	  
/

--######################################################
--# Drop Geometry Column to Meta Table
--#
--# SchemaName   :     User Name
--# TableName    :     Table Name
--# ColumnName   :     Geometry Column Name
--######################################################


CREATE OR REPLACE PROCEDURE StoDropGeometryColumns( SchemaName VARCHAR(40), TableName VARCHAR(40), ColumnName VARCHAR(40) )
AS
	type_name VARCHAR(40);
BEGIN

	SELECT (SELECT type_name FROM v$datatype WHERE data_type = c.data_type) INTO type_name
        FROM system_.sys_users_ u, system_.sys_tables_ t, system_.sys_columns_ c
        WHERE u.user_id = t.user_id AND
                t.table_id = c.table_id AND
                u.user_name = SchemaName AND
                t.table_name = TableName AND
                c.column_name = ColumnName
        limit 1;

        IF type_name <> 'GEOMETRY' THEN
                RAISE_APPLICATION_ERROR(990995,'This column is not geometry column.');
        END IF;

	DELETE FROM sto.sto_geometry_columns 
	WHERE  f_table_schema = SchemaName AND 
                      f_table_name = TableName AND
                      f_geometry_column = ColumnName;
EXCEPTION
        WHEN OTHERS THEN
                IF SQLCODE = 201066 THEN
                        RAISE_APPLICATION_ERROR(990997,'It is unable to drop nonexistent column.');
                ELSE
                        RAISE;
                END IF;
END;      
/


--######################################################
--# Get Spatial Reference System ID Function
--#
--# SchemaName   :     User Name
--# TableName    :     Table Name
--# ColumnName   :     Geometry Column Name
--######################################################


CREATE OR REPLACE FUNCTION StoSRID( SchemaName VARCHAR(40), TableName VARCHAR(40), ColumnName VARCHAR(40) )
RETURN INTEGER
AS
	type_name VARCHAR(40);
        srid      INTEGER;
BEGIN

	SELECT (SELECT type_name FROM v$datatype WHERE data_type = c.data_type) INTO type_name
        FROM system_.sys_users_ u, system_.sys_tables_ t, system_.sys_columns_ c
        WHERE u.user_id = t.user_id AND
                t.table_id = c.table_id AND
                u.user_name = SchemaName AND
                t.table_name = TableName AND
                c.column_name = ColumnName
        limit 1;

        IF type_name <> 'GEOMETRY' THEN
                RAISE_APPLICATION_ERROR(990995,'This column is not geometry column.');
        END IF;

        SELECT srid INTO srid
	FROM sto.sto_geometry_columns 
	WHERE  f_table_schema = SchemaName AND 
                      f_table_name = TableName AND
                      f_geometry_column = ColumnName;

        RETURN srid;
EXCEPTION
        WHEN OTHERS THEN
                IF SQLCODE = 201066 THEN
                        RAISE_APPLICATION_ERROR(990997,'It is unable to get nonexistent column.');
                ELSE
                        RAISE;
                END IF;
END;      
/

----------------------------------------------------------------
-- Drop synonym

DROP PUBLIC SYNONYM AddGeometryColumns;
DROP PUBLIC SYNONYM DropGeometryColumns;
DROP PUBLIC SYNONYM SRID;

-- grant privilege

GRANT EXECUTE ON StoAddGeometryColumns TO PUBLIC;
GRANT EXECUTE ON StoDropGeometryColumns TO PUBLIC;
GRANT EXECUTE ON StoSRID TO PUBLIC;

-- create synonym

CREATE PUBLIC SYNONYM AddGeometryColumns FOR StoAddGeometryColumns;
CREATE PUBLIC SYNONYM DropGeometryColumns FOR StoDropGeometryColumns;
CREATE PUBLIC SYNONYM SRID FOR StoSRID;

----------------------------------------------------------------

