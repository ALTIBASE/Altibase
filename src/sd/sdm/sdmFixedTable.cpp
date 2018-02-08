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
 * $Id$
 **********************************************************************/

#include <ideErrorMgr.h>
#include <sdm.h>
#include <sdmFixedTable.h>
#include <sdl.h>
#include <cmnDef.h>

iduFixedTableColDesc gShardConnectionInfoColDesc[]=
{
    {
        (SChar*)"NODE_ID",
        offsetof( sdmConnectInfo4PV, mNodeId ),
        IDU_FT_SIZEOF( sdmConnectInfo4PV, mNodeId ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"NODE_NAME",
        offsetof( sdmConnectInfo4PV, mNodeName ),
        SDI_NODE_NAME_MAX_SIZE,
        IDU_FT_TYPE_VARCHAR,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"COMM_NAME",
        offsetof( sdmConnectInfo4PV, mCommName ),
        IDL_IP_ADDR_MAX_LEN,
        IDU_FT_TYPE_VARCHAR,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"TOUCH_COUNT",
        offsetof( sdmConnectInfo4PV, mTouchCount ),
        IDU_FT_SIZEOF(sdmConnectInfo4PV, mTouchCount),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"LINK_FAILURE",
        offsetof( sdmConnectInfo4PV, mLinkFailure ),
        IDU_FT_SIZEOF(sdmConnectInfo4PV, mLinkFailure),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    }
};

iduFixedTableDesc  gShardConnectionInfoTableDesc =
{
    (SChar *)"X$SHARD_CONNECTION_INFO",
    sdmFixedTable::buildRecordForShardConnectionInfo,
    gShardConnectionInfoColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

IDE_RC sdmFixedTable::buildRecordForShardConnectionInfo(
    idvSQL              * aStatistics,
    void                * aHeader,
    void                * /*aDumpObj*/,
    iduFixedTableMemory * aMemory )
{
    sdmConnectInfo4PV   sConnectInfo4PV;
    idvSQL            * sStatistics;
    idvSession        * sSess;
    void              * sMmSess;
    qcSession         * sQcSess;
    sdiClientInfo     * sClientInfo;
    sdiConnectInfo    * sConnectInfo;
    idBool              sLinkAlive;
    UShort              i = 0;

    sStatistics = aStatistics;
    IDE_TEST_CONT( sStatistics == NULL, NORMAL_EXIT );

    sSess = sStatistics->mSess;
    IDE_TEST_CONT( sSess == NULL, NORMAL_EXIT );

    sMmSess = sSess->mSession;
    IDE_TEST_CONT( sMmSess == NULL, NORMAL_EXIT );

    sQcSess = qci::mSessionCallback.mGetQcSession( sMmSess );
    IDE_TEST_CONT( sQcSess == NULL, NORMAL_EXIT );

    sClientInfo = sQcSess->mQPSpecific.mClientInfo;
    IDE_TEST_CONT( sClientInfo == NULL, NORMAL_EXIT );

    sConnectInfo = sClientInfo->mConnectInfo;

    for ( i = 0; i < sClientInfo->mCount; i++, sConnectInfo++ )
    {
        if ( sConnectInfo->mDbc != NULL )
        {
            idlOS::memset( &sConnectInfo4PV, 0, ID_SIZEOF(sdmConnectInfo4PV) );

            // node ID
            sConnectInfo4PV.mNodeId = sConnectInfo->mNodeId;

            // node name
            idlOS::memcpy( sConnectInfo4PV.mNodeName,
                           sConnectInfo->mNodeName,
                           SDI_NODE_NAME_MAX_SIZE + 1 );

            // comm namae
            (void) sdl::getLinkInfo( sConnectInfo->mDbc,
                                     sConnectInfo->mNodeName,
                                     sConnectInfo4PV.mCommName,
                                     IDL_IP_ADDR_MAX_LEN,
                                     CMN_LINK_INFO_ALL );

            // touch count
            sConnectInfo4PV.mTouchCount = sConnectInfo->mTouchCount;

            // link failure
            sConnectInfo4PV.mLinkFailure = sConnectInfo->mLinkFailure;

            // link failure가 아닌경우 현재 시점에서 한번 더 검사한다.
            if ( sConnectInfo4PV.mLinkFailure == ID_FALSE )
            {
                (void) sdl::checkDbcAlive( sConnectInfo->mDbc,
                                           sConnectInfo->mNodeName,
                                           &sLinkAlive,
                                           &sConnectInfo->mLinkFailure );

                if ( ( sConnectInfo->mLinkFailure == ID_TRUE ) ||
                     ( sLinkAlive == ID_FALSE ) )
                {
                    sConnectInfo4PV.mLinkFailure = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }

            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                  aMemory,
                                                  (void *)&sConnectInfo4PV )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    IDE_EXCEPTION_CONT( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
