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
 
#include <ilo.h>

iloCommandCompiler *gCommandCompiler;
SChar               gszCommand[COMMAND_LEN];

IDE_RC ConnectDB( ALTIBASE_ILOADER_HANDLE   aHandle,
                  iloSQLApi               * aISPApi,
                  SChar                   * aHost,
                  SChar                   * aDB,
                  SChar                   * aUser,
                  SChar                   * aPasswd,
                  SChar                   * aNLS,
                  SInt                      aPortNo,
                  SInt                      aConnType,
                  iloBool                   aPreferIPv6, /* BUG-29915 */
                  SChar                   * aSslCa, /* BUG-41406 SSL */
                  SChar                   * aSslCapath,
                  SChar                   * aSslCert,
                  SChar                   * aSslKey,
                  SChar                   * aSslVerify,
                  SChar                   * aSslCipher)
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_TEST(aISPApi->Open(aHost, aDB, aUser, aPasswd, aNLS, aPortNo,
                           aConnType, 
                           aPreferIPv6, /* BUG-29915 */
                           aSslCa,
                           aSslCapath,
                           aSslCert,
                           aSslKey,
                           aSslVerify,
                           aSslCipher)
             != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        utePrintfErrorCode(stdout, sHandle->mErrorMgr);
    }

    return IDE_FAILURE;
}

IDE_RC DisconnectDB(iloSQLApi * aISPApi)
{
    aISPApi->Close();

    return IDE_SUCCESS;
}
