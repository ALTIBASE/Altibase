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
 

/***********************************************************************
 * $Id$ sdl.h
 **********************************************************************/

#ifndef _O_SDL_H_
#define _O_SDL_H_ 1

#include <sdi.h>
#include <sdlSqlConnAttrType.h>

#define SQL_FALSE               0
#define SQL_TRUE                1

#define SQL_HANDLE_ENV          1
#define SQL_HANDLE_DBC          2
#define SQL_HANDLE_STMT         3
#define SQL_HANDLE_DESC         4

#define SQL_NULL_HANDLE         0L

#define SQL_SUCCESS             0
#define SQL_SUCCESS_WITH_INFO   1
#define SQL_NO_DATA             100
#define SQL_NO_DATA_FOUND       100
#define SQL_NULL_DATA           ((SInt)-1)
#define SQL_ERROR               ((SInt)-1)
#define SQL_INVALID_HANDLE      ((SInt)-2)

#define SQL_CLOSE               0
#define SQL_DROP                1
#define SQL_UNBIND              2
#define SQL_RESET_PARAMS        3

#define SQL_NTS                 (-3)

#define MAX_ODBC_ERROR_CNT       4
#define MAX_ODBC_ERROR_MSG_LEN   1024
#define MAX_ODBC_MSG_LEN         512

#define MAX_HOST_NUM    128

#define HOST_MAX_LEN    SDI_SERVER_IP_SIZE
#define USER_ID_LEN     128
#define MAX_PWD_LEN     32
#define MAX_HOST_INFO_LEN (HOST_MAX_LEN + 3 + USER_ID_LEN + MAX_PWD_LEN + 8)
       // 3=(:::), 8=(port length)

typedef void * LIBRARY_HANDLE;

class sdl
{
public:

    static IDE_RC initOdbcLibrary();
    static void   finiOdbcLibrary();

    static IDE_RC allocConnect( sdiConnectInfo * aConnectInfo );

    static IDE_RC disconnect( void   * aSession,
                              SChar  * aNodeName,
                              idBool * aIsLinkFailure = NULL );

    static IDE_RC disconnectLocal( void   * aDbc,
                                   SChar  * aNodeName,
                                   idBool * aIsLinkFailure = NULL );

    static IDE_RC freeConnect( void   * aSession,
                               SChar  * aNodeName,
                               idBool * aIsLinkFailure = NULL);

    static IDE_RC allocStmt( void    * aDbc,
                             void   ** aStmt,
                             SChar   * aNodeName,
                             idBool  * aIsLinkFailure = NULL );

    static IDE_RC freeStmt( void   * aStmt,
                            SShort   aOption,
                            SChar  * aNodeName,
                            idBool * aIsLinkFailure = NULL );

    static IDE_RC prepare( void   * aStmt,
                           SChar  * aQuery,
                           SInt     aLength,
                           SChar  * aNodeName,
                           idBool * aIsLinkFailure = NULL );

    static IDE_RC addPrepareCallback( void ** aCallback,
                                      UInt    aNodeIndex,
                                      void   *aStmt,
                                      SChar  *aQuery,
                                      SInt    aLength,
                                      SChar  *aNodeName,
                                      idBool *aIsLinkFailure = NULL );

    static IDE_RC describeCol( void   * aStmt,
                               UInt     aColNo,
                               SChar  * aColName,
                               SInt     aBufferLength,
                               UInt   * aNameLength,
                               UInt   * aTypeId,
                               SInt   * aPrecision,
                               SShort * aScale,
                               SShort * aNullable,
                               SChar  * aNodeName,
                               idBool * aIsLinkFailure = NULL );

    static IDE_RC getParamsCount( void   * aStmt,
                                  UShort * aParamCount,
                                  SChar  * aNodeName,
                                  idBool * aIsLinkFailure = NULL );

    static IDE_RC bindParam( void   * aStmt,
                             UShort   aParamNum,
                             UInt     aInOutType,
                             UInt     aValueType,
                             void   * aParamValuePtr,
                             SInt     aBufferLen,
                             SInt     aPrecision,
                             SInt     aScale,
                             SChar  * aNodeName,
                             idBool * aIsLinkFailure = NULL );

    static IDE_RC bindCol( void   * aStmt,
                           UInt     aColNo,
                           UInt     aColType,
                           UInt     aColSize,
                           void   * aBuffer,
                           SChar  * aNodeName,
                           idBool * aIsLinkFailure = NULL );

