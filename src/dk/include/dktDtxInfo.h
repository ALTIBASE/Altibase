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
 
#ifndef _O_DKT_DTX_INFO_H_
#define _O_DKT_DTX_INFO_H_ 1

#include <idl.h>
#include <dkDef.h>
#include <dkt.h>
#include <smiDef.h>
#include <sdi.h>

class dktDtxInfo
{
private:

public:
    smTID          mLocalTxId;
    UInt           mResult;
    smLSN          mPrepareLSN;
    UInt           mGlobalTxId;
    iduList        mBranchTxInfo;
    UInt           mBranchTxCount;
    dktLinkerType  mLinkerType;
    iduListNode    mNode;

    void initialize( UInt aLocalTxId, UInt aGlobalTxId );

    void finalize( );

    IDE_RC createDtxInfo( UInt aTID, UInt  aGlobalTxId );

    IDE_RC removeDtxBranchTx( ID_XID * aXID );
    IDE_RC removeDtxBranchTx( dktDtxBranchTxInfo * aDtxBranchTxInfo );
    void removeAllBranchTx();

    IDE_RC addDtxBranchTx( ID_XID * aXID, SChar * aTarget );
    IDE_RC addDtxBranchTx( ID_XID * aXID,
                           SChar  * aNodeName,
                           SChar  * aUserName,
                           SChar  * aUserPassword,
                           SChar  * aDataServerIP,
                           UShort   aDataPortNo,
                           UShort   aConnectType );
    IDE_RC addDtxBranchTx( dktDtxBranchTxInfo * aDtxBranchTxInfo );
    dktDtxBranchTxInfo * getDtxBranchTx( ID_XID * aXID );

    UInt estimateSerializeBranchTx();
    IDE_RC serializeBranchTx( UChar * aBranchTxInfo, UInt aSize );
    IDE_RC unserializeAndAddDtxBranchTx( UChar * aBranchTxInfo, UInt aSize );
    IDE_RC copyString( UChar ** aBuffer,
                       UChar * aFence,
                       SChar * aString );

    inline dktLinkerType getLinkerType();
    inline smLSN * getPrepareLSN( void );
    inline UInt getFileNo( void );
    inline idBool isEmpty();
};

class dktXid
{
public:
    static void   initXID( ID_XID * aXID );
    static void   copyXID( ID_XID * aDst, ID_XID * aSrc );
    static idBool isEqualXID( ID_XID * aXID1, ID_XID * aXID2 );
    static UChar  sizeofXID( ID_XID * aXID );
};

inline dktLinkerType dktDtxInfo::getLinkerType()
{
    return mLinkerType;
}

inline smLSN * dktDtxInfo::getPrepareLSN( void )
{
    return &mPrepareLSN;
}

inline idBool dktDtxInfo::isEmpty() 
{ 
    if ( mBranchTxCount == 0 )
    {
        return ID_TRUE;
    }
    else
    {
        return ID_FALSE;
    }
}
#endif  /* _O_DKT_DTX_INFO_H_ */
