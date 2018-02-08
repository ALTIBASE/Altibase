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

#include <idsCrypt.h>
#include <idv.h>
#include <mtl.h>
#include <qcm.h>
#include <qcmUser.h>
#include <mmm.h>
#include <mmErrorCode.h>
#include <mmcSession.h>
#include <mmcTask.h>
#include <mmtSessionManager.h>
#include <mmtServiceThread.h>
#include <smi.h>
#include <idl.h>
#include <mtz.h>
#include <mtuProperty.h>
#include <mmtAdminManager.h>
#include <mmtAuditManager.h>
#include <idvAudit.h>
#include <qci.h>

/* PROJ-2177 User Interface - Cancel */
static IDE_RC answerConnectResult(cmiProtocolContext *aProtocolContext, mmcSessID aSessionID)
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK(aProtocolContext, 5);
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP(aProtocolContext, CMP_OP_DB_ConnectResult);
    CMI_WR4(aProtocolContext, &aSessionID);

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if( (sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED) && (cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }

    return IDE_FAILURE;
}

/* BUG-38496 Notify users when their password expiry date is approaching */
static IDE_RC answerConnectExResult( cmiProtocolContext *aProtocolContext, 
                                     mmcSessID aSessionID, 
                                     qciUserInfo *aUserInfo)
{
    UInt               sRemainingDays   = 0;
    UInt               sErrorCode;
    ULong              sReserved        = 0;
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK(aProtocolContext, 21);
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP(aProtocolContext, CMP_OP_DB_ConnectExResult);
    CMI_WR4(aProtocolContext, &aSessionID);

    sRemainingDays = qci::getRemainingGracePeriod( aUserInfo );
    if( sRemainingDays > 0 )
    {
        sErrorCode = mmERR_IGNORE_PASSWORD_GRACE_PERIOD;
    }
    else
    {
        sErrorCode = mmERR_IGNORE_NO_ERROR;
    }

    CMI_WR4(aProtocolContext, &sErrorCode);
    CMI_WR4(aProtocolContext, &sRemainingDays);
    /* reserved */
    CMI_WR8(aProtocolContext, (ULong *)&sReserved);

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if( (sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED) && (cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }

    return IDE_FAILURE;
}

static IDE_RC answerDisconnectResult(cmiProtocolContext *aProtocolContext)
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK(aProtocolContext, 1);
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP(aProtocolContext, CMP_OP_DB_DisconnectResult);

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if( (sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED) && (cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }

    return IDE_FAILURE;
}

static IDE_RC answerPropertyGetResult(cmiProtocolContext *aProtocolContext,
                                      mmcSession         *aSession,
                                      UShort              aPropertyID,
                                      idBool             *aIsUnsupportedProperty)
{
    mmcSessionInfo    *sInfo            = aSession->getInfo();
    SChar             *sDBCharSet       = NULL;
    SChar             *sNationalCharSet = NULL;
    UInt               sLen;
    UShort             sOrgCursor       = aProtocolContext->mWriteBlock->mCursor;
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    switch (aPropertyID)
    {
        case CMP_DB_PROPERTY_NLS:
            sLen = idlOS::strlen(sInfo->mNlsUse);

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7 + sLen);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sLen);
            CMI_WCP(aProtocolContext, sInfo->mNlsUse, sLen);
            break;

        case CMP_DB_PROPERTY_AUTOCOMMIT:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 4);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR1(aProtocolContext, (sInfo->mCommitMode == MMC_COMMITMODE_AUTOCOMMIT) ? 1 : 0);
            break;

        case CMP_DB_PROPERTY_EXPLAIN_PLAN:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 4);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR1(aProtocolContext, sInfo->mExplainPlan);
            break;

        case CMP_DB_PROPERTY_ISOLATION_LEVEL: /* BUG-39817 */

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mIsolationLevel);
            break;

        case CMP_DB_PROPERTY_OPTIMIZER_MODE:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mOptimizerMode);
            break;

        case CMP_DB_PROPERTY_HEADER_DISPLAY_MODE:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mHeaderDisplayMode);
            break;

        case CMP_DB_PROPERTY_STACK_SIZE:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mStackSize);
            break;

        case CMP_DB_PROPERTY_NORMALFORM_MAXIMUM:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mNormalFormMaximum);
            break;

        case CMP_DB_PROPERTY_IDLE_TIMEOUT:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mIdleTimeout);
            break;

        case CMP_DB_PROPERTY_QUERY_TIMEOUT:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mQueryTimeout);
            break;

        /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
        case CMP_DB_PROPERTY_DDL_TIMEOUT:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mDdlTimeout);
            break;

        case CMP_DB_PROPERTY_FETCH_TIMEOUT:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mFetchTimeout);
            break;

        case CMP_DB_PROPERTY_UTRANS_TIMEOUT:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mUTransTimeout);
            break;

        case CMP_DB_PROPERTY_DATE_FORMAT:
            sLen = idlOS::strlen(sInfo->mDateFormat);

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7 + sLen);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sLen);
            CMI_WCP(aProtocolContext, sInfo->mDateFormat, sLen);
            break;

        // fix BUG-18971
        case CMP_DB_PROPERTY_SERVER_PACKAGE_VERSION:
            sLen = idlOS::strlen(iduVersionString);

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7 + sLen);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sLen);
            CMI_WCP(aProtocolContext, iduVersionString, sLen);
            break;

        // PROJ-1579 NCHAR
        case CMP_DB_PROPERTY_NLS_NCHAR_LITERAL_REPLACE:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mNlsNcharLiteralReplace);
            break;

        // PROJ-1579 NCHAR
        case CMP_DB_PROPERTY_NLS_CHARACTERSET:
            sDBCharSet = smiGetDBCharSet();

            sLen = idlOS::strlen(sDBCharSet);

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7 + sLen);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sLen);
            CMI_WCP(aProtocolContext, sDBCharSet, sLen);
            break;

        // PROJ-1579 NCHAR
        case CMP_DB_PROPERTY_NLS_NCHAR_CHARACTERSET:
            sNationalCharSet = smiGetNationalCharSet();

            sLen = idlOS::strlen(sNationalCharSet);

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7 + sLen);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sLen);
            CMI_WCP(aProtocolContext, sNationalCharSet, sLen);
            break;

        // PROJ-1579 NCHAR
        case CMP_DB_PROPERTY_ENDIAN:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 4);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR1(aProtocolContext, (iduBigEndian == ID_TRUE) ? 1 : 0);
            break;

        /* PROJ-2047 Strengthening LOB - LOBCACHE */
        case CMP_DB_PROPERTY_LOB_CACHE_THRESHOLD:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mLobCacheThreshold);

            /* BUG-42012 */
            IDU_FIT_POINT_RAISE( "answerPropertyGetResult::LobCacheThreshold",
                                 UnsupportedProperty );
            break;

        case CMP_DB_PROPERTY_REMOVE_REDUNDANT_TRANSMISSION:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 7);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR4(aProtocolContext, &sInfo->mRemoveRedundantTransmission);
            break;

            /* PROJ-2436 ADO.NET MSDTC */
        case CMP_DB_PROPERTY_TRANS_ESCALATION:

            sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
            CMI_WRITE_CHECK(aProtocolContext, 4);
            sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

            CMI_WOP(aProtocolContext, CMP_OP_DB_PropertyGetResult);
            CMI_WR2(aProtocolContext, &aPropertyID);
            CMI_WR1(aProtocolContext, 0); /* dummy value */
            break;

        /* 서버-클라이언트(Session) 프로퍼티는 FIT 테스트를 추가하자. since 2015.07.09 */

        default:
            /* BUG-36256 Improve property's communication */
            IDE_RAISE(UnsupportedProperty);
            break;
    }

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION(UnsupportedProperty)
    {
        *aIsUnsupportedProperty = ID_TRUE;
    }

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if( (sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED) && (cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }
    aProtocolContext->mWriteBlock->mCursor = sOrgCursor;

    return IDE_FAILURE;
}

