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
 * $Id: rpxSenderHandshake.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <ide.h>
#include <idm.h>

#include <smi.h>
#include <smErrorCode.h>

#include <qci.h>

#include <rpDef.h>
#include <rpuProperty.h>
#include <rpcHBT.h>
#include <rpcManager.h>
#include <rpxSender.h>
#include <rpnComm.h>

/* PROJ-2240 */
#include <rpdCatalog.h>

//===================================================================
//
// Name:          attemptHandshake
//
// Return Value:  IDE_SUCCESS/IDE_FAILURE
//
// Argument:
//
// Called By:     rpxSender::run() or rpcManager::startSenderThread()
//
// Description:
//
//===================================================================

IDE_RC
rpxSender::attemptHandshake(idBool *aHandshakeFlag)
{
    SInt         sHostNum;
    SInt         sOldHostNum;
    rpMsgReturn  sRC             = RP_MSG_DISCONNECT;
    IDE_RC       sConnSuccess    = IDE_FAILURE;
    idBool       sRegistHost     = ID_FALSE;
    idBool       sTraceLogEnable = ID_TRUE;
    smSN         sReceiverXSN    = SM_SN_NULL;

    *aHandshakeFlag = ID_FALSE;

    IDE_TEST(rpdCatalog::getIndexByAddr(mMeta.mReplication.mLastUsedHostNo,
                                     mMeta.mReplication.mReplHosts,
                                     mMeta.mReplication.mHostCount,
                                     &sHostNum)
             != IDE_SUCCESS);

    sOldHostNum = sHostNum;

    // To Fix PR-4064
    idlOS::memset( mRCMsg, 0x00, RP_ACK_MSG_LEN );

    //--------------------------------------------------------------//
    //    handshake until success
    //--------------------------------------------------------------//
    while(checkInterrupt() != RP_INTR_EXIT)
    {
        IDE_CLEAR();
        sConnSuccess = IDE_FAILURE;

        //--------------------------------------------------------------//
        //    connect to Peer Server
        //--------------------------------------------------------------//
        /*
         *   IDE_SUCCESS : aRC is      ID_FALSE, ID_TRUE
         *   IDE_FAILURE : aRC is only ID_FALSE (create socket error)
         */
        do
        {
            sConnSuccess = connectPeer(sHostNum);

            if(sConnSuccess != IDE_SUCCESS)
            {
                IDE_TEST( isParallelChild() == ID_TRUE );
                if(mCurrentType != RP_OFFLINE)
                {
                    IDE_TEST(getNextLastUsedHostNo(&sHostNum) != IDE_SUCCESS);
                }

                IDE_TEST( ( mTryHandshakeOnce == ID_TRUE ) &&
                          ( sHostNum == sOldHostNum ) );
                /* 네트워크 장애 발생 후 재접속 하였는데도 접속이 안되는 경우에는
                 * 실제 장애 상황으로 판단함 
                 */
                if( ( mIsRemoteFaultDetect == ID_TRUE ) && 
                    ( rpcManager::isStartupFailback() != ID_TRUE ) &&
                    ( isParallelChild() != ID_TRUE ) ) /*의미를 명확히 하기 위해 남겨둠*/ 
                {
                    // REMOTE_FAULT_DETECT_TIME 컬럼을 현재 시간으로 갱신한다.
                    IDE_TEST( updateRemoteFaultDetectTime() != IDE_SUCCESS );
                }
                else
                {
                    /* nothing to do */
                }
                sleepForNextConnect();
            }
            else
            {
                break;
            }
        }
        // Parallel Child의 Network 장애 시 Exit Flag를 설정하기 위해,
        // checkInterrupt()를 사용해야 한다.
        while(checkInterrupt() != RP_INTR_EXIT);

        if(sConnSuccess == IDE_SUCCESS)
        {
            //--------------------------------------------------------------//
            //    Connect Success & check Replication Available
            //--------------------------------------------------------------//
            IDE_TEST(checkReplAvailable(&sRC, &mFailbackStatus, &sReceiverXSN) 
                     != IDE_SUCCESS);

            switch(sRC)
            {
                case RP_MSG_OK :            // success return
                    // PROJ-1537
                    if((mMeta.mReplication.mRole != RP_ROLE_ANALYSIS) && 
                       (mCurrentType != RP_OFFLINE)) //PROJ-1915
                    {
                        IDE_TEST( rpcHBT::registHost( &mRsc,
                                                     mMeta.mReplication.mReplHosts[sHostNum].mHostIp,
                                                     mMeta.mReplication.mReplHosts[sHostNum].mPortNo )
                                 != IDE_SUCCESS );
                        /*
                         *  unSet HBT Resource to Messenger
                         */
                        mMessenger.setHBTResource( mRsc );

                        sRegistHost = ID_TRUE;
                    }
                    IDE_CONT(VALIDATE_SUCCESS);

                case RP_MSG_DISCONNECT :    // continue to re-connect
                    // checkReplAvailable Network Error  ===> retry
                    disconnectPeer();
                    IDE_TEST_RAISE(isParallelChild() == ID_TRUE, ERR_NOT_AVAILABLE);
                    IDE_TEST(getNextLastUsedHostNo(&sHostNum) != IDE_SUCCESS);

                    IDE_TEST_RAISE((mTryHandshakeOnce == ID_TRUE) &&
                                   (sHostNum == sOldHostNum),
                                   ERR_NOT_AVAILABLE);

                    IDU_FIT_POINT( "1.BUG-15084@rpxSender::attemptHandshake" );

                    // BUG-15084
                    sleepForNextConnect();
                    break;

                case RP_MSG_DENY :
                case RP_MSG_NOK :
                    disconnectPeer();
                    IDE_TEST_RAISE(isParallelChild() == ID_TRUE, ERR_NOT_AVAILABLE);
                    IDE_TEST(getNextLastUsedHostNo(&sHostNum) != IDE_SUCCESS);
                    IDE_TEST_RAISE(mTryHandshakeOnce == ID_TRUE, ERR_NOT_AVAILABLE);
                    sleepForNextConnect();
                    break;

                case RP_MSG_PROTOCOL_DIFF :
                case RP_MSG_META_DIFF :     // error return to caller
                    IDE_RAISE(ERR_INVALID_REPL);

                default :
                    // TODO : 패킷이 깨진 것일까? 뭔가 처리를 해야한다.
                    IDE_CALLBACK_FATAL("[Repl Sender] Can't be here");
            } // switch
        } // if
    } // while

    // Exit Flag가 ID_TRUE로 설정되었을 때는 아래의 작업을 하면 안 된다.
    return IDE_SUCCESS;

    RP_LABEL(VALIDATE_SUCCESS);

    mSenderInfo->initReconnectCount();
    mSenderInfo->setOriginReplMode( mMeta.mReplication.mReplMode );

    // BUG-15507
    mRetry = 0;

    IDE_TEST(initXSN(sReceiverXSN) != IDE_SUCCESS);

    IDE_TEST( addXLogKeepAlive() != IDE_SUCCESS );

    //----------------------------------------------------------------//
    //   set TCP information
    //----------------------------------------------------------------//
    if ( mSocketType == RP_SOCKET_TYPE_TCP )
    {
        mMessenger.updateTcpAddress();
    }
    else
    {
        /* nothing to do */
    }

    *aHandshakeFlag = ID_TRUE;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_AVAILABLE);
    {
        sTraceLogEnable = ID_FALSE;
    }
    IDE_EXCEPTION(ERR_INVALID_REPL);
    {
        sTraceLogEnable = ID_FALSE;
    }
    IDE_EXCEPTION_END;

    if ( sTraceLogEnable == ID_TRUE )
    {
        IDE_ERRLOG( IDE_RP_0 );
    }

    if(sConnSuccess == IDE_SUCCESS)
    {
        disconnectPeer();

        if(sRegistHost == ID_TRUE)
        {
            mMessenger.setHBTResource( NULL );
            rpcHBT::unregistHost(mRsc);
            mRsc = NULL;
        }
    }

    if(idlOS::strlen(mRCMsg) == 0)
    {
        idlOS::sprintf(mRCMsg, "Handshake Process Error");
    }
    IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_SENDER_HANDSHAKE, mRCMsg));

    return IDE_FAILURE;
}

