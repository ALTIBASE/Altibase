/**
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _O_CMP_DEF_RP_H_
#define _O_CMP_DEF_RP_H_ 1


/*
 * 프로토콜 버전업으로 상수를 추가할 때는 다음처럼 추가한다.
 *
 *    enum {
 *        CM_ID_NONE = 0,
 *        ...
 *        CM_ID_MAX_VER1
 *    };  <-- 프로토콜 버전1의 기존 정의
 *
 *    enum {
 *        CM_ID_NEW = CM_ID_MAX_VER1,
 *        ...
 *        CM_ID_MAX_VER2
 *    };  <-- 프로토콜 버전2의 새로운 상수 정의
 *
 *    #define CM_ID_MAX CM_ID_MAX_VER2  <-- 마지막 프로토콜 버전의 MAX로 정의
 */


/*
 * Protocol Version
 */

enum
{
    CMP_VER_RP_NONE = 0,
    CMP_VER_RP_V1,
    CMP_VER_RP_MAX
};


/*
 * Operation ID
 */

enum
{
    CMP_OP_RP_Version = 0,
    CMP_OP_RP_MetaRepl,
    CMP_OP_RP_MetaReplTbl,
    CMP_OP_RP_MetaReplCol,
    CMP_OP_RP_MetaReplIdx,
    CMP_OP_RP_MetaReplIdxCol,
    CMP_OP_RP_HandshakeAck,
    CMP_OP_RP_TrBegin,
    CMP_OP_RP_TrCommit,
    CMP_OP_RP_TrAbort,
    CMP_OP_RP_SPSet,
    CMP_OP_RP_SPAbort,
    CMP_OP_RP_StmtBegin,
    CMP_OP_RP_StmtEnd,
    CMP_OP_RP_CursorOpen,
    CMP_OP_RP_CursorClose,
    CMP_OP_RP_Insert,
    CMP_OP_RP_Update,
    CMP_OP_RP_Delete,
    CMP_OP_RP_UIntID,
    CMP_OP_RP_Value,
    CMP_OP_RP_Stop,
    CMP_OP_RP_KeepAlive,
    CMP_OP_RP_Flush,
    CMP_OP_RP_FlushAck,
    CMP_OP_RP_Ack,
    CMP_OP_RP_LobCursorOpen,
    CMP_OP_RP_LobCursorClose,
    CMP_OP_RP_LobPrepare4Write,
    CMP_OP_RP_LobPartialWrite,
    CMP_OP_RP_LobFinish2Write,
    CMP_OP_RP_TxAck,
    CMP_OP_RP_RequestAck,
    CMP_OP_RP_Handshake,
    CMP_OP_RP_SyncPKBegin,
    CMP_OP_RP_SyncPK,
    CMP_OP_RP_SyncPKEnd,
    CMP_OP_RP_FailbackEnd,
    CMP_OP_RP_SyncTableNumber,
    CMP_OP_RP_SyncStart,
    CMP_OP_RP_SyncRebuildIndex,
    CMP_OP_RP_LobTrim,
    CMP_OP_RP_MetaReplCheck, /* BUG-34360 */
    CMP_OP_RP_MetaDictTableCount, /* BUG-38759 */
    CMP_OP_RP_AckOnDML,
    CMP_OP_RP_AckEager,
    CMP_OP_RP_MAX_VER1
};

#define CMP_OP_RP_MAX CMP_OP_RP_MAX_VER1

#endif
