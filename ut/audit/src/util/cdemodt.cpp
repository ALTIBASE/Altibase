/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifdef RCSID
static char *RCSid =
   "$Header$ ";
#endif /* RCSID */

#ifndef CDEMOXML
#define CDEMOXML

/*------------------------------------------------------------------------
 * Include Files
 */

#ifndef STDIO
#include <stdio.h>
#endif

#ifndef STDLIB
#include <stdlib.h>
#endif

#ifndef STRING
#include <string.h>
#endif

#ifndef OCI_ORACLE
#include <oci.h>
#endif

/*--------------------- Public Constants and Variables ----------------------*/

#include <utdb.h> 
#include <utOra.h>
   
/* constants */
#define MAXDATELEN 64
#define COLNUM 4

/* database login information */
static OraText *user=(OraText *)"SCOTT";
static OraText *password=(OraText *)"TIGER";

/* OCI Handles and Variables */
static OCIEnv        *envhp;
static OCIServer     *srvhp;
static OCISvcCtx     *svchp;
static OCIError      *errhp;
static OCISession    *authp;
static OCIBind       *bndhp[COLNUM] = { 0,0,0,0};
static OCIDefine     *defhp[COLNUM] = { 0,0,0,0};
static OCIStmt       *stmthp = (OCIStmt *) 0;

/* Misellaneous */
static boolean        ret = FALSE;
static boolean        tab_exists = FALSE;
static boolean        proc_exists = FALSE;
static ub2            sqlt[COLNUM -1] = {SQLT_TIMESTAMP_TZ, SQLT_TIMESTAMP_TZ, SQLT_TIMESTAMP_TZ};
static ub4            ocit[COLNUM -1] = {OCI_DTYPE_TIMESTAMP_TZ, OCI_DTYPE_TIMESTAMP_TZ, OCI_DTYPE_TIMESTAMP_TZ};

/*----------------- End of Constants and Variables -----------------*/

/*--------------------- Functions Declaration --------------------*/

static sb4 add_interval( OraText *dt, OraText *interval, OraText *fmt, ub4 type1, ub4 type2 );
static sb4 alloc_desc( OCIDateTime *dvar[], sword size );
static sb4 chk_err( OCIError *errhp, boolean ec, sb2 linenum, OraText *comment );
static void cleanup( void );
static sb4 connect_server( OraText *cstring );
static sb4 conv_dt( OraText *str1, ub4 type1, ub4 type2, OraText *fmt1,
   OraText *fmt2, OraText *name1, OraText *name2 );
static void disconnect_server( void );
static sb4 exe_sql( OraText *stmt );
static sb4 free_desc( OCIDateTime *dvar[], sword size);
static sb4 init_env_handle( void );
static sb4 init_sql( void );
static sb4 insert_dt( size_t rownum );
static sb4 select_dt( size_t rownum );
static sb4 set_sdtz();
static sb4 tzds_1();
static sb4 tzds_2();

/*--------------------- End of Functions Declaration --------------------*/

#endif


/*---------------------------Main function -----------------------------*/
int main(int argc,char *argv[])
{
 OraText cstring[50];
 size_t rownum = 1;

 strcpy((char* )cstring,"TTDB");

 if (argc > 1 ) 
 {
   strcpy((char *)cstring, argv[1] );
 }

 /* Initialize the environment and allocate handles */
 if (init_env_handle())
 {
   printf("FAILED: init_env_handle()!\n");
   return OCI_ERROR;
 }

 /* Log on to the server and begin a session */
 if (connect_server(cstring))
 {
   printf("FAILED: connect_server()!\n");
   cleanup();
   return OCI_ERROR;
 }

 /* Initialize the demo table and PL/SQL procedure */
 if (init_sql())
 {
   printf("FAILED: init_sql()\n");
   disconnect_server();
   return OCI_ERROR;
 }

 /* Set the session datetime zone */
 // set_sdtz();

 rownum = 0;
 /* Insertion via SQL statement */
 if (insert_dt(rownum))
 {
   printf("FAILED: insert_dt()!\n");
   disconnect_server();
   return OCI_ERROR;
 }

  OCITransCommit(svchp, errhp, (ub4)0);
 
 /* Selection via SQL statement */
 if (select_dt(rownum))
 {
   printf("FAILED: select_dt()!\n");
   disconnect_server();
   return OCI_ERROR;
 }

  /* Detach from a server and clean up the environment */
 disconnect_server();

 return OCI_SUCCESS;
}