//===================================================================
//
// Name:          releaseHandshake
//
// Return Value:  IDE_SUCCESS/IDE_FAILURE
//
// Argument:
//
// Called By:
//
// Description:
//             Handshake시 할당 받은 통신 리소스를 해제한다.
//===================================================================
void rpxSender::releaseHandshake()
{
    disconnectPeer();

    /*
     *  unSet HBT Resource to Messenger
     */
    mMessenger.setHBTResource( NULL );

    // PROJ-1537
    if((mMeta.mReplication.mRole != RP_ROLE_ANALYSIS) &&
       (mCurrentType != RP_OFFLINE)) //PROJ-1915
    {
        rpcHBT::unregistHost(mRsc);
    }

    mRsc = NULL;

    return;
}



//===================================================================
//
// Name:          connectPeer
//
// Return Value:  IDE_SUCCESS/IDE_FAILURE
//
// Argument:
//
// Called By:     rpxSender::attemptHandshake()
//
// Description:
//
//===================================================================

IDE_RC rpxSender::connectPeer(SInt aHostNum)
{
    SChar * sHostIP = NULL;

    sHostIP = mMeta.mReplication.mReplHosts[aHostNum].mHostIp;

    // PROJ-1537
    if(idlOS::strMatch(RP_SOCKET_UNIX_DOMAIN_STR, RP_SOCKET_UNIX_DOMAIN_LEN,
                       sHostIP, idlOS::strlen(sHostIP)) == 0)
    {
        mSocketType = RP_SOCKET_TYPE_UNIX;

        idlOS::memset(mSocketFile, 0x00, RP_SOCKET_FILE_LEN);
        idlOS::snprintf(mSocketFile, RP_SOCKET_FILE_LEN, "%s%c%s%c%s%s",
                        idlOS::getenv(IDP_HOME_ENV), IDL_FILE_SEPARATOR,
                        "trc", IDL_FILE_SEPARATOR,
                        "rp-", mRepName);

        IDE_TEST( mMessenger.connectThroughUnix( mSocketFile )
                  != IDE_SUCCESS );
    }
    else
    {
        mSocketType = RP_SOCKET_TYPE_TCP;

        IDE_TEST( mMessenger.connectThroughTcp( 
                      sHostIP, mMeta.mReplication.mReplHosts[aHostNum].mPortNo )
                  != IDE_SUCCESS );
    }

    mNetworkError = ID_FALSE;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    mNetworkError = ID_TRUE;

    return IDE_FAILURE;
}