static IDE_RC answerPropertySetResult(cmiProtocolContext *aProtocolContext)
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK(aProtocolContext, 1);
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP(aProtocolContext, CMP_OP_DB_PropertySetResult);

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if( (sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED) && (cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }

    return IDE_FAILURE;
}

IDE_RC connectProtocolCore(cmiProtocolContext *aProtocolContext,
                           qciUserInfo        *aUserInfo,
                           void               *aSessionOwner )
{
    mmcTask          *sTask   = (mmcTask *)aSessionOwner;
    mmcSession       *sSession;
    SChar             sDbmsName[IDP_MAX_PROP_DBNAME_LEN + 1] = {'\0', };
    SChar             sUserName[QC_MAX_OBJECT_NAME_LEN + 1] = {'\0', };
    SChar             sPassword[QC_MAX_NAME_LEN + 1] = {'\0', };
    UShort            sDbmsNameLen;
    UShort            sUserNameLen;
    UShort            sPasswordLen;
    UShort            sMode;
    UShort            sOrgCursor = aProtocolContext->mReadBlock->mCursor;

    /* TASK-5894 Permit sysdba via IPC */
    idBool            sHasSysdbaViaIPC = ID_FALSE;
    idBool            sIsSysdba        = ID_FALSE;
    SChar             sErrMsg[MAX_ERROR_MSG_LEN] = {'\0', };
    cmiLink          *sLink            = NULL;

    /* PROJ-2446 */
    idvSQL           *sStatistics = NULL;

    IDE_CLEAR();

    IDE_TEST_RAISE(sTask == NULL, NoTask);

    idlOS::memset(aUserInfo, 0, ID_SIZEOF(qciUserInfo));

    CMI_RD2(aProtocolContext, &sDbmsNameLen);
    if (sDbmsNameLen > IDP_MAX_PROP_DBNAME_LEN)
    {
        IDE_RAISE(DbmsNameTooLong);
    }
    else if (sDbmsNameLen > 0)
    {
        CMI_RCP(aProtocolContext, sDbmsName, sDbmsNameLen);
        // BUGBUG (2013-01-23) catalog를 지원하지 않으므로 property에 설정된 db name과 단순 비교한다.
        // 나중에 catalog를 지원하게 되면, user처럼 확인해야 할 것이다.
        IDE_TEST_RAISE(idlOS::strcasecmp(sDbmsName, mmuProperty::getDbName())
                       != 0, DbmsNotFound);
    }
    else
    {
        // use default dbms
        // BUGBUG (2013-01-23) connect 인자로 db name을 받지 않는 CLI를 위해, 비어있으면 기본 dbms로 연결해준다.
        // 나중에 catalog를 지원하게 되면, 기본으로 사용자가 속한 catalog를 쓰도록 해야 할 것이다.
    }

    CMI_RD2(aProtocolContext, &sUserNameLen);
    if ( sUserNameLen > QC_MAX_OBJECT_NAME_LEN )
    {
        IDE_RAISE(UserNameTooLong);
    }
    else
    {
        CMI_RCP(aProtocolContext, sUserName, sUserNameLen);
    }

    CMI_RD2(aProtocolContext, &sPasswordLen);
    /* BUG-39382 To check password's length is not correct */
    if ( sPasswordLen > QC_MAX_NAME_LEN )
    {
        IDE_RAISE(PasswordTooLong);
    }
    else
    {
        CMI_RCP(aProtocolContext, sPassword, sPasswordLen);
    }

    CMI_RD2(aProtocolContext, &sMode);

    /* TASK-5894 Permit sysdba via IPC */
    sIsSysdba = (sMode == CMP_DB_CONNECT_MODE_SYSDBA) ? ID_TRUE : ID_FALSE;
    /* BUG-43654 */
    sHasSysdbaViaIPC = mmtAdminManager::isConnectedViaIPC();

    IDE_TEST_RAISE(cmiPermitConnection(sTask->getLink(),
                                       sHasSysdbaViaIPC,
                                       sIsSysdba)
                   != IDE_SUCCESS, ConnectionNotPermitted);

    // To Fix BUG-17430
    // 무조건 대문자로 변경하면 안됨.
    sUserName[sUserNameLen] = '\0';
    mtl::makeNameInSQL( aUserInfo->loginID, sUserName, sUserNameLen );

    // To fix BUG-21137
    // password에도 double quotation이 올 수 있다.
    // 이를 makeNameInSQL함수로 제거한다.
    sPassword[sPasswordLen] = '\0';
    mtl::makePasswordInSQL( aUserInfo->loginPassword, sPassword, sPasswordLen );

    // PROJ-2002 Column Security
    // login IP(session login IP)는 모든 레코드의 컬럼마다
    // 호출하여 호출 횟수가 많아 IP를 별도로 저장한다.
    if( cmiGetLinkInfo( sTask->getLink(),
                        aUserInfo->loginIP,
                        QCI_MAX_IP_LEN,
                        CMI_LINK_INFO_TCP_REMOTE_IP_ADDRESS )
        == IDE_SUCCESS )
    {
        aUserInfo->loginIP[QCI_MAX_IP_LEN] = '\0';
    }
    else
    {
        idlOS::snprintf( aUserInfo->loginIP,
                         QCI_MAX_IP_LEN,
                         "127.0.0.1" );
        aUserInfo->loginIP[QCI_MAX_IP_LEN] = '\0';
    }

    aUserInfo->mUsrDN         = NULL;
    aUserInfo->mSvrDN         = NULL;
    aUserInfo->mCheckPassword = ID_TRUE;

    aUserInfo->mIsSysdba = sIsSysdba;

    /* BUG-40505 TCP user connect fail in windows 2008 */
    aUserInfo->mDisableTCP = QCI_DISABLE_TCP_FALSE;

    if (mmm::getCurrentPhase() == MMM_STARTUP_SERVICE)
    {
        /* >> PROJ-2446 */
        sSession = sTask->getSession();

        if (sSession != NULL)
        {
            sStatistics = sSession->getStatSQL();
        }
        else
        {
            /* do nothing */
        }

        IDE_TEST( mmtServiceThread::getUserInfoFromDB( sStatistics, 
                                                       aUserInfo ) 
                  != IDE_SUCCESS );
        /* << PROJ-2446 */
    }
    else
    {
        IDE_TEST_RAISE(aUserInfo->mIsSysdba != ID_TRUE, OnlyAdminAcceptable);

        IDE_TEST(mmtServiceThread::getUserInfoFromFile(aUserInfo) != IDE_SUCCESS);
    }

    /* PROJ-2474 SSL/TLS */
    IDE_TEST(cmiGetLinkForProtocolContext(aProtocolContext, &sLink));
    aUserInfo->mConnectType = (qciConnectType)sLink->mImpl;

    IDE_TEST(sTask->authenticate(aUserInfo) != IDE_SUCCESS);

    /*
     * Session 생성
     */

    IDE_TEST(mmtSessionManager::allocSession(sTask, aUserInfo->mIsSysdba) != IDE_SUCCESS);

    /*
     * Session 상태 확인
     */

    sSession = sTask->getSession();

    // proj_2160 cm_type removal
    // set sessionID at packet header.
    // this value in the header is currently not used.
    aProtocolContext->mWriteHeader.mA7.mSessionID =
        (UShort) sSession->getSessionID();

    IDE_TEST_RAISE(sSession->getSessionState() >= MMC_SESSION_STATE_AUTH, AlreadyConnectedError);

    /*
     * Session에 로그인정보 저장
     */

    sTask->getSession()->setUserInfo(aUserInfo);

    sSession->setSessionState(MMC_SESSION_STATE_AUTH);

    IDV_SESS_ADD(sTask->getSession()->getStatistics(), IDV_STAT_INDEX_LOGON_CUMUL, 1);
    IDV_SESS_ADD(sTask->getSession()->getStatistics(), IDV_STAT_INDEX_LOGON_CURR, 1);

    return IDE_SUCCESS;

    IDE_EXCEPTION(NoTask);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_SESSION_NOT_SPECIFIED));
    }
    IDE_EXCEPTION(DbmsNameTooLong);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_TOO_LONG_IDENTIFIER_NAME));
    }
    IDE_EXCEPTION(DbmsNotFound);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_DBMS_NOT_FOUND, sDbmsName));
    }
    IDE_EXCEPTION(UserNameTooLong);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_TOO_LONG_IDENTIFIER_NAME));
    }
    IDE_EXCEPTION(PasswordTooLong);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_TOO_LONG_IDENTIFIER_NAME));
    }
    IDE_EXCEPTION(OnlyAdminAcceptable);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_ADMIN_MODE_ONLY));
    }
    IDE_EXCEPTION(AlreadyConnectedError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_ALREADY_CONNECT_ERROR));
    }
    IDE_EXCEPTION( ConnectionNotPermitted )
    {
        idlOS::snprintf( sErrMsg, ID_SIZEOF(sErrMsg), ideGetErrorMsg() );
        IDE_SET( ideSetErrorCode(mmERR_ABORT_FAIL_TO_ESTABLISH_CONNECTION, sErrMsg) );
    }

    IDE_EXCEPTION_END;

    /* PROJ-2160 CM 타입제거
       프로토콜이 읽는 도중에 실패해도 모두 읽어야 한다. */
    aProtocolContext->mReadBlock->mCursor = sOrgCursor;

    CMI_RD2(aProtocolContext, &sDbmsNameLen);
    CMI_SKIP_READ_BLOCK(aProtocolContext, sDbmsNameLen);
    CMI_RD2(aProtocolContext, &sUserNameLen);
    CMI_SKIP_READ_BLOCK(aProtocolContext, sUserNameLen);
    CMI_RD2(aProtocolContext, &sPasswordLen);
    CMI_SKIP_READ_BLOCK(aProtocolContext, sPasswordLen);
    CMI_RD2(aProtocolContext, &sMode);

    IDE_DASSERT( ideIsIgnore() != IDE_SUCCESS ) /*BUG-40961*/

    return IDE_FAILURE;
}

