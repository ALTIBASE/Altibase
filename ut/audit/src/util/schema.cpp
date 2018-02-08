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
 * $Id: schema.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utdb.h>

static SInt print =  0;

uteErrorMgr  gErrorMgr;

IDE_RC schema(SChar*,SChar*,SChar*,SChar*,UInt);
void  printTable();

dbDriver * dbdSl = NULL;
dbDriver * dbdMa = NULL;

int main(int argc, char **argv)
{
  const SChar * sMasterURL =
      "altibase://sys:manager@DSN=localhost;PORT_NO=20545;NLS_USE=US7ASCII";
  const SChar * sSlaveURL  = "oracle://scott:tiger@ORA9V880";
      
  SChar * tf1 = "CREATE TABLE %s ("
    " i1 NUMERIC(38,10)"
    ",i2 DATE default sysdate"
//  ",I2 NUMERIC(20) default 0"
//    ",i2 TIMESTAMP(6) default systimestamp" // ORACLE DATE 
    ",i3 VARCHAR(10) "
    ",i4 NUMERIC" //  UNIQUE" 
    ",i5 NUMERIC(38,14) DEFAULT 1.321"
    ",i6 VARCHAR(46) default systimestamp"
    ",i7 VARCHAR(100)"
    ",PRIMARY KEY(i1,i2)"
    ")" ;
  SChar * tf2 = "CREATE TABLE %s ("
    " i1 NUMERIC(38,10)"
//  ",I2 NUMERIC(20) default 0"
    ",i2 DATE default sysdate"
//  ",i2 TIMESTAMP(6) default systimestamp" // ORACLE DATE 
    ",i3 VARCHAR(10) "
    ",i4 NUMERIC" //  UNIQUE" 
    ",i5 NUMERIC(38,14) DEFAULT 1.321"
    ",i6 VARCHAR(46) default systimestamp"
    ",i7 VARCHAR(100)"
    ",PRIMARY KEY(i1,i2)"
  //  ", i1 INTEGER"
    ")";

  SInt     op;
  UInt     count = 15;

  while( (op = idlOS::getopt(argc, argv, "m:s:n:p")) != EOF)
  {
      switch(op)
      {
          case 'm':
              sMasterURL = optarg;
              break;
              
          case 's':
              sSlaveURL  = optarg;
              break;
              
          case 'n':
              count = idlOS::atoi(optarg);
              break;
          case 'p':
              print = 1;
              break;
      }
  }
      

  dbdMa = new dbDriver();
  IDE_TEST( dbdMa->initialize( sMasterURL ) != IDE_SUCCESS);   
   
  dbdSl = new dbDriver();
  IDE_TEST( dbdSl->initialize( sSlaveURL  ) != IDE_SUCCESS);   
 
  IDE_TEST( schema("T1", tf2,"T1", tf1, count) != IDE_SUCCESS );

  return 0;
  IDE_EXCEPTION_END;
  return 1;  
}