/*---------------------------Subfunctions -----------------------------*/

/*--------------------------------------------------------*/
/* Add an interval to a datetime */
/*--------------------------------------------------------*/
sb4 add_interval(OraText *dt,OraText *interval,OraText *fmt,
ub4 type1, ub4 type2)
{
 OCIDateTime *var1, *result;
 OCIInterval *var2;
 OraText *str, *fmt2;
 ub4 buflen;

   /* Initialize the output string */
 str = (OraText *)malloc(MAXDATELEN);
 memset ((void *) str, '\0', MAXDATELEN);
 fmt2 = (OraText *)malloc(MAXDATELEN);
 memset ((void *) fmt2, '\0', MAXDATELEN);

   /* Allocate descriptors */
 if (ret = OCIDescriptorAlloc((dvoid *)envhp,(dvoid **)&var1, type1,
                           0, (dvoid **) 0) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDescriptorAlloc");
   return OCI_ERROR;
 }
 if (ret = OCIDescriptorAlloc((dvoid *)envhp,(dvoid **)&result, type1,
                           0, (dvoid **) 0) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDescriptorAlloc");
   return OCI_ERROR;
 }
 if (ret = OCIDescriptorAlloc((dvoid *)envhp,(dvoid **)&var2, type2,
                           0,(dvoid **)0) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDescriptorAlloc");
   return OCI_ERROR;
 }

   /* Change to datetime type format */
 if (ret = OCIDateTimeFromText((dvoid *)authp,errhp,(CONST OraText *)dt,
             (ub4) strlen((char *)dt), (CONST OraText *)fmt,
             (ub1) strlen((char *)fmt),(CONST OraText *)0,0, var1) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDateTimeFromText");
   return OCI_ERROR;
 }
   /* Change to interval type format */
 if ((ret = OCIIntervalFromText((dvoid *)authp, errhp,(CONST OraText *)interval,
              (ub4) strlen((char *)interval),
              (OCIInterval *)var2)) != OCI_SUCCESS) {
   chk_err(errhp, ret, __LINE__,(OraText *)"OCIIntervalFromText");
   return OCI_ERROR;
 }

   /* Add the interval to datetime */
 if (ret = OCIDateTimeIntervalAdd(envhp, errhp, var1, var2, result) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDateTimeIntervalAdd");
   return OCI_ERROR;
 }

 buflen = MAXDATELEN;

   /* Change the result to text format */
 strcpy((char *)fmt2,"DD-MON-RR HH.MI.SSXFF AM TZR TZD");
 if (ret = OCIDateTimeToText((dvoid *)authp,errhp,(CONST OCIDateTime *)result,
         (CONST OraText *)fmt2,(ub1)strlen((char *)fmt2),(ub1) 2,
         (CONST OraText *) NULL,
         (ub4)0, (ub4 *) &buflen, (OraText *) str) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDateTimeToText");
   return OCI_ERROR;
 }

 printf("( %s ) + ( %s ) --->\n\t%s\n\n", dt, interval, str);

   /* Free the descriptors */
 if (var1) {
   OCIDescriptorFree((dvoid *) var1, (ub4) type1);
 }
 if (result) {
   OCIDescriptorFree((dvoid *) result, (ub4) type1);
 }
 if (var2) {
   OCIDescriptorFree((dvoid *) var2, (ub4) type2);
 }
 
 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Allocate descriptors */
/*--------------------------------------------------------*/
sb4 alloc_desc(OCIDateTime *dvar[], sword size)
{
 sword i;

 /* allocate datetime and interval descriptors */
 for (i=0; i<size; i++) {
   if ((ret = OCIDescriptorAlloc(envhp,(dvoid **)&dvar[i], ocit[i],
                               0,(dvoid **)0)) != OCI_SUCCESS) {
     printf("Error With Descriptor Alloc!\n");
     return OCI_ERROR;
   }
 }
 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Error handling  */
/*--------------------------------------------------------*/
sb4 chk_err(
OCIError  *errhp,
boolean ec,
sb2 linenum,
OraText *comment)
{
  OraText  msgbuf[1024];
  sb4 errcode;
  sb4 ret = OCI_ERROR;
  
  switch(ec) {
    case OCI_SUCCESS:
        ret = OCI_SUCCESS;
      break;

    case OCI_SUCCESS_WITH_INFO:
      printf("------------------------------------------------------------\n");
      printf("Error:Line # %d: OCI_SUCCESS_WITH_INFO - %s\n",linenum,comment);
      OCIErrorGet ((dvoid *) errhp,(ub4) 1, (OraText *) NULL, &errcode,
                         msgbuf, (ub4) sizeof(msgbuf), (ub4) OCI_HTYPE_ERROR);
      printf("%s\n",msgbuf);
      ret = OCI_SUCCESS;
      break;

    case OCI_NEED_DATA:
      printf("------------------------------------------------------------\n");
      printf("Error:Line # %d: OCI_NEED_DATA - %s\n",linenum,comment);
      break;

    case OCI_NO_DATA:
      printf("------------------------------------------------------------\n");
      printf("Error:Line # %d: OCI_NO_DATA - %s\n",linenum,comment);
      break;

    case OCI_ERROR:
      printf("------------------------------------------------------------\n");
      printf("Error:Line # %d: OCI_ERROR - %s\n",linenum,comment);
      OCIErrorGet ((dvoid *) errhp,(ub4) 1, (OraText *) NULL, &errcode,
              msgbuf, (ub4) sizeof(msgbuf), (ub4) OCI_HTYPE_ERROR);
      printf("%s\n",msgbuf);
      break;

    case OCI_INVALID_HANDLE:
      printf("------------------------------------------------------------\n");
      printf("Error:Line # %d: OCI_INVALID_HANDLE - %s\n",linenum,comment);
      exit(1);

    case OCI_STILL_EXECUTING:
      printf("------------------------------------------------------------\n");
      printf("Error:Line # %d: OCI_STILL_EXECUTING - %s\n",linenum,comment);

      break;

    case OCI_CONTINUE:
      printf("Error:Line # %d: OCI_CONTINUE - %s\n",linenum,comment);
      break;

    default:
      printf("------------------------------------------------------------\n");
      printf("Error:Line # %d: Error Code %d :%s\n",linenum,ec,comment);
      break;
  }
  return ret;
}


/*--------------------------------------------------------*/
/* Free the envhp whenever there is an error  */
/*--------------------------------------------------------*/
void cleanup()
{
 if (envhp) {
    OCIHandleFree((dvoid *)envhp, OCI_HTYPE_ENV);
 }

 return;
}


/*--------------------------------------------------------*/
/* attach to server, set attributes, and begin session */
/*--------------------------------------------------------*/
sb4 connect_server(OraText* cstring)
{
 /* attach to server */
 if ((ret = OCIServerAttach( srvhp,errhp,(OraText *) cstring,
                          (sb4) strlen((char *)cstring), OCI_DEFAULT)) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIServerAttach");
   return OCI_ERROR;
 }

 /* set server attribute to service context */
 if ((ret = OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX,
                       (dvoid *) srvhp, (ub4) 0, (ub4) OCI_ATTR_SERVER, errhp))
       != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIAttrSet: OCI_HTYPE_SVCCTX");
   return OCI_ERROR;
 }

 /* set user attribute to session */
 if ((ret = OCIAttrSet ( (dvoid *) authp,OCI_HTYPE_SESSION,
                         (dvoid *) user, (ub4) strlen((char *)user),
                   (ub4) OCI_ATTR_USERNAME, (OCIError *) errhp)) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIAttrSet: OCI_ATTR_USERNAME");
   return OCI_ERROR;
 }

 /* set password attribute to session */
 if ((ret = OCIAttrSet ( (dvoid *) authp,OCI_HTYPE_SESSION,
                         (dvoid *) password, (ub4) strlen((char *)password),
                   (ub4) OCI_ATTR_PASSWORD,(OCIError *) errhp)) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIAttrSet: OCI_ATTR_PASSWORD");
   return OCI_ERROR;
 }

 /* Begin a session  */
 if ( (ret = OCISessionBegin (svchp,errhp,authp,OCI_CRED_RDBMS,
                  OCI_DEFAULT)) == OCI_SUCCESS)
   printf("Logged in!\n\n");
 else {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCISessionBegin");
   return OCI_ERROR;
 }

 /* set session attribute to service context */
 if ((ret = OCIAttrSet((dvoid *)svchp, (ub4)OCI_HTYPE_SVCCTX,
                       (dvoid *)authp, (ub4)0,(ub4)OCI_ATTR_SESSION, errhp))
       != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIAttrSet: OCI_ATTR_SESSION");
   return OCI_ERROR;
 }


 /* Drop the demo table and procedure if any */
 exe_sql((OraText *)"drop table t1");
   
 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Convert a datetime type to another */
/*--------------------------------------------------------*/
sb4 conv_dt(
OraText *str1,
ub4 type1,
ub4 type2,
OraText *fmt1,
OraText *fmt2,
OraText *name1,
OraText *name2)
{
 OCIDateTime *var1, *var2;
 OraText *str2;
 ub4 buflen;

   /* Initialize the output string */
 str2 = (OraText *)malloc(MAXDATELEN);
 memset ((void *) str2, '\0', MAXDATELEN);

   /* Allocate descriptors */
 if (ret = OCIDescriptorAlloc((dvoid *)envhp,(dvoid **)&var1, type1,
                           0, (dvoid **) 0) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDescriptorAlloc--var1");
   return OCI_ERROR;
 }
 if (ret = OCIDescriptorAlloc((dvoid *)envhp,(dvoid **)&var2, type2,
                           0,(dvoid **)0) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDescriptorAlloc--var2");
   return OCI_ERROR;
 }

   /* Change to datetime type format */
 if (ret = OCIDateTimeFromText((dvoid *)authp,errhp,(CONST OraText *)str1,
             (ub4) strlen((char *)str1), (CONST OraText *)fmt1,
             (ub1) strlen((char *)fmt1),(CONST OraText *)0,0, var1) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDateTimeFromText--var1");
   return OCI_ERROR;
 }

   /* Convert one datetime type to another */
 if (ret = OCIDateTimeConvert((dvoid *)authp,errhp,var1,var2) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDateTimeConvert");
   return OCI_ERROR;
 }

 buflen = MAXDATELEN;

   /* Change to text format */
 if (ret = OCIDateTimeToText((dvoid *)authp,errhp,(CONST OCIDateTime *)var2,
         (CONST OraText *)fmt2,(ub1)strlen((char *)fmt2),(ub1) 2,
         (CONST OraText *) NULL,
         (ub4)0, (ub4 *) &buflen, (OraText *) str2) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIDateTimeToText");
   return OCI_ERROR;
 }

 printf("%s ---> %s:\n", name1, name2);
 printf("\t%s ---> %s\n\n", str1, str2);

   /* Free the descriptors */
 if (var1) {
   OCIDescriptorFree((dvoid *) var1, (ub4) type1);
 }
 if (var2) {
   OCIDescriptorFree((dvoid *) var2, (ub4) type2);
 }
 
 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* End the session, detach server and free environmental handles. */
/*--------------------------------------------------------*/
void disconnect_server()
{
  printf("\n\nLogged off and detached from server.\n");


   /* End a session */
 if (ret = OCISessionEnd((OCISvcCtx *)svchp, (OCIError *)errhp,
                           (OCISession *)authp, (ub4) OCI_DEFAULT)) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCISessionEnd");
   cleanup();
   return;
 }

   /* Detach from the server */
 if (ret = OCIServerDetach((OCIServer *)srvhp, (OCIError *)errhp,
                             (ub4)OCI_DEFAULT)) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIServerDetach");
   cleanup();
   return;
 }

   /* Free the handles */
 if (stmthp) {
    OCIHandleFree((dvoid *)stmthp, (ub4) OCI_HTYPE_STMT);
 }
 if (authp) {
    OCIHandleFree((dvoid *)authp, (ub4) OCI_HTYPE_SESSION);
 }
 if (svchp) {
    OCIHandleFree((dvoid *)svchp, (ub4) OCI_HTYPE_SVCCTX);
 }
 if (srvhp) {
    OCIHandleFree((dvoid *)srvhp, (ub4) OCI_HTYPE_SERVER);
 }
 if (errhp) {
    OCIHandleFree((dvoid *)errhp, (ub4) OCI_HTYPE_ERROR);
 }
 if (envhp) {
    OCIHandleFree((dvoid *)envhp, (ub4) OCI_HTYPE_ENV);
 }

  return;
}


/*--------------------------------------------------------*/
/* Execute the insertion */
/*--------------------------------------------------------*/
sb4 exe_insert(OraText *insstmt, OCIDateTime *dvar[3], size_t rownum)
{
 sword i;
 size_t colc = rownum;
 ORADate * odt;   

  /* Preparation of statment handle for the SQL statement */
 if ((ret = OCIStmtPrepare(stmthp, errhp, insstmt, (ub4)strlen((char *)insstmt), OCI_NTV_SYNTAX,
                      OCI_DEFAULT)) != OCI_SUCCESS) {
    chk_err(errhp,ret,__LINE__,(OraText *)"OCIStmtPrepare");
    return OCI_ERROR;
 }

   /* Binds the input variable */
 for (i=0; i < 3; i++) {
   if ((ret = OCIBindByPos(stmthp,&bndhp[i],errhp,i+1,(dvoid *)&dvar[i],
              (sb4) sizeof(OCIDateTime *), sqlt[i], (dvoid *)0,
              (ub2 *) 0,(ub2 *) 0,(ub4) 0,(ub4 *)0,OCI_DEFAULT)) != OCI_SUCCESS) {
     chk_err(errhp,ret,__LINE__,(OraText *)"OCIBindByPos");
     return OCI_ERROR;
   }

    odt = (ORADate*) dvar[i];
    idlOS::fprintf(stderr,"before->%d/%02d/%02d %02d:%02d:%02d.%09d  %d\n",
               (SInt)odt->yy,
               (SInt)odt->mm,
               (SInt)odt->dd,

               (SInt)odt->ts.hh,
               (SInt)odt->ts.mi,
               (SInt)odt->ts.ss,
               odt->ts.fs,
               (UInt)odt->ts.tz );
         
 
 }
   if ((ret = OCIBindByPos(stmthp,&bndhp[3], errhp, 4, (dvoid *)&colc,
            (sb4) sizeof(colc), SQLT_INT, (dvoid *)0,
            (ub2 *) 0,(ub2 *) 0,(ub4) 0,(ub4 *)0,OCI_DEFAULT)) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *) "OCIBindByPos");
   return OCI_ERROR;
 }

   /* Executing the SQL statement */
 if ((ret = OCIStmtExecute(svchp, stmthp, errhp, 1, 0, (OCISnapshot *) NULL,
                           (OCISnapshot *) NULL, OCI_DEFAULT)) != OCI_SUCCESS) {
    chk_err(errhp,ret,__LINE__,(OraText *)"OCIStmtExecute");
    return OCI_ERROR;
 }
 
 printf("Values inserted\n\n");

 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Execute a sql statement */