IDE_RC mmtServiceThread::connectProtocol(cmiProtocolContext *aProtocolContext,
                                         cmiProtocol        * /* aProtocol */,
                                         void               *aSessionOwner,
                                         void               *aUserContext)
{
        IDE_RC            sRet;
        mmtServiceThread *sThread  = (mmtServiceThread *)aUserContext;
        mmcSession       *sSession = NULL;
        qciUserInfo       sUserInfo;

        /* BUG-41986 */
        idvAuditTrail     sAuditTrail;
        UInt              sResultCode = 0;
        
        sRet = connectProtocolCore( aProtocolContext, 
                                    &sUserInfo,
                                    aSessionOwner );
        
        if( sRet == IDE_SUCCESS && sThread->mErrorFlag != ID_TRUE )
        {
            sSession = ((mmcTask *)aSessionOwner)->getSession();

            /* PROJ-2177 User Interface - Cancel
             * 중복되지 않는 StmtCID 생성을 위해 SessionID 참조 */
            sRet = answerConnectResult(aProtocolContext, sSession->getSessionID());

            /* Set the error code for auditing */
            sResultCode = (sRet == IDE_SUCCESS) ? 0 : E_ERROR_CODE(ideGetErrorCode());
        }
        else
        {
            /* Set the error code for auditing first 
               before a cm error happends and overwrites the error. */
            sResultCode = E_ERROR_CODE(ideGetErrorCode());

            /* In case the connectProtocolCore() has failed. */
            sRet = sThread->answerErrorResult( aProtocolContext, CMI_PROTOCOL_OPERATION(DB, Connect), 0 );
            if ( sRet == IDE_SUCCESS )
            {
                sThread->setErrorFlag( ID_TRUE );
            }
            else
            {
                /* No error has been set in this thread
                 * since the server couldn't send the error result to the client. */
            }
        }

        /* BUG-41986 */
        IDE_TEST_CONT( mmtAuditManager::isAuditStarted() != ID_TRUE, AUDIT_NOT_STARTED );

        mmtAuditManager::initAuditConnInfo( ((mmcTask *)aSessionOwner)->getSession(),
                                            &sAuditTrail,
                                            &sUserInfo,
                                            sResultCode,
                                            QCI_AUDIT_OPER_CONNECT );

        mmtAuditManager::auditConnectInfo( &sAuditTrail );

        IDE_EXCEPTION_CONT( AUDIT_NOT_STARTED );

        return sRet;
}

