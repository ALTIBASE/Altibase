
--############################
--  MMDBUSR.MMDB_ADDRESS  
--############################
drop table MMDB_ADDRESS;
create table MMDB_ADDRESS(
MEMBERNO NUMERIC(10) not null,
HOMEZIPCODE CHAR(6),
HOMEADDR1 VARCHAR(50),
HOMEADDR2 VARCHAR(50))
 tablespace SYS_TBS_MEMORY;

--############################
--  MMDBUSR.MMDB_CASE  
--############################
drop table MMDB_CASE;
create table MMDB_CASE(
CASEID NUMERIC(5) not null,
CASECODE CHAR(4) not null,
INPUT CHAR(1) not null,
OUTPUT CHAR(1) not null,
FIELD CHAR(15) not null)
 tablespace SYS_TBS_MEMORY;

--############################
--  MMDBUSR.MMDB_INIT_URL  
--############################
drop table MMDB_INIT_URL;
create table MMDB_INIT_URL(
URL_CODE CHAR(2) not null,
URL VARCHAR(255) VARIABLE ,
URL_DESC VARCHAR(255) VARIABLE ,
INPUT_DTIME DATE)
 tablespace SYS_TBS_MEMORY;

--############################
--  MMDBUSR.MMDB_MASTER  
--############################
drop table MMDB_MASTER;
create table MMDB_MASTER(
MEMBERID CHAR(12) not null,
MEMBERNO NUMERIC(10) not null,
CTN CHAR(12),
PWD CHAR(12),
PERS_ID CHAR(13),
REAL_PERS_ID CHAR(13),
PERS_NAME CHAR(60),
BAN NUMERIC(9),
SOC CHAR(9),
SID CHAR(40),
MBOX_PATH CHAR(20),
MTYPE CHAR(4),
SITEREG CHAR(70),
MUSICREG CHAR(30),
USERINFO CHAR(30),
AD_PWD CHAR(4),
PHONEINFO1 CHAR(6),
PHONEINFO2 CHAR(20),
INIT_URL CHAR(2),
UNIT_MDL CHAR(15),
IMSI CHAR(15),
POT_ROUTE CHAR(3),
POT_FLAG CHAR(1),
REGDATE DATE,
INTIME DATE,
DISABLED VARCHAR(12) NOT NULL,
MMDB_COMMITTIME DATE)
 tablespace SYS_TBS_MEMORY;

--############################
--  MMDBUSR.MMDB_SVC  
--############################
drop table MMDB_SVC;
create table MMDB_SVC(
SEQ NUMERIC(5) not null,
SVC_ID CHAR(22) not null,
SVC_PW CHAR(64) not null,
HOST_IP CHAR(15) not null,
CASECODE CHAR(4) not null)
 tablespace SYS_TBS_MEMORY;