/*--------------------------------------------------------*/
sb4 exe_sql(OraText *stmt)
{
  /* prepare statement */
  if (ret = OCIStmtPrepare(stmthp, errhp, stmt, (ub4) strlen((char *) stmt), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT))
  {
    if( chk_err(errhp,ret,__LINE__,(OraText *)"OCIStmtPrepare") != OCI_SUCCESS)
    {
        return OCI_ERROR;
    };
  }
  /* execute statement */
  if (ret = OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
                    (CONST OCISnapshot *) 0, (OCISnapshot *) 0,
                    (ub4) OCI_DEFAULT))
  {
     chk_err(errhp,ret,__LINE__,(OraText *)"OCIStmtExecute");
  }

  return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Free the descriptors */
/*--------------------------------------------------------*/
sb4 free_desc(OCIDateTime *dvar[], sword size)
{
 sword i;

   /* Free the descriptors */
 for (i=0; i<size; i++) {
   if (dvar[i]) {
     OCIDescriptorFree((dvoid *) dvar[i], (ub4) ocit[i]);
   }
 }

 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Initialize the environment and allocate handles */
/*--------------------------------------------------------*/
sb4 init_env_handle()
{
 /* Environment initialization and creation */
 if ((ret = OCIEnvCreate((OCIEnv **) &envhp, (ub4) OCI_DEFAULT | OCI_THREADED | OCI_OBJECT,
                   (dvoid *) 0, (dvoid * (*)(dvoid *,size_t)) 0,
                   (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
                   (void (*)(dvoid *, dvoid *)) 0, (size_t) 0, (dvoid **) 0))
       != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIEnvCreate");
   return OCI_ERROR;
 }

 /* allocate error handle */
 if ((ret = OCIHandleAlloc((dvoid *) envhp,(dvoid **) &errhp,OCI_HTYPE_ERROR,
                           (size_t) 0, (dvoid **) 0)) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIHandleAlloc: errhp");
   return OCI_ERROR;
 }

 /* allocate server handle */
 if ((ret = OCIHandleAlloc ( (dvoid *) envhp,(dvoid **) &srvhp,OCI_HTYPE_SERVER,
                             (size_t) 0, (dvoid **) 0)) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIHandleAlloc: srvhp");
   return OCI_ERROR;
 }

 /* allocate service context handle */
 if ((ret = OCIHandleAlloc ( (dvoid *) envhp,(dvoid **) &svchp,OCI_HTYPE_SVCCTX,
                             (size_t) 0, (dvoid **) 0)) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIHandleAlloc: svchp");
   return OCI_ERROR;
 }

 /* allocate session handle */
 if ((ret = OCIHandleAlloc((dvoid *) envhp,(dvoid **) &authp,OCI_HTYPE_SESSION,
                             (size_t) 0,(dvoid **) 0)) != OCI_SUCCESS) {
   chk_err(errhp,ret,__LINE__,(OraText *)"OCIHandleAlloc: authp");
   return OCI_ERROR;
 }

 /* allocate statement handle */
 if ((ret = OCIHandleAlloc(envhp,(dvoid**)  &stmthp,
                                  OCI_HTYPE_STMT, 0, (dvoid **)0)) != OCI_SUCCESS) {
   printf("Error in getting stmt handle\n");
   return OCI_ERROR;
 }

 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Create table and procedure for demo */
/*--------------------------------------------------------*/
sb4 init_sql()
{

  tab_exists = TRUE;
 return  chk_err(errhp,
  exe_sql( (OraText *) "CREATE TABLE T1(\
                                 I1 NUMERIC, \
                                 I2 TIMESTAMP, \
                                 I3 TIMESTAMP WITH TIME ZONE, \
                                 I4 TIMESTAMP WITH LOCAL TIME ZONE, \
                                 PRIMARY KEY (I1, I2) )" )
  ,__LINE__,(OraText *)"init_sql");
}


/*--------------------------------------------------------*/
/* Insert datetime/interval values into the table */
/*--------------------------------------------------------*/
sb4 insert_dt(size_t rownum)
{
 sword i;
 OraText *strdate[COLNUM-1];
 OraText *insstmt = (OraText *)"INSERT INTO T1(I2,I3,I4,I1)  VALUES (:1, :2, :3, :4)";
 OCIDateTime *dvar[3];
 ORADate * odt;

 /* allocate datetime and interval descriptors */
 if (alloc_desc(dvar,3)) {
   printf("FAILED: alloc_desc()!\n");
   return OCI_ERROR;
 }

 for(i = 0; i<3; i++)
 {
   odt = (ORADate*)dvar[i];

   odt->yy = 1990 + rownum;
   odt->mm = 1    + i;
   odt->dd = 1    + i;

   odt->ts.hh = 1 + i;
   odt->ts.mi = 1 + i;
   odt->ts.ss = 1 + i;
   odt->ts.fs = 100011111 * (i+1);

   odt->ts.tz = i;
 }    
 
 printf("-->Insertion via SQL statement\n\n");
   /* execute the insertion */
 if (exe_insert(insstmt, dvar, rownum)) {
   printf("FAILED: exe_insert()!\n");
   return OCI_ERROR;
 }

   /* Free the descriptors */
 if (free_desc(dvar, 3)) {
   printf("FAILED: free_desc()!\n");
   return OCI_ERROR;
 }

 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Select datetime/interval values from the table */
/*--------------------------------------------------------*/
sb4 select_dt(size_t rownum)
{
 sword i, var6;
 size_t colc = rownum;
 OraText *strdate[COLNUM-1];
 OraText *fmtdate[3];
 ub4 str_size[COLNUM-1];
 size_t resultlen; 
 OCIDateTime *dvar[3];
 ORADate     * odt;
 OraText *selstmt = (OraText *) "SELECT I2,I3,I4 FROM T1 WHERE I1 >= :1";

 /* allocate datetime and interval descriptors */
 if (alloc_desc(dvar, 3)) {
   printf("FAILED: alloc_desc()!\n");
   return OCI_ERROR;
 }

 printf("-->Selection via SQL statement\n\n");
   /* Preparation of statment handle for the SQL statement */
 if ((ret = OCIStmtPrepare(stmthp, errhp, selstmt, (ub4)strlen((char *)selstmt), OCI_NTV_SYNTAX,
                      OCI_DEFAULT)) != OCI_SUCCESS) {
    chk_err(errhp,ret,__LINE__,(OraText *)"OCIStmtPrepare");
    return OCI_ERROR;
 }

   /* associate variable colc with bind placeholder in the SQL statement */
 if ((ret = OCIBindByPos(stmthp, &bndhp[4], errhp, 1, &colc, sizeof(colc),
                      SQLT_INT,(void *)0, (ub2 *)0,(ub2 *)0, 0, (ub4 *)0, OCI_DEFAULT)) != OCI_SUCCESS) {
    chk_err(errhp,ret,__LINE__,(OraText *)"OCIBindByPos");
    return OCI_ERROR;
 }

   /* Defining the positions for the variables */
 for (i=0; i<3; i++) {
   if ((ret = OCIDefineByPos(stmthp, &defhp[i], errhp, i+1, &dvar[i], sizeof(dvar[i]),
                      sqlt[i], (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT)) != OCI_SUCCESS) {
      chk_err(errhp,ret,__LINE__,(OraText *)"OCIDefineByPos");
      return OCI_ERROR;
   }
   memset(dvar[i],0,11);
 }
 

 //////////>>>>>>>>>>>>>>>>>>>>>>>>>>>
  /* Executing the SQL statement */
 if ((ret = OCIStmtExecute(svchp, stmthp, errhp, 1, 0, (OCISnapshot *) NULL,
                           (OCISnapshot *) NULL, OCI_DEFAULT)) != OCI_SUCCESS) {
    chk_err(errhp,ret,__LINE__,(OraText *)"OCIStmtExecute");
    return OCI_ERROR;
 }
 
 for (i=0; i < 3; i++) 
 {
  odt = (ORADate*) dvar[i];
  idlOS::fprintf(stderr,"I(%i)->%d/%02d/%02d %02d:%02d:%02d.%09d  %d\n",i,
               (SInt)odt->yy,
               (SInt)odt->mm,
               (SInt)odt->dd,

               (SInt)odt->ts.hh,
               (SInt)odt->ts.mi,
               (SInt)odt->ts.ss,
               odt->ts.fs,
               (UInt)odt->ts.tz );
 }

   /* Free the descriptors */
 if (free_desc(dvar, 3)) {
   printf("FAILED: free_desc()!\n");
   return OCI_ERROR;
 }

 return OCI_SUCCESS;
}


/*--------------------------------------------------------*/
/* Set session datetime zone */
/*--------------------------------------------------------*/
sb4 set_sdtz()
{
 OraText *altstmt=(OraText *)"ALTER SESSION SET TIME_ZONE='US/Central'";
 OraText *selstmt=(OraText *)"SELECT SESSIONTIMEZONE FROM DUAL";
 OraText *str;

   /* Initialize strings */
 str = (OraText *)malloc(MAXDATELEN);
 memset ((void *) str, '\0', MAXDATELEN);

   /* Alter session time zone */
 if (exe_sql(altstmt)) {
   printf("FAILED: exe_sql()!\n");
   return OCI_ERROR;
 }

    /* Preparation of statment handle for the select SQL statement */
 if ((ret = OCIStmtPrepare(stmthp, errhp, selstmt, (ub4) strlen((char *)selstmt),
                OCI_NTV_SYNTAX, OCI_DEFAULT)) != OCI_SUCCESS) {
    chk_err(errhp,ret,__LINE__,(OraText *)"OCIStmtPrepare");
    return OCI_ERROR;
 }

   /* Defining the position for the variable */
 if ((ret = OCIDefineByPos(stmthp, &defhp[0], errhp, 1, str, MAXDATELEN,
                      SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT)) != OCI_SUCCESS) {
    chk_err(errhp,ret,__LINE__,(OraText *)"OCIDefineByPos");
    return OCI_ERROR;
 }

    /* Executing the SQL statement */
 if ((ret = OCIStmtExecute(svchp, stmthp, errhp, 1, 0, (OCISnapshot *) NULL,
                           (OCISnapshot *) NULL, OCI_DEFAULT)) != OCI_SUCCESS) {
    chk_err(errhp,ret,__LINE__,(OraText *)"OCIStmtExecute");
    return OCI_ERROR;
 }

 printf("The session time zone is set to %s\n\n", str);

 return OCI_SUCCESS;
}

/* end of file cdemodt.c */