IDE_RC mmtServiceThread::connectExProtocol(cmiProtocolContext *aProtocolContext,
                                           cmiProtocol        * /* aProtocol */,
                                           void               *aSessionOwner,
                                           void               *aUserContext)
{
        IDE_RC            sRet;
        mmtServiceThread *sThread = (mmtServiceThread *)aUserContext;
        mmcSession       *sSession = NULL;

        /* BUG-41986 */
        idvAuditTrail     sAuditTrail;
        qciUserInfo       sUserInfo;
        UInt              sResultCode = 0;


        sRet = connectProtocolCore( aProtocolContext, 
                                    &sUserInfo,
                                    aSessionOwner );
        
        if( sRet == IDE_SUCCESS && sThread->mErrorFlag != ID_TRUE )
        {
            sSession = ((mmcTask *)aSessionOwner)->getSession();

            /* PROJ-2177 User Interface - Cancel
             * 중복되지 않는 StmtCID 생성을 위해 SessionID 참조 */
            sRet = answerConnectExResult(aProtocolContext, sSession->getSessionID(), sSession->getUserInfo());

            sResultCode = (sRet == IDE_SUCCESS) ? 0 : E_ERROR_CODE(ideGetErrorCode());
        }
        else
        {
            /* Set the error code for auditing first 
               before a cm error happends and overwrites the error. */
            sResultCode = E_ERROR_CODE(ideGetErrorCode());

            /* In case the connectProtocolCore() has failed. */
            sRet = sThread->answerErrorResult( aProtocolContext, CMI_PROTOCOL_OPERATION(DB, Connect), 0 );
            if ( sRet == IDE_SUCCESS )
            {
                sThread->setErrorFlag( ID_TRUE );
            }
            else
            {
                /* No error has been set in this thread
                 * since the server couldn't send the error result to the client. */
            }
        }

        /* BUG-41986 */
        IDE_TEST_CONT( mmtAuditManager::isAuditStarted() != ID_TRUE, AUDIT_NOT_STARTED );

        mmtAuditManager::initAuditConnInfo( ((mmcTask *)aSessionOwner)->getSession(),
                                            &sAuditTrail, 
                                            &sUserInfo,
                                            sResultCode,
                                            QCI_AUDIT_OPER_CONNECT );

        mmtAuditManager::auditConnectInfo( &sAuditTrail );

        IDE_EXCEPTION_CONT( AUDIT_NOT_STARTED );

        return sRet;
}

IDE_RC mmtServiceThread::disconnectProtocol(cmiProtocolContext *aProtocolContext,
                                            cmiProtocol        * /*aProtocol*/,
                                            void               *aSessionOwner,
                                            void               *aUserContext)
{
    mmcTask          *sTask = (mmcTask *)aSessionOwner;
    mmtServiceThread *sThread = (mmtServiceThread *)aUserContext;
    mmcSession       *sSession = NULL;
    IDE_RC            sRet = IDE_FAILURE;

    /* BUG-41986 */
    idvAuditTrail     sAuditTrail;

    /* PROJ-2160 CM 타입제거
       모두 읽은 다음에 프로토콜을 처리해야 한다. */
    CMI_SKIP_READ_BLOCK(aProtocolContext, 1);

    IDE_CLEAR();

    IDE_TEST(findSession(sTask, &sSession, sThread) != IDE_SUCCESS);

    IDE_TEST(checkSessionState(sSession, MMC_SESSION_STATE_AUTH) != IDE_SUCCESS);

    aProtocolContext->mIsDisconnect = ID_TRUE;

    /* BUG-41986 
     * Auditing for disconnection must be conducted 
     * before releasing the resources related to the session to get the session information
     */
    IDE_TEST_CONT( mmtAuditManager::isAuditStarted() != ID_TRUE, AUDIT_NOT_STARTED );

    mmtAuditManager::initAuditConnInfo( sSession, 
                                        &sAuditTrail, 
                                        sSession->getUserInfo(),
                                        0,  /* success */
                                        QCI_AUDIT_OPER_DISCONNECT );

    mmtAuditManager::auditConnectInfo( &sAuditTrail );

    IDE_EXCEPTION_CONT( AUDIT_NOT_STARTED );

    /* shard connection인 경우 disconnect시 rollback한다. */
    if (sSession->isShardData() == ID_TRUE )
    {
        /* freeSession에서 rollback한다. */
    }
    else
    {
        sSession->setSessionState(MMC_SESSION_STATE_END);

        /* BUG-38585 IDE_ASSERT remove */
        IDU_FIT_POINT("mmtServiceThread::disconnectProtocol::EndSession");
        IDE_TEST(sSession->endSession() != IDE_SUCCESS);
    }

    IDE_TEST(mmtSessionManager::freeSession(sTask) != IDE_SUCCESS);

    sRet = answerDisconnectResult(aProtocolContext);

    return sRet;

    IDE_EXCEPTION_END;

    /* BUG-41986 */
    IDE_TEST_CONT( mmtAuditManager::isAuditStarted() != ID_TRUE, AUDIT_NOT_STARTED_FOR_ERR_RESULT );

    mmtAuditManager::initAuditConnInfo( sSession, 
                                        &sAuditTrail, 
                                        sSession->getUserInfo(), 
                                        E_ERROR_CODE(ideGetErrorCode()),
                                        QCI_AUDIT_OPER_DISCONNECT );

    mmtAuditManager::auditConnectInfo( &sAuditTrail );

    IDE_EXCEPTION_CONT( AUDIT_NOT_STARTED_FOR_ERR_RESULT );

    sRet = sThread->answerErrorResult(aProtocolContext,
                                      CMI_PROTOCOL_OPERATION(DB, Disconnect),
                                      0);
    
    return sRet;
}

