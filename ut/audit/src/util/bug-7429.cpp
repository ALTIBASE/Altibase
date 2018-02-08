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
 * $Id: bug-7429.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include "uto.h"

#include <utAtb.h>

IDE_RC schema(Connection * con,SChar *name, SChar* passwd, SChar* dsn,SChar * aTable, UInt count);

static newExtConnection  newConn = NULL;

Connection * newOraConnection();

int main(int argc, char **argv)
{
 Connection * con = NULL;
 UInt count = ( argc > 1 )?idlOS::atoi(argv[1]):15;

 con = new utAtbConnection();
// IDE_TEST( schema(con,"SYS","MANAGER","DSN=rtserver;PORT_NO=20545;NLS_USE=US7ASCII",
//           "T2", count) != IDE_SUCCESS );
 
 
 con = newOraConnection();
 IDE_TEST( schema(con,"ATEST","ATEST","ORA817","T1",count) != IDE_SUCCESS );

 delete con;

 
 return 0;
 IDE_EXCEPTION_END;
 return 1;  
}


IDE_RC schema(Connection * con,SChar *name, SChar* passwd, SChar* dsn,SChar * aTable, UInt count)
{
 Query         *sQueryI;
 SChar        sStr [64];
 UInt                 i;

 IDE_TEST( con == NULL );
 
 IDE_TEST( con->initialize() != IDE_SUCCESS );
 IDE_TEST( con->connect(name,passwd,dsn ) != IDE_SUCCESS) ;

 
 sQueryI = con->query();
 IDE_TEST( con->autocommit(false) != IDE_SUCCESS) ;
 IDE_TEST(
   sQueryI->prepare("INSERT INTO %s(i1,i2) VALUES(?,?)",aTable)
   != IDE_SUCCESS);

  
 IDE_TEST( sQueryI->bind(1,&    i,sizeof(    i),SQL_INTEGER) != IDE_SUCCESS );
 IDE_TEST( sQueryI->bind(2,& sStr,sizeof( sStr),SQL_VARCHAR) != IDE_SUCCESS );

 for(i = 0  ; i < count; i++ )
 {
  idlOS::sprintf(sStr,"S-%d",i );
  idlOS::printf("i=%d\n",i);
  IDE_TEST(  sQueryI->execute() != IDE_SUCCESS);
  IDE_TEST( con->commit() != IDE_SUCCESS);
 }
 
 
 IDE_TEST( sQueryI->execute("SELECT count(*) from %s",aTable) != IDE_SUCCESS);
 IDE_TEST( printSelect( sQueryI) != IDE_SUCCESS );
 
 return IDE_SUCCESS;  

 IDE_EXCEPTION_END;
 {
  if( con )
  {   
    SChar *s = con->error();   
    if(s)
    {   
    idlOS::printf("%s\n", s ) ;
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
    _handle = idlOS::dlopen ("libutOra.so", RTLD_NOW );
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