void rpxSender::disconnectPeer()
{
    mMessenger.disconnect();

    mNetworkError = ID_TRUE;

    return;
}

//===================================================================
//
// Name:          checkReplAvailable
//
// Return Value:  IDE_SUCCESS/IDE_FAILURE
//
// Argument:
//
// Called By:     rpxSender::attemptHandshake()
//
// Description:
//
//===================================================================

IDE_RC rpxSender::checkReplAvailable(rpMsgReturn *aRC,
                                     SInt        *aFailbackStatus,
                                     smSN        *aXSN)
{
    /* Server가 Startup 단계인지 여부는 유동적이다. */
    if ( rpcManager::isStartupFailback() == ID_TRUE )
    {
        rpdMeta::setReplFlagFailbackServerStartup( &(mMeta.mReplication) );
    }
    else
    {
        rpdMeta::clearReplFlagFailbackServerStartup( &(mMeta.mReplication) );
    }

    IDE_TEST( mMessenger.handshakeAndGetResults( &mMeta, aRC,
                                                 mRCMsg,
                                                 sizeof( mRCMsg ),
                                                 aFailbackStatus,
                                                 aXSN )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : Network를 종료하지 않고 Handshake를 수행한다.
 *
 ***********************************************************************/
IDE_RC rpxSender::handshakeWithoutReconnect()
{
    rpMsgReturn sRC = RP_MSG_DISCONNECT;
    SInt        sFailbackStatus;
    ULong       sDummyXSN;

    if(mMeta.mReplication.mRole == RP_ROLE_ANALYSIS)
    {
        ideLog::log( IDE_RP_0, "[%s] XLog Sender: Send XLog for Meta change.\n", mMeta.mReplication.mRepName );
        // Ala Receiver에게 Handshake를 다시 할 것이라고 알리고 종료한다.
        IDE_TEST(addXLogHandshake() != IDE_SUCCESS);

        /* Send Replication Stop Message */
        ideLog::log( IDE_RP_0, "[%s] XLog Sender: SEND Stop Message for meta change\n", mMeta.mReplication.mRepName );
        
        IDE_TEST( mMessenger.sendStop( getRestartSN() ) != IDE_SUCCESS );

        ideLog::log( IDE_RP_0, "SEND Stop Message SUCCESS!!!\n" );

        finalizeSenderApply();
        // Altibase Log Analyzer로 동작할 때는 Network 연결을 종료한다.
        mNetworkError = ID_TRUE;
        mSenderInfo->deActivate(); //isDisconnect()
    }
    else
    {
        IDU_FIT_POINT( "rpxSender::handshakeWithoutReconnect::ERR_HANDSHAKE" );

        IDE_WARNING(IDE_RP_0, RP_TRC_SH_NTC_HANDSHAKE_XLOG);

        // Receiver에게 Handshake를 다시 할 것이라고 알린다.
        IDE_TEST(addXLogHandshake() != IDE_SUCCESS);

        // Sender Apply Thread가 Handshake Ready XLog를 수신할 때까지 기다린다.
        while(mSenderApply->isSuspended() != ID_TRUE)
        {
            IDE_TEST_CONT(checkInterrupt() != RP_INTR_NONE, NORMAL_EXIT);
            idlOS::thr_yield();
        }

        // Handshake를 수행한다.
        IDE_TEST(checkReplAvailable(&sRC,
                                    &sFailbackStatus, // Dummy
                                    &sDummyXSN)
                 != IDE_SUCCESS);   // always return IDE_SUCCESS
        if(sRC != RP_MSG_OK)
        {
            mNetworkError = ID_TRUE;
            mSenderInfo->deActivate(); //isDisconnect()
        }

        // Sender Apply Thread가 정상 동작하도록 한다.
        mSenderApply->resume();
    }

    RP_LABEL(NORMAL_EXIT);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