IDE_RC mmtServiceThread::propertyGetProtocol(cmiProtocolContext *aProtocolContext,
                                             cmiProtocol        *,
                                             void               *aSessionOwner,
                                             void               *aUserContext)
{
    mmcTask             *sTask = (mmcTask *)aSessionOwner;
    mmtServiceThread    *sThread = (mmtServiceThread *)aUserContext;
    mmcSession          *sSession;
    UShort               sPropertyID;
    idBool               sIsUnsupportedProperty = ID_FALSE;
    UInt                 sErrorIndex = 0;

    CMI_RD2(aProtocolContext, &sPropertyID);

    IDE_CLEAR();

    IDE_TEST(findSession(sTask, &sSession, sThread) != IDE_SUCCESS);

    IDE_TEST(checkSessionState(sSession, MMC_SESSION_STATE_AUTH) != IDE_SUCCESS);

    IDE_TEST(answerPropertyGetResult(aProtocolContext,
                                     sSession,
                                     sPropertyID,
                                     &sIsUnsupportedProperty) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if (sIsUnsupportedProperty == ID_TRUE)
    {
        /*
         * BUG-36256 Improve property's communication
         *
         * ulnCallbackDBPropertySetResult 함수를 이용할 수 없기에 Get()도
         * 통일성을 위해 answerErrorResult를 이용해 에러를 발생시키지 않고
         * Client에게 응답을 준다.
         */
        ideLog::log(IDE_MM_0,
                    MM_TRC_GET_UNSUPPORTED_PROPERTY,
                    sPropertyID);

        IDE_SET(ideSetErrorCode(mmERR_IGNORE_UNSUPPORTED_PROPERTY, sPropertyID));

        sErrorIndex = sPropertyID;
    }
    else
    {
        /* Nothing */
    }

    return sThread->answerErrorResult(aProtocolContext,
                                      CMI_PROTOCOL_OPERATION(DB, PropertyGet),
                                      sErrorIndex);
}

IDE_RC mmtServiceThread::propertySetProtocol(cmiProtocolContext *aProtocolContext,
                                             cmiProtocol        *aProtocol,
                                             void               *aSessionOwner,
                                             void               *aUserContext)
{
    mmcTask             *sTask   = (mmcTask *)aSessionOwner;
    mmtServiceThread    *sThread = (mmtServiceThread *)aUserContext;
    mmcSession          *sSession;
    mmcSessionInfo      *sInfo;
    UInt                 sLen;
    UInt                 sMaxLen;
    UChar                sBool;
    UInt                 sValue4; /* BUG-39817 */
    ULong                sValue8;
    UShort               sPropertyID;
    UShort               sOrgCursor;
    UInt                 sValueLen   = 0;
    UInt                 sErrorIndex = 0;

    CMI_RD2(aProtocolContext, &sPropertyID);

    /* BUG-41793 Keep a compatibility among tags */
    switch (aProtocol->mOpID)
    {
        /* CMP_OP_DB_PropertySetV2(1) | PropetyID(2) | ValueLen(4) | Value(?) */
        case CMP_OP_DB_PropertySetV2:
            CMI_RD4(aProtocolContext, &sValueLen);
            break;

        /* CMP_OP_DB_PropertySet  (1) | PropetyID(2) | Value(?) */
        case CMP_OP_DB_PropertySet:
        default:
            break;
    }

    sOrgCursor = aProtocolContext->mReadBlock->mCursor;

    IDE_CLEAR();

    IDE_TEST(findSession(sTask, &sSession, sThread) != IDE_SUCCESS);

    IDE_TEST(checkSessionState(sSession, MMC_SESSION_STATE_AUTH) != IDE_SUCCESS);

    sInfo = sSession->getInfo();

    switch (sPropertyID)
    {
        case CMP_DB_PROPERTY_CLIENT_PACKAGE_VERSION:
            IDE_TEST_RAISE(sSession->getSessionState() != MMC_SESSION_STATE_AUTH,
                           InvalidSessionState);

            CMI_RD4(aProtocolContext, &sLen);
            sMaxLen = ID_SIZEOF(sInfo->mClientPackageVersion) - 1;
            IDE_TEST_RAISE( sLen > sMaxLen, InvalidLengthError );
            CMI_RCP(aProtocolContext, sInfo->mClientPackageVersion, sLen);
            sInfo->mClientPackageVersion[sLen] = 0;

            sSession->changeSessionStateService();

            break;

        case CMP_DB_PROPERTY_CLIENT_PROTOCOL_VERSION:
            IDE_TEST_RAISE(sSession->getSessionState() != MMC_SESSION_STATE_AUTH,
                           InvalidSessionState);

            CMI_RD8(aProtocolContext, &sValue8);

            idlOS::snprintf(sInfo->mClientProtocolVersion,
                            ID_SIZEOF(sInfo->mClientProtocolVersion),
                            "%" ID_UINT32_FMT ".%" ID_UINT32_FMT ".%" ID_UINT32_FMT,
                            CM_GET_MAJOR_VERSION(sValue8),
                            CM_GET_MINOR_VERSION(sValue8),
                            CM_GET_PATCH_VERSION(sValue8));

            sSession->changeSessionStateService();

            break;

        case CMP_DB_PROPERTY_CLIENT_PID:
            IDE_TEST_RAISE(sSession->getSessionState() != MMC_SESSION_STATE_AUTH,
                           InvalidSessionState);

            CMI_RD8(aProtocolContext, &(sInfo->mClientPID));

            sSession->changeSessionStateService();

            break;

        case CMP_DB_PROPERTY_CLIENT_TYPE:
            IDE_TEST_RAISE(sSession->getSessionState() != MMC_SESSION_STATE_AUTH,
                           InvalidSessionState);

            CMI_RD4(aProtocolContext, &sLen);
            sMaxLen = ID_SIZEOF(sInfo->mClientType) - 1;
            IDE_TEST_RAISE( sLen > sMaxLen, InvalidLengthError );
            CMI_RCP(aProtocolContext, sInfo->mClientType, sLen);
            sInfo->mClientType[sLen] = 0;

            sSession->changeSessionStateService();

            break;

        case CMP_DB_PROPERTY_APP_INFO:
            CMI_RD4(aProtocolContext, &sLen);
            sMaxLen = ID_SIZEOF(sInfo->mClientAppInfo) - 1;
            IDE_TEST_RAISE( sLen > sMaxLen, InvalidLengthError );
            CMI_RCP(aProtocolContext, sInfo->mClientAppInfo, sLen);
            sInfo->mClientAppInfo[sLen] = 0;

            /* PROJ-2626 Snapshot Export
             * iloader 인지 아닌지를 구분해야 될 경우 매번 string compare를 하기
             * 보다 미리 값을 정해 놓는다.
             */
            if ( ( sLen == 7 ) &&
                 ( idlOS::strncmp( sInfo->mClientAppInfo, "iloader", sLen ) == 0 ) )
            {
                sInfo->mClientAppInfoType = MMC_CLIENT_APP_INFO_TYPE_ILOADER;
            }
            else
            {
                /* Nothing to do */
            }
            break;

        case CMP_DB_PROPERTY_NLS:
            IDE_TEST_RAISE(sSession->getSessionState() != MMC_SESSION_STATE_AUTH,
                           InvalidSessionState);

            CMI_RD4(aProtocolContext, &sLen);
            sMaxLen = ID_SIZEOF(sInfo->mNlsUse) - 1;
            IDE_TEST_RAISE( sLen > sMaxLen, InvalidLengthError );
            CMI_RCP(aProtocolContext, sInfo->mNlsUse, sLen);
            sInfo->mNlsUse[sLen] = 0;

            IDE_TEST(sSession->findLanguage() != IDE_SUCCESS);

            sSession->changeSessionStateService();

            break;

        case CMP_DB_PROPERTY_AUTOCOMMIT:
            CMI_RD1(aProtocolContext, sBool);

            /* BUG-21230 */
            IDE_TEST_RAISE(sSession->getXaAssocState() != MMD_XA_ASSOC_STATE_NOTASSOCIATED,
                           DCLNotAllowedError);

            IDE_TEST(sSession->setCommitMode(sBool ?
                                             MMC_COMMITMODE_AUTOCOMMIT :
                                             MMC_COMMITMODE_NONAUTOCOMMIT) != IDE_SUCCESS);

            break;

        case CMP_DB_PROPERTY_EXPLAIN_PLAN:
            CMI_RD1(aProtocolContext, sBool);

            sSession->setExplainPlan(sBool);
            break;

        case CMP_DB_PROPERTY_ISOLATION_LEVEL: /* BUG-39817 */
            CMI_RD4(aProtocolContext, &sValue4);

            IDE_TEST(sSession->setTX(SMI_ISOLATION_MASK, sValue4, ID_TRUE));
            break;

        case CMP_DB_PROPERTY_OPTIMIZER_MODE:
            CMI_RD4(aProtocolContext, &(sInfo->mOptimizerMode));
            break;

        case CMP_DB_PROPERTY_HEADER_DISPLAY_MODE:
            CMI_RD4(aProtocolContext, &(sInfo->mHeaderDisplayMode));
            break;

        case CMP_DB_PROPERTY_STACK_SIZE:
            CMI_RD4(aProtocolContext, &(sInfo->mStackSize));
            break;

        case CMP_DB_PROPERTY_IDLE_TIMEOUT:
            CMI_RD4(aProtocolContext, &(sInfo->mIdleTimeout));
            break;

        case CMP_DB_PROPERTY_QUERY_TIMEOUT:
            CMI_RD4(aProtocolContext, &(sInfo->mQueryTimeout));
            break;

        /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
        case CMP_DB_PROPERTY_DDL_TIMEOUT:
            CMI_RD4(aProtocolContext, &(sInfo->mDdlTimeout));
            break;

        case CMP_DB_PROPERTY_FETCH_TIMEOUT:
            CMI_RD4(aProtocolContext, &(sInfo->mFetchTimeout));
            break;

        case CMP_DB_PROPERTY_UTRANS_TIMEOUT:
            CMI_RD4(aProtocolContext, &(sInfo->mUTransTimeout));
            break;

        case CMP_DB_PROPERTY_DATE_FORMAT:
            CMI_RD4(aProtocolContext, &sLen);
            sMaxLen = ID_SIZEOF(sInfo->mDateFormat) - 1;
            IDE_TEST_RAISE( sLen > sMaxLen, InvalidLengthError );
            CMI_RCP(aProtocolContext, sInfo->mDateFormat, sLen);
            sInfo->mDateFormat[sLen] = 0;

            break;
            
        /* PROJ-2209 DBTIMEZONE */
        case CMP_DB_PROPERTY_TIME_ZONE:

            CMI_RD4(aProtocolContext, &sLen);
            sMaxLen = ID_SIZEOF(sInfo->mTimezoneString) - 1;
            IDE_TEST_RAISE( sLen > sMaxLen, InvalidLengthError );

            CMI_RCP(aProtocolContext, sInfo->mTimezoneString, sLen);
            sInfo->mTimezoneString[sLen] = 0;

            if ( idlOS::strCaselessMatch( sInfo->mTimezoneString, "DB_TZ" ) == 0 )
            {
                idlOS::memcpy( sInfo->mTimezoneString, MTU_DB_TIMEZONE_STRING, MTU_DB_TIMEZONE_STRING_LEN );
                sInfo->mTimezoneString[MTU_DB_TIMEZONE_STRING_LEN] = 0;
                sInfo->mTimezoneSecond = MTU_DB_TIMEZONE_SECOND;
            }
            else
            {
                IDE_TEST ( mtz::getTimezoneSecondAndString( sInfo->mTimezoneString,
                                                            &sInfo->mTimezoneSecond,
                                                            sInfo->mTimezoneString )
                           != IDE_SUCCESS );
            }
            break;

        // PROJ-1579 NCHAR
        case CMP_DB_PROPERTY_NLS_NCHAR_LITERAL_REPLACE:
            CMI_RD4(aProtocolContext, &sInfo->mNlsNcharLiteralReplace);
            break;

        /* BUG-31144 */
        case CMP_DB_PROPERTY_MAX_STATEMENTS_PER_SESSION:
            CMI_RD4(aProtocolContext, &sLen);
            IDE_TEST_RAISE(sLen < sSession->getNumberOfStatementsInSession(), StatementNumberExceedsInputValue);

            sInfo->mMaxStatementsPerSession = sLen;
            break;
        /* BUG-31390 Failover info for v$session */
        case CMP_DB_PROPERTY_FAILOVER_SOURCE:
            CMI_RD4(aProtocolContext, &sLen);
            sMaxLen = ID_SIZEOF(sInfo->mFailOverSource) - 1;
            IDE_TEST_RAISE( sLen > sMaxLen, InvalidLengthError );
            CMI_RCP(aProtocolContext, sInfo->mFailOverSource, sLen);
            sInfo->mFailOverSource[sLen] = 0;
            break;

        /* PROJ-2047 Strengthening LOB - LOBCACHE */
        case CMP_DB_PROPERTY_LOB_CACHE_THRESHOLD:
            CMI_RD4(aProtocolContext, &sInfo->mLobCacheThreshold);

            /* BUG-42012 */
            IDU_FIT_POINT_RAISE( "mmtServiceThread::propertySetProtocol::LobCacheThreshold",
                                 UnsupportedProperty );
            break;

        // PROJ-2331
        case CMP_DB_PROPERTY_REMOVE_REDUNDANT_TRANSMISSION:
            if (idlOS::strncmp(sInfo->mClientType, "NEW_JDBC", 8) == 0)
            {
                CMI_RD4(aProtocolContext, &sInfo->mRemoveRedundantTransmission );
            }
            else
            {
                CMI_SKIP_READ_BLOCK(aProtocolContext, 4);
            }
            break;

            /* PROJ-2436 ADO.NET MSDTC */
        case CMP_DB_PROPERTY_TRANS_ESCALATION:
            CMI_RD1(aProtocolContext, sBool);

            sSession->setTransEscalation(sBool ?
                                         MMC_TRANS_ESCALATE :
                                         MMC_TRANS_DO_NOT_ESCALATE);

            break;

            /* PROJ-2638 shard native linker */
        case CMP_DB_PROPERTY_SHARD_LINKER_TYPE:
            CMI_RD1( aProtocolContext, sBool );
            if ( sBool > 0 )
            {
                IDE_TEST( sSession->setShardLinker() != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }
            break;

            /* PROJ-2638 shard native linker */
        case CMP_DB_PROPERTY_SHARD_NODE_NAME:
            CMI_RD4( aProtocolContext, &sLen );
            sMaxLen = ID_SIZEOF(sInfo->mShardNodeName) - 1;
            IDE_TEST_RAISE( sLen > sMaxLen, InvalidLengthError );
            CMI_RCP( aProtocolContext, sInfo->mShardNodeName, sLen );
            sInfo->mShardNodeName[sLen] = '\0';
            break;

            /* PROJ-2660 hybrid sharding */
        case CMP_DB_PROPERTY_SHARD_PIN:
            CMI_RD8( aProtocolContext, &sInfo->mShardPin );
            break;

        case CMP_DB_PROPERTY_DBLINK_GLOBAL_TRANSACTION_LEVEL:
            CMI_RD1( aProtocolContext, sBool );
            IDE_TEST( dkiSessionSetGlobalTransactionLevel(
                          sSession->getDatabaseLinkSession(),
                          (UInt)sBool )
                      != IDE_SUCCESS );
            sSession->setDblinkGlobalTransactionLevel((UInt)sBool);
            break;

        /* 서버-클라이언트(Session) 프로퍼티는 FIT 테스트를 추가하자. since 2015.07.09 */

        default:
            /* BUG-36256 Improve property's communication */
            IDE_RAISE(UnsupportedProperty);
            break;
    }

    return answerPropertySetResult(aProtocolContext);

    IDE_EXCEPTION(DCLNotAllowedError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_NOT_ALLOWED_DCL));
    }
    IDE_EXCEPTION(InvalidSessionState);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_SESSION_STATE));
    }
    IDE_EXCEPTION(InvalidLengthError)
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_VALUE_LENGTH_EXCEED,
                                cmpGetDbPropertyName(sPropertyID), sMaxLen));
    }

    /* BUG-31144 */
    IDE_EXCEPTION(StatementNumberExceedsInputValue);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_STATEMENT_NUMBER_EXCEEDS_INPUT_VALUE));
    }

    IDE_EXCEPTION(UnsupportedProperty)
    {
        /*
         * BUG-36256 Improve property's communication
         *
         * CMP_OP_DB_ProprtySetResult는 OP(2)만 보내기 때문에 하위 호환성을
         * 유지하기 위해서는 ulnCallbackDBPropertySetResult 함수를 이용할 수 없다.
         * answerErrorResult를 이용해 에러를 발생시키지 않고 Client에게 응답을 준다.
         * 초기 설계가 아쉬운 부분이다.
         */
        ideLog::log(IDE_MM_0,
                    MM_TRC_SET_UNSUPPORTED_PROPERTY,
                    sPropertyID);

        IDE_SET(ideSetErrorCode(mmERR_IGNORE_UNSUPPORTED_PROPERTY, sPropertyID));

        sErrorIndex = sPropertyID;
    }

    IDE_EXCEPTION_END;

    /* PROJ-2160 CM 타입제거
       프로토콜이 읽는 도중에 실패해도 모두 읽어야 한다. */
    aProtocolContext->mReadBlock->mCursor = sOrgCursor;

    /* BUG-41793 Keep a compatibility among tags */
    if (aProtocol->mOpID == CMP_OP_DB_PropertySetV2)
    {
        CMI_SKIP_READ_BLOCK(aProtocolContext, sValueLen);
    }
    else
    {
        /*
         * 6.3.1과의 하위호환성을 위해 둔다.
         * 프로퍼티가 추가되어도 아래에 추가할 필요가 없다. since CM 7.1.3
         */
        switch (sPropertyID)
        {
            case CMP_DB_PROPERTY_AUTOCOMMIT:
            case CMP_DB_PROPERTY_EXPLAIN_PLAN:
            case CMP_DB_PROPERTY_TRANS_ESCALATION: /* PROJ-2436 ADO.NET MSDTC */
                CMI_SKIP_READ_BLOCK(aProtocolContext, 1);
                break;

            case CMP_DB_PROPERTY_ISOLATION_LEVEL: /* BUG-39817 */
            case CMP_DB_PROPERTY_OPTIMIZER_MODE:
            case CMP_DB_PROPERTY_HEADER_DISPLAY_MODE:
            case CMP_DB_PROPERTY_STACK_SIZE:
            case CMP_DB_PROPERTY_IDLE_TIMEOUT:
            case CMP_DB_PROPERTY_QUERY_TIMEOUT:
            case CMP_DB_PROPERTY_FETCH_TIMEOUT:
            case CMP_DB_PROPERTY_UTRANS_TIMEOUT:
            case CMP_DB_PROPERTY_NLS_NCHAR_LITERAL_REPLACE:
            case CMP_DB_PROPERTY_MAX_STATEMENTS_PER_SESSION:
            case CMP_DB_PROPERTY_DDL_TIMEOUT:
            case CMP_DB_PROPERTY_LOB_CACHE_THRESHOLD:  /* BUG-36966 */
            case CMP_DB_PROPERTY_REMOVE_REDUNDANT_TRANSMISSION:
                CMI_SKIP_READ_BLOCK(aProtocolContext, 4);
                break;

            case CMP_DB_PROPERTY_CLIENT_PACKAGE_VERSION:
            case CMP_DB_PROPERTY_CLIENT_TYPE:
            case CMP_DB_PROPERTY_APP_INFO:
            case CMP_DB_PROPERTY_NLS:
            case CMP_DB_PROPERTY_DATE_FORMAT:
            case CMP_DB_PROPERTY_TIME_ZONE:
            case CMP_DB_PROPERTY_FAILOVER_SOURCE:
                CMI_RD4(aProtocolContext, &sLen);
                CMI_SKIP_READ_BLOCK(aProtocolContext, sLen);
                break;

            case CMP_DB_PROPERTY_CLIENT_PROTOCOL_VERSION:
            case CMP_DB_PROPERTY_CLIENT_PID:
                CMI_SKIP_READ_BLOCK(aProtocolContext, 8);
                break;

            default:
                break;
        }
    }

    return sThread->answerErrorResult(aProtocolContext,
                                      CMI_PROTOCOL_OPERATION(DB, PropertySet),
                                      sErrorIndex);
}