    static IDE_RC execDirect( void    * aStmt,
                              SChar   * aNodeName,
                              SChar   * aQuery,
                              SInt      aQueryLen,
                              idBool  * aIsLinkFailure = NULL );

    static IDE_RC execute( UInt          aNodeIndex,
                           sdiDataNode * aNode,
                           SChar       * aNodeName,
                           idBool      * aIsLinkFailure = NULL );

    static IDE_RC addExecuteCallback( void        ** aCallback,
                                      UInt           aNodeIndex,
                                      sdiDataNode  * aNode,
                                      SChar        * aNodeName,
                                      idBool       * aIsLinkFailure = NULL );

    static IDE_RC addPrepareTranCallback( void   ** aCallback,
                                          UInt      aNodeIndex,
                                          void    * aDbc,
                                          ID_XID  * aXID,
                                          UChar   * aReadOnly,
                                          SChar   * aNodeName,
                                          idBool  * aIsLinkFailure = NULL );

    static IDE_RC addEndTranCallback( void   ** aCallback,
                                      UInt      aNodeIndex,
                                      void    * aDbc,
                                      idBool    aIsCommit,
                                      SChar   * aNodeName,
                                      idBool  * aIsLinkFailure = NULL );

    static void doCallback( void * aCallback );

    static IDE_RC resultCallback( void   * aCallback,
                                  UInt     aNodeIndex,
                                  idBool   aReCall,
                                  SShort   aHandleType,
                                  void   * aHandle,
                                  SChar  * aNodeName,
                                  SChar  * aFuncName,
                                  idBool * aIsLinkFailure = NULL );

    static void removeCallback( void * aCallback );

    static IDE_RC fetch( void   * aStmt,
                         SShort * aResult,
                         SChar  * aNodeName,
                         idBool * aIsLinkFailure = NULL );

    static IDE_RC rowCount( void   * aStmt,
                            vSLong * aNumRows,
                            SChar  * aNodeName,
                            idBool * aIsLinkFailure = NULL );

    static IDE_RC getPlan( void    * aStmt,
                           SChar   * aNodeName,
                           SChar  ** aPlan,
                           idBool  * aIsLinkFailure = NULL );

    static IDE_RC setAutoCommit( void   * aDbc,
                                 SChar  * aNodeName,
                                 idBool   aIsAuto,
                                 idBool * aIsLinkFailure = NULL );

    static IDE_RC setConnAttr( void   * aDbc,
                               SChar  * aNodeName,
                               UShort   aAttrType,
                               ULong    aValue,
                               idBool * aIsLinkFailure = NULL );

    static IDE_RC getConnAttr( void   * aDbc,
                               SChar  * aNodeName,
                               UShort   aAttrType,
                               void   * aValuePtr,
                               SInt     aBuffLength = 0,
                               SInt   * aStringLength = NULL );

    static IDE_RC endPendingTran( void   * aDbc,
                                  SChar  * aNodeName,
                                  ID_XID * aXID,
                                  idBool   aIsCommit,
                                  idBool * aIsLinkFailure = NULL );

    static IDE_RC commit( void   * aDbc,
                          SChar  * aNodeName,
                          idBool * aIsLinkFailure = NULL );

    static IDE_RC rollback( void        * aDbc,
                            void        * aStmt,
                            SChar       * aNodeName,
                            const SChar * aSavePoint,
                            idBool      * aIsLinkFailure = NULL );

    static IDE_RC setSavePoint( void        * aStmt,
                                SChar       * aNodeName,
                                const SChar * aSavePoint,
                                idBool      * aIsLinkFailure = NULL );

    static IDE_RC checkDbcAlive( void   * aDbc,
                                 SChar  * aNodeName,
                                 idBool * aIsAliveConnection,
                                 idBool * aIsLinkFailure = NULL );

    static IDE_RC getLinkInfo( void   * aDbc,
                               SChar  * aNodeName,
                               SChar  * aBuf,
                               UInt     aBufSize,
                               SInt     aKey );

    static IDE_RC isDataNode( void   * aDbc,
                              idBool * aIsDataNode );

private:

    static void setIdeError( SShort   aHandleType,
                             void   * aHandle,
                             SChar  * aNodeName,
                             SChar  * aCallFncName,
                             idBool * aIsLinkFailure = NULL );

    static IDE_RC loadLibrary();

private:
    static LIBRARY_HANDLE mOdbcLibHandle;
    static idBool         mInitialized;
};

#endif /* _O_SDL_H_ */