IDE_RC schema( SChar* tNameA, SChar * tf1,
               SChar *tNameB, SChar * tf2,
               UInt count)
{
  Connection    *conA = NULL;
  Connection    *conB = NULL;
  Query      *sQueryA, *sQueryB;
  Row        *row = NULL;
 
  SChar    sStr [512]; 
  UInt     i;

  conA = dbdMa->connection();
  IDE_TEST( conA == NULL );
  IDE_TEST( conA->initialize() != IDE_SUCCESS );
  IDE_TEST( conA->connect() != IDE_SUCCESS) ;
 
  conB = dbdSl->connection();
  IDE_TEST( conB == NULL );
  IDE_TEST( conB->initialize() != IDE_SUCCESS );
  IDE_TEST( conB->connect() != IDE_SUCCESS) ;
 

  conA->execute("DROP TABLE %s",tNameA);
  
 
  conB->execute("DROP TABLE %s",tNameB);

  IDE_TEST( conA->execute( tf1, tNameA)!=IDE_SUCCESS);
  IDE_TEST( conB->execute( tf2, tNameB)!=IDE_SUCCESS);

  
  
  sQueryA   = conA->query();
  IDE_TEST( sQueryA->prepare("INSERT INTO %s( I1, I3, I4, I5) VALUES( ?, ?, ?, ?)", tNameA) != IDE_SUCCESS);

  


  IDE_TEST( sQueryA->bind(1,&   i,sizeof(    i),SQL_INTEGER) != IDE_SUCCESS );
//IDE_TEST( sQueryA->bind(2,&   i,sizeof(    i),SQL_INTEGER) != IDE_SUCCESS );
  IDE_TEST( sQueryA->bind(2,&sStr,sizeof( sStr),SQL_VARCHAR) != IDE_SUCCESS );

  IDE_TEST( sQueryA->bind(3,&   i,sizeof(    i),SQL_INTEGER) != IDE_SUCCESS );
  IDE_TEST( sQueryA->bind(4,&   i,sizeof(    i),SQL_INTEGER) != IDE_SUCCESS );

  sStr[0] = 0;
  for(i = 0  ; i < count; i++ )
    {

      IDE_TEST(  sQueryA->execute() != IDE_SUCCESS);
      idlOS::sprintf(sStr,"Alex %d",i+1 );  
    }

  IDE_TEST( sQueryA->execute(
              "SELECT I1, I2, I3, I4, I5   FROM %s"
              ,tNameA
              ) != IDE_SUCCESS);
  i = 0;
 
 
    IDE_TEST( sQueryA->bindColumn(1,SQL_NUMERIC  ) != IDE_SUCCESS);
    IDE_TEST( sQueryA->bindColumn(2,SQL_TIMESTAMP)  != IDE_SUCCESS);
//  IDE_TEST( sQueryA->bindColumn(2,SQL_DATE )  != IDE_SUCCESS);
//    IDE_TEST( sQueryA->bindColumn(2,SQL_NUMERIC  ) != IDE_SUCCESS);
    IDE_TEST( sQueryA->bindColumn(3,SQL_VARCHAR  ) != IDE_SUCCESS);
    IDE_TEST( sQueryA->bindColumn(4,SQL_NUMERIC  ) != IDE_SUCCESS);
    IDE_TEST( sQueryA->bindColumn(5,SQL_NUMERIC  ) != IDE_SUCCESS);

//  printSelect(sQueryA);


  row = sQueryA->fetch( DBA_ATB );

  IDE_TEST(row == NULL );


  
  sQueryB   = conB->query();
    IDE_TEST( sQueryB->prepare("INSERT INTO %s( i1, i2,  i3, i4, i5) VALUES(?,?,?,?,?)",tNameB)
	    != IDE_SUCCESS);
  
  IDE_TEST( sQueryB->bind(1, row->getField(1) ) != IDE_SUCCESS );
  IDE_TEST( sQueryB->bind(2, row->getField(2) ) != IDE_SUCCESS );
  IDE_TEST( sQueryB->bind(3, row->getField(3) ) != IDE_SUCCESS );
  IDE_TEST( sQueryB->bind(4, row->getField(4) ) != IDE_SUCCESS );
  IDE_TEST( sQueryB->bind(5, row->getField(5) ) != IDE_SUCCESS );

  row = sQueryA->fetch( DBA_ATB );
  while( row ) 
    {

      IDE_TEST(sQueryB->execute() != IDE_SUCCESS );   
      row = sQueryA->fetch(DBA_ATB); // FETCH in Altibase Format 
    }    
 
  sQueryA->execute("UPDATE %s set I5 = I1  +1.12345 where I1 > 2 AND I1 < 5 ",tNameA ); // SU/SD 
  sQueryB->execute("UPDATE %s set I5 = I1  -1.00001 where I1 > 2 AND I1 < 5 ",tNameB ); // SU/SD

  sQueryB->execute("UPDATE %s set i4 = -i1 where I1 > 5 AND I1 < 7",tNameB );
   
  sQueryA->execute("DELETE from %s where I1 > %d AND I1 < %d",tNameA, count/3-2 , count/3+2 );    // SI/SD
   
  sQueryB->execute("DELETE from %s where I1 = 5",tNameB);  // MI/SD
  sQueryB->execute("DELETE from %s where I1 > %d",tNameB,count - 5); // MI/SD

  if( print > 0 )
    {    
      IDE_TEST( sQueryA->execute("SELECT i1,i2,i3,i4,i5,i6 from %s",tNameA) != IDE_SUCCESS);
      IDE_TEST( printSelect( sQueryA) != IDE_SUCCESS );

      IDE_TEST( sQueryB->execute("SELECT i1,i2,i3,i4,i5,i6 from %s",tNameB) != IDE_SUCCESS);
      IDE_TEST( printSelect( sQueryB) != IDE_SUCCESS );
    }
  delete conA; 
  delete conB;
  return IDE_SUCCESS;  

  IDE_EXCEPTION_END;
  if(conA)
    {
      SChar *s = conA->error();
      if(s)
	{    
	  idlOS::printf("Master->RROR =>%s\n", s ) ;
	} 
      delete conA;
    }
  if(conB)
    {   
      SChar *s = conB->error();   
      if(s)
	{   
	  idlOS::printf("Slave->RROR =>%s\n", s ) ;
	} 
      delete conB;
    } 
   
  return IDE_FAILURE;
}
