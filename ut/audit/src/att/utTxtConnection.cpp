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
 * $Id: utTxtConnection.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utTxt.h>


IDE_RC utTxtConnection::connect(const SChar * fileName, ... )

{
 va_list args;

  /* set up the FILE name */
  IDE_TEST( fileName == NULL );
  mSchema = (SChar*)fileName;

  va_start(args, fileName);

  mExt = va_arg(args,SChar*);                // extantion of FILE name
  Connection::mDSN    = va_arg(args,SChar*); // Directory Name (PATH)

  va_end(args);

   IDE_TEST( connect() != IDE_SUCCESS);

  return IDE_SUCCESS;
  IDE_EXCEPTION_END;
  return IDE_FAILURE;
}

IDE_RC utTxtConnection::connect()
{
  time_t timet;
  const SChar *sSep = IDL_FILE_SEPARATORS;
  const SChar *sDir = Connection::mDSN;

  IDE_TEST( disconnect() != IDE_SUCCESS);

  /* just for schema */
  if(mSchema == NULL)
  {
    mSchema = dbd->getUserName();
  }

  /* default dir */
  if( sDir == NULL )
  {
    sSep = "";
    sDir = "";
  }

  /* BUG-30301 */
  if (mExt == NULL)
  {
      mExt = "sql";
  }
  
  /* get timestamp values */
  idlOS::time(&timet);
  idlOS::sprintf(mDSN, "%s"  "%s"     "%s"  "_%"ID_UINT64_FMT ".%s"
                     ,sDir, sSep, mSchema,       (ULong)timet, mExt);

  fd = idlOS::fopen(mDSN,"w+");

  IDE_TEST( fd == NULL);

  if( mQuery == NULL )
    {
      mQuery =  new utTxtQuery( this );
      IDE_TEST( mQuery == NULL );
      IDE_TEST( mQuery->initialize() != IDE_SUCCESS);
    }

  return IDE_SUCCESS;

  IDE_EXCEPTION_END;

  fd = NULL;

  mErrNo = SQL_ERROR;
  return IDE_FAILURE;
}

IDE_RC  utTxtConnection::autocommit(bool)
{
  return IDE_SUCCESS;
}


IDE_RC  utTxtConnection::commit()
{
  return (0 == idlOS::fflush(fd) ) ? IDE_SUCCESS: IDE_FAILURE;
}

IDE_RC  utTxtConnection::rollback()
{
  return IDE_SUCCESS;
}


Query * utTxtConnection::query(void)
{
  utTxtQuery * sQuery  = new utTxtQuery( this );
  if( sQuery != NULL )
    {
      IDE_TEST( sQuery->initialize() != IDE_SUCCESS);
      IDE_TEST( mQuery->add( sQuery )!= IDE_SUCCESS);
    }

  return     sQuery;

  IDE_EXCEPTION_END;

  sQuery->finalize();

  delete sQuery;

  return NULL;
}

IDE_RC utTxtConnection::disconnect()
{
  if(fd != NULL)
  {
   IDE_TEST( 0 > idlOS::fclose(fd));
   fd = NULL;
  }

  return IDE_SUCCESS;

   IDE_EXCEPTION_END;

   fd = NULL;

  return IDE_FAILURE;
}

bool utTxtConnection::isConnected( )
{
  return  (fd != NULL);
}

utTxtConnection::utTxtConnection(dbDriver *aDbd)
    :Connection(aDbd)
{
  fd = NULL;
  Connection::mDSN = aDbd->getDSN();
  mDSN[0]  ='\0';
  mExt = NULL;
}

IDE_RC utTxtConnection::initialize(SChar * buffErr, UInt buffSize)
{
  IDE_TEST( Connection::initialize(buffErr, buffSize) != IDE_SUCCESS);
  mErrNo = SQL_SUCCESS;

  return IDE_SUCCESS;

   IDE_EXCEPTION_END;

  return IDE_FAILURE;
}

metaColumns * utTxtConnection::getMetaColumns(SChar *,SChar *)
{
  return  NULL;
}


utTxtConnection::~utTxtConnection(void)
{
  finalize();
}

IDE_RC utTxtConnection::finalize()
{

  IDE_TEST( Connection::finalize() );

  return IDE_SUCCESS;

  IDE_EXCEPTION_END;

  return IDE_FAILURE;
}


SChar * utTxtConnection::error(void *)
{
  static SChar * sRetStr = (SChar *)"";
  return sRetStr;
}

dba_t utTxtConnection::getDbType(void)
{
  return DBA_TXT;
}
