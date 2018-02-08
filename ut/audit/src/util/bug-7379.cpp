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
 
/*******************************************************************************
 * $Id: bug-7379.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include "uto.h"

#include <utAtb.h>

IDE_RC schema(Connection*,SChar*, SChar*, SChar*, UInt);

static newExtConnection  newConn = NULL;
Connection * newOraConnection();    

int main(int argc, char **argv)
{
 UInt count = ( argc > 1 )?idlOS::atoi(argv[1]):15;
 Connection * con = new utAtbConnection();
 IDE_TEST( schema(con,"SYS","MANAGER","DSN=e450;PORT_NO=20545;NLS_USE=US7ASCII",count
           ) != IDE_SUCCESS );
 return 0;
 IDE_EXCEPTION_END;
 return 1;  
}


IDE_RC schema(Connection * con,SChar *name, SChar* passwd, SChar* dsn, UInt count)
{
 union
 {
  UChar   hss_bytes[254];
 };
 UChar   *p; 

 Query         *sQueryI;
 UInt             sz, i;

 idlOS::memset(hss_bytes,0x11,sizeof(hss_bytes) );

 IDE_TEST( con == NULL );
 
 IDE_TEST( con->initialize() != IDE_SUCCESS );
 IDE_TEST( con->connect(name,passwd,dsn ) != IDE_SUCCESS) ;
 
           con->execute(  "DROP TABLE T1 " );
 IDE_TEST( con->execute("CREATE TABLE T1("
  "MDN         HSS_BYTES(9) PRIMARY KEY,"
  "MSID        HSS_BYTES(9) ,"
  "ESN         HSS_BYTES(4) ,"
  "MS_STS      HSS_BYTES(1) ,"
  "TERM_CAPA   HSS_BYTES(1) ,"
  "CRTE_DATE   HSS_BYTES(4) ,"
  "ORIG_IND    HSS_BYTES(1) ,"
  "TERM_CODE   HSS_BYTES(1) ,"
  "AUTH_DENY   HSS_BYTES(1) ,"
  "AUTH_PERD   HSS_BYTES(2) ,"
  "DENIED_AUTH HSS_BYTES(2) ,"
  "TRC_TYPE    HSS_BYTES(1) ,"
  "TRC_PERD    HSS_BYTES(2) ,"
  "ZONE_CLASS  HSS_BYTES(1) ,"
  "AUTH_CAPA   HSS_BYTES(1) ,"
  "AC_ID       HSS_BYTES(2) ,"
  "MSCID       HSS_BYTES(3) ,"
  "PCSSN       HSS_BYTES(5) ,"
  "LAI         HSS_BYTES(2) ,"
  "TRAN_CAPA   HSS_BYTES(2) ,"
  "SYS_CAPA    HSS_BYTES(1) ,"
  "SMS_ADDR    HSS_BYTES(4) ,"
  "SSVC1       HSS_BYTES(1) ,"
  "SSVC2       HSS_BYTES(1) ,"
  "SSVC3       HSS_BYTES(1) ,"
  "SSVC4       HSS_BYTES(1) ,"
  "SSVC5       HSS_BYTES(1) ,"
  "SSVC6       HSS_BYTES(1) ,"
  "SSVC7       HSS_BYTES(1) ,"
  "SSVC8       HSS_BYTES(1) ,"
  "SSVC9       HSS_BYTES(1) ,"
  "SSVC10      HSS_BYTES(1) ,"
  "SSVC11      HSS_BYTES(1) ,"
  "SSVC12      HSS_BYTES(1) ,"
  "SSVC13      HSS_BYTES(1) ,"
  "SSVC14      HSS_BYTES(1) ,"
  "SSVC15      HSS_BYTES(1) ,"
  "SSVC16      HSS_BYTES(1) ,"
  "SSVC17      HSS_BYTES(1) ,"
  "SSVC18      HSS_BYTES(1) ,"
  "SSVC19      HSS_BYTES(1) ,"
  "SSVC20      HSS_BYTES(1) ,"
  "SSVC21      HSS_BYTES(1) ,"
  "SSVC22      HSS_BYTES(1) ,"
  "SSVC23      HSS_BYTES(1) ,"
  "SSVC24      HSS_BYTES(1) ,"
  "SSVC25      HSS_BYTES(1) ,"
  "SSVC26      HSS_BYTES(1) ,"
  "SSVC27      HSS_BYTES(1) ,"
  "SSVC28      HSS_BYTES(1) ,"
  "CNIP_TYPE   HSS_BYTES(1) ,"
  "TI          HSS_BYTES(1) ,"
  "CFU_NUM     HSS_BYTES(12),"
  "CFB_NUM     HSS_BYTES(12),"
  "CFNA_NUM    HSS_BYTES(12),"
  "CFD_NUM     HSS_BYTES(12),"
  "CC_VALUE    HSS_BYTES(1) ,"
  "VMS_ID      HSS_BYTES(2) ,"
  "MWN_TYPE    HSS_BYTES(1) ,"
  "PCA_PSWD    HSS_BYTES(2) ,"
  "PCA_NUM     HSS_BYTES(11),"
  "SCA_NUM     HSS_BYTES(11),"
  "PIN         HSS_BYTES(2) ,"
  "SPINI_TRG   HSS_BYTES(4) ,"
  "SMS_ID      HSS_BYTES(2) ,"
  "SMS_NOTIND  HSS_BYTES(1) ,"
  "SMS_MWI     HSS_BYTES(1) ,"
  "SMS_ORIG    HSS_BYTES(1) ,"
  "SMS_TERM    HSS_BYTES(1) ,"
  "PACA_IND    HSS_BYTES(1) ,"
  "PL_IND      HSS_BYTES(1) ,"
  "VAD_ID      HSS_BYTES(2) ,"
  "PPS_ID      HSS_BYTES(1) ,"
  "RCS_ID      HSS_BYTES(2) ,"
  "WIN_ID      HSS_BYTES(1) ,"
  "MCS_ID      HSS_BYTES(2) ,"
  "XRS_ID      HSS_BYTES(2) ,"
  "CMS_ID      HSS_BYTES(2) ,"
  "CIS_ID      HSS_BYTES(2) ,"
  "LAST_CALL   HSS_BYTES(13),"
  "SCA_ID      HSS_BYTES(4) ,"
  "FA_PDN      HSS_BYTES(5) ,"
  "MAH_PDN     HSS_BYTES(5) ,"
  "SCS_ID      HSS_BYTES(2) ,"
  "CNFN        HSS_BYTES(12))"
   ) != IDE_SUCCESS);

 
 sQueryI = con->query();
 IDE_TEST(
 sQueryI->prepare(
 "INSERT INTO T1 "
  "( MDN, MSID, ESN, MS_STS, TERM_CAPA, CRTE_DATE, ORIG_IND  , TERM_CODE, AUTH_DENY,"
    "AUTH_PERD, DENIED_AUTH, TRC_TYPE , TRC_PERD , ZONE_CLASS, AUTH_CAPA, AC_ID, MSCID,"
    "PCSSN, LAI, TRAN_CAPA, SYS_CAPA, SMS_ADDR, SSVC1, SSVC2, SSVC3, SSVC4, SSVC5, SSVC6, SSVC7,"
    "SSVC8, SSVC9, SSVC10, SSVC11, SSVC12, SSVC13, SSVC14, SSVC15, SSVC16, SSVC17, SSVC18, SSVC19,"
    "SSVC20, SSVC21, SSVC22, SSVC23, SSVC24, SSVC25, SSVC26, SSVC27, SSVC28, CNIP_TYPE, TI, CFU_NUM,"
    "CFB_NUM, CFNA_NUM, CFD_NUM, CC_VALUE, VMS_ID, MWN_TYPE, PCA_PSWD, PCA_NUM, SCA_NUM, PIN, SPINI_TRG,"
    "SMS_ID, SMS_NOTIND, SMS_MWI, SMS_ORIG, SMS_TERM, PACA_IND, PL_IND, VAD_ID, PPS_ID, RCS_ID,"
    "WIN_ID, MCS_ID, XRS_ID, CMS_ID, CIS_ID, LAST_CALL, SCA_ID, FA_PDN, MAH_PDN, SCS_ID, CNFN )"
"VALUES( ?, ?, ?, ?, ?,"
       " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?," 
       " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?," 
       " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?," 
       " ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" 
 ) != IDE_SUCCESS);            
  
  i = 0;
  p = hss_bytes;

  IDE_TEST( sQueryI->bind(++i,p,sz=9,SQL_BYTES) != IDE_SUCCESS ); p+=sz; 
  IDE_TEST( sQueryI->bind(++i,p,sz=9,SQL_BYTES) != IDE_SUCCESS ); p+=sz; 
  IDE_TEST( sQueryI->bind(++i,p,sz=4,SQL_BYTES) != IDE_SUCCESS ); p+=sz; 
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz; 
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=4,SQL_BYTES) != IDE_SUCCESS ); p+=sz; 
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=3,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=5,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=4,SQL_BYTES) != IDE_SUCCESS ); p+=sz;

  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz,SQL_BYTES) != IDE_SUCCESS ); p+=sz;

  IDE_TEST( sQueryI->bind(++i,p,sz=12,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz   ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz   ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz   ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  
  IDE_TEST( sQueryI->bind(++i,p,sz=11,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz   ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=4,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
 
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz  ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz  ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz  ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz  ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz  ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;

  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=1,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz  ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz  ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz  ,SQL_BYTES) != IDE_SUCCESS ); p+=sz;

  IDE_TEST( sQueryI->bind(++i,p,sz=13,SQL_BYTES) != IDE_SUCCESS );p+=sz;

  IDE_TEST( sQueryI->bind(++i,p,sz=4,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=5,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=5,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=2,SQL_BYTES) != IDE_SUCCESS ); p+=sz;
  IDE_TEST( sQueryI->bind(++i,p,sz=12,SQL_BYTES) != IDE_SUCCESS ); p+=sz;

  idlOS::printf("count = %d, size = %d \n", i,(p - hss_bytes) );

 for(i = 0 ; i < count; i++)
 {
   idlOS::memset(hss_bytes,0x11+i,sizeof(hss_bytes) );   
   IDE_TEST(  sQueryI->execute() != IDE_SUCCESS);
 }

 
 IDE_TEST( sQueryI->execute("SELECT * from T1") != IDE_SUCCESS);
 IDE_TEST( printSelect( sQueryI) != IDE_SUCCESS );
 
 return IDE_SUCCESS;  

 IDE_EXCEPTION_END;
 {
  if( con )
  {   
    SChar *s = con->error();   
    if(s)
    {   
    idlOS::printf("Slave->RROR =>%s\n", s ) ;
    }
  } 
 } 
  return IDE_FAILURE;
}


Connection * newOraConnection()
{
 SChar              * error;
 void * _handle;

 if( newConn == NULL) 
 {
    _handle = idlOS::dlopen ("libutOra.so", RTLD_NOW | RTLD_GLOBAL );
    if ((error   = idlOS::dlerror()) != NULL)
    {
     idlOS::fprintf (stderr, "%s\n", error);
     exit(1);
    }
  newConn = (newExtConnection)idlOS::dlsym(_handle,"newConnection");
    if ((error = idlOS::dlerror()) != NULL)
    {
     idlOS::fprintf (stderr, "%s\n", error);
     exit(1);
    }
 }   
 return newConn();
}    