/*********************************************************/
/* proj_2160 cm_type removal
 * new callback for DB Handshake protocol added.
 * old version: BASE handshake */
IDE_RC mmtServiceThread::handshakeProtocol(cmiProtocolContext *aCtx,
                                           cmiProtocol        * /*aProtocol*/,
                                           void               * /*aSessionOwner*/,
                                           void               *aUserContext)
{
    UChar                    sModuleID;      // 1: DB, 2: RP
    UChar                    sMajorVersion;  // CM major ver of client
    UChar                    sMinorVersion;  // CM minor ver of client
    UChar                    sPatchVersion;  // CM patch ver of client
    UChar                    sFlags;         // reserved

    cmpArgBASEHandshake      sArgA5;
    cmpArgBASEHandshake*     sResultA5;
    cmiProtocol              sProtocol;

    mmtServiceThread*        sThread  = (mmtServiceThread *)aUserContext;

    IDE_CLEAR();

    // client is A7 or higher.
    if (cmiGetPacketType(aCtx) != CMP_PACKET_TYPE_A5)
    {
        CMI_RD1(aCtx, sModuleID);
        CMI_RD1(aCtx, sMajorVersion);
        CMI_RD1(aCtx, sMinorVersion);
        CMI_RD1(aCtx, sPatchVersion);
        CMI_RD1(aCtx, sFlags);

        IDE_TEST_RAISE(sModuleID != aCtx->mModule->mModuleID, InvalidModule);
        sModuleID     = CMP_MODULE_DB;
        sMajorVersion = CM_MAJOR_VERSION;
        sMinorVersion = CM_MINOR_VERSION;
        sPatchVersion = CM_PATCH_VERSION;
        sFlags        = 0;

        CMI_WOP(aCtx, CMP_OP_DB_HandshakeResult);
        CMI_WR1(aCtx, sModuleID);
        CMI_WR1(aCtx, sMajorVersion);
        CMI_WR1(aCtx, sMinorVersion);
        CMI_WR1(aCtx, sPatchVersion);
        CMI_WR1(aCtx, sFlags);

        if (cmiGetLinkImpl(aCtx) == CMI_LINK_IMPL_IPCDA)
        {
            /* PROJ-2616 */
            MMT_IPCDA_INCREASE_DATA_COUNT(aCtx);
        }
        else
        {
            IDE_TEST(cmiSend(aCtx, ID_TRUE) != IDE_SUCCESS);
        }
    }
    // client is A5. so let's send a A5 packet through A5 CM.
    else
    {
        CMI_RD1(aCtx, sArgA5.mBaseVersion);
        CMI_RD1(aCtx, sArgA5.mModuleID);
        CMI_RD1(aCtx, sArgA5.mModuleVersion);

        IDE_TEST_RAISE(sArgA5.mModuleID != aCtx->mModule->mModuleID, InvalidModule);

        // initialize writeBlock's header using recved header
        aCtx->mWriteHeader.mA5.mHeaderSign      = CMP_HEADER_SIGN_A5;
        aCtx->mWriteHeader.mA5.mModuleID        = aCtx->mReadHeader.mA5.mModuleID;
        aCtx->mWriteHeader.mA5.mModuleVersion   = aCtx->mReadHeader.mA5.mModuleVersion;
        aCtx->mWriteHeader.mA5.mSourceSessionID = aCtx->mReadHeader.mA5.mTargetSessionID;
        aCtx->mWriteHeader.mA5.mTargetSessionID = aCtx->mReadHeader.mA5.mSourceSessionID;

        // mModule has to be set properly for cmiWriteProtocol
        aCtx->mModule = gCmpModule[CMP_MODULE_BASE];
        CMI_PROTOCOL_INITIALIZE(sProtocol, sResultA5, BASE, Handshake);

        sResultA5->mBaseVersion   = CMP_VER_BASE_MAX - 1; // 1
        sResultA5->mModuleID      = CMP_MODULE_DB;        // 1
        sResultA5->mModuleVersion = CM_PATCH_VERSION;

        IDE_TEST(cmiWriteProtocol(aCtx, &sProtocol) != IDE_SUCCESS);

        IDE_TEST(cmiFlushProtocol(aCtx, ID_TRUE) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(InvalidModule);
    {
        IDE_SET(ideSetErrorCode(cmERR_ABORT_INVALID_MODULE));
    }
    IDE_EXCEPTION_END;

    if (cmiGetPacketType(aCtx) != CMP_PACKET_TYPE_A5)
    {
        return sThread->answerErrorResult(aCtx,
                                          CMI_PROTOCOL_OPERATION(DB, Handshake),
                                          0);
    }
    else
    {
        return IDE_FAILURE;
    }
}
