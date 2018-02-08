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
 

/***********************************************************************/
/* $Id: qdq.cpp 82075 2018-01-17 06:39:52Z jina.kim $ */
/**********************************************************************/

#include <idl.h>
#include <ide.h>
#include <qcg.h>
#include <qdq.h>
#include <qmv.h>
#include <qmx.h>
#include <qdbCreate.h>
#include <qdbAlter.h>
#include <qdd.h>
#include <qcuProperty.h>
#include <qcuSqlSourceInfo.h>
#include <qcg.h>
#include <qdpDrop.h>
#include <qcmUser.h>
#include <qcmProc.h>
#include <qdpPrivilege.h>
#include <qcmAudit.h>
#include <qcmPkg.h>
#include <qcmTableSpace.h>
#include <qdpRole.h>

IDE_RC qdq::executeCreateQueue(qcStatement *aStatement)
{
#define IDE_FN "qdq::executeCreateQueue"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY("qdq::executeCreateQueue"));
    const void              * sSequenceHandle;
    SChar                     sSqlStr[QCM_MAX_SQL_STR_LEN];
    SChar                     sQueueName [ QC_MAX_OBJECT_NAME_LEN + 1 ];
    qdTableParseTree        * sParseTree;
    UInt                      sSeqTblID;
    smOID                     sSequenceOID;
    vSLong                    sRowCnt;
    
    SChar                     sTBSName[ SMI_MAX_TABLESPACE_NAME_LEN + 1 ];
    smiTableSpaceAttr         sTBSAttr;

    /* ======================== *
     * [1] Create Queue Table   *
     * ======================== */
    IDE_TEST( qdbCreate::executeCreateTable(aStatement) != IDE_SUCCESS );

    sParseTree = (qdTableParseTree *) aStatement->myPlan->parseTree;

    QC_STR_COPY( sQueueName, sParseTree->tableName );

    IDE_TEST(qcm::getNextTableID(aStatement, &sSeqTblID) != IDE_SUCCESS);

    IDE_TEST(
        smiTable::createSequence(
            QC_SMI_STMT( aStatement ),
            1,
            1,
            20,
            ((SLong)(ID_ULONG_MAX/2 -1)),
            1,
            SMI_SEQUENCE_CIRCULAR_ENABLE,
            &sSequenceHandle)
        != IDE_SUCCESS);

    sSequenceOID = smiGetTableId(sSequenceHandle);

    // PROJ-1502 PARTITIONED DISK TABLE
    // queue는 partition될 수 없음.
    // PROJ-1407 Temporary Table
    // queue는 temporary로 생성할 수 없음
    IDE_TEST( qcmTablespace::getTBSAttrByID( SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC,
                                             & sTBSAttr )
              != IDE_SUCCESS );
    idlOS::memcpy( sTBSName, sTBSAttr.mName, sTBSAttr.mNameLength );
    sTBSName[sTBSAttr.mNameLength] = '\0';
    
    idlOS::snprintf(sSqlStr, ID_SIZEOF(sSqlStr),
                    "INSERT INTO SYS_TABLES_ VALUES( "
                    "INTEGER'%"ID_INT32_FMT"', "
                    "INTEGER'%"ID_INT32_FMT"', "
                    "BIGINT'%"ID_INT64_FMT"', "
                    "INTEGER'0', "
                    "VARCHAR'%s_NEXT_MSG_ID', "
                    "'W', " // W means QUEUE_SEQUENCE
                    "INTEGER'0', "
                    "INTEGER'0', "
                    "BIGINT'0', "
                    "INTEGER'%"ID_INT32_FMT"', "
                    "VARCHAR'%s', "                  // 11 TBS_NAME
                    "INTEGER'%"ID_INT32_FMT"', "
                    "INTEGER'%"ID_INT32_FMT"', "
                    "INTEGER'%"ID_INT32_FMT"', "
                    "INTEGER'%"ID_INT32_FMT"', "
                    "BIGINT'%"ID_INT64_FMT"', "
                    "BIGINT'%"ID_INT64_FMT"', "
                    "BIGINT'%"ID_INT64_FMT"', "
                    "BIGINT'%"ID_INT64_FMT"', "
                    "CHAR'F',"
                    "CHAR'N',"
                    "CHAR'N',"
                    "CHAR'W',"
                    "INTEGER'1'," // Parallel degree
                    "SYSDATE, SYSDATE )",
                    sParseTree->userID,
                    sSeqTblID,
                    (ULong)sSequenceOID,
                    sQueueName,
                    SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC,
                    sTBSName,
                    QD_MEMORY_TABLE_DEFAULT_PCTFREE,
                    QD_MEMORY_TABLE_DEFAULT_PCTUSED,
                    QD_MEMORY_TABLE_DEFAULT_INITRANS,
                    QD_MEMORY_TABLE_DEFAULT_MAXTRANS,
                    (ULong)QD_MEMORY_SEGMENT_DEFAULT_STORAGE_INITEXTENTS,
                    (ULong)QD_MEMORY_SEGMENT_DEFAULT_STORAGE_NEXTEXTENTS,
                    (ULong)QD_MEMORY_SEGMENT_DEFAULT_STORAGE_MINEXTENTS,
                    (ULong)QD_MEMORY_SEGMENT_DEFAULT_STORAGE_MAXEXTENTS );

    IDE_TEST(qcg::runDMLforDDL( QC_SMI_STMT( aStatement ),
                                sSqlStr,
                                & sRowCnt ) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

#undef IDE_FN
}

IDE_RC qdq::executeDropQueue(qcStatement *aStatement)
{
#define IDE_FN "qdq::executeDropQueue"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY("qdq::executeDropQueue"));

    qdDropParseTree    * sParseTree;
    UInt                 sTableID;
    UChar                sQueueName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar                sSequenceName[QC_MAX_SEQ_NAME_LEN + 1];
    void               * sSequenceHandle;
    qcmSequenceInfo      sSequenceInfo;

    qciArgDropQueue      sArgDropQueue;

    sParseTree = (qdDropParseTree *)aStatement->myPlan->parseTree;

    //---------------------------------------
    // TASK-2176 Table에 대한 Lock을 획득한다.
    //---------------------------------------

    IDE_TEST( qcm::validateAndLockTable(aStatement,
                                        sParseTree->tableHandle,
                                        sParseTree->tableSCN,
                                        SMI_TABLE_LOCK_X)
              != IDE_SUCCESS );

    sTableID = sParseTree->tableInfo->tableID;

    //---------------------------------------
    // DROP QUEUE TABLE
    //---------------------------------------

    IDE_TEST( qdd::executeDropTable(aStatement) != IDE_SUCCESS );

    //---------------------------------------
    // DROP QUEUE SEQUENCE
    //---------------------------------------

    QC_STR_COPY( sQueueName, sParseTree->objectName );

    idlOS::snprintf(sSequenceName,
                    ID_SIZEOF(sSequenceName),
                    "%s_NEXT_MSG_ID",
                    sQueueName);

    IDE_TEST( qcm::getSequenceInfoByName( QC_SMI_STMT( aStatement ),
                                          sParseTree->userID,
                                          (UChar*)sSequenceName,
                                          idlOS::strlen(sSequenceName),
                                          &sSequenceInfo,
                                          &sSequenceHandle )
              != IDE_SUCCESS);

    IDE_TEST_RAISE(sSequenceInfo.sequenceType != QCM_QUEUE_SEQUENCE,
                   ERR_META_CRASH);

    IDE_TEST(qdd::deleteTableFromMeta(aStatement, sSequenceInfo.sequenceID)
             != IDE_SUCCESS);

    IDE_TEST(qdpDrop::removePriv4DropTable(aStatement, sSequenceInfo.sequenceID)
             != IDE_SUCCESS);

    // PROJ-2223 audit
    IDE_TEST( qcmAudit::deleteObject( aStatement,
                                      sSequenceInfo.sequenceID,
                                      sSequenceInfo.name )
              != IDE_SUCCESS );
    
    // delete from sm
    IDE_TEST( smiTable::dropSequence( QC_SMI_STMT( aStatement ),
                                      sSequenceHandle )
              != IDE_SUCCESS );

    /* PROJ-2197 PSM Renewal */
    // related PSM
    IDE_TEST(qcmProc::relSetInvalidProcOfRelated(
        aStatement,
        sParseTree->userID,
        sSequenceInfo.name,
        idlOS::strlen((SChar*)sSequenceInfo.name),
        QS_TABLE) != IDE_SUCCESS);

    // PROJ-1073 Package
    IDE_TEST(qcmPkg::relSetInvalidPkgOfRelated(
        aStatement,
        sParseTree->userID,
        sSequenceInfo.name,
        idlOS::strlen((SChar*)sSequenceInfo.name),
        QS_TABLE) != IDE_SUCCESS);


    sArgDropQueue.mTableID   = sTableID;
    sArgDropQueue.mMmSession = aStatement->session->mMmSession;
    
    // MM의 QUEUE관련 데이터 삭제요청 (commit시 삭제됨)
    IDE_TEST( qcg::mDropQueueFuncPtr( (void *)&sArgDropQueue )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_META_CRASH);
    {
        IDE_SET(ideSetErrorCode(qpERR_ABORT_QCM_META_CRASH));
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

#undef IDE_FN
}

IDE_RC qdq::executeEnqueue(qcStatement *aStatement)
{
#define IDE_FN "qdq::executeEnqueue"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY("qdq::executeStart"));

    IDE_TEST( qmx::executeInsertValues(aStatement) != IDE_SUCCESS );

    IDE_TEST ( wakeupDequeue((qcStatement *)aStatement) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

#undef IDE_FN
}

IDE_RC qdq::wakeupDequeue(qcStatement * /*aStatement*/ )
{
    return IDE_SUCCESS;
}

IDE_RC qdq::executeDequeue(qcStatement * /*aStatement*/,
                           idBool      * /*aIsTimeOut*/ )
{
#define IDE_FN "qdq::executeDequeue"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY("qdq::executeStart"));


    return IDE_SUCCESS;
#undef IDE_FN
}

IDE_RC qdq::waitForEnqueue(qcStatement * /*aStatement */,
                           idBool      * /*aIsTimeOut */ )
{
#define IDE_FN "qdq::waitForEnqueue"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY("qdq::waitForEnqueue"));

    return IDE_SUCCESS;

#undef IDE_FN
}

IDE_RC qdq::validateAlterCompactQueue(qcStatement * aStatement)
{
/***********************************************************************
 *
 * Description : ALTER QUEUE queue_name COMPACT 구문의 validation
 *
 * Implementation :
 *    (1) Queue가 존재하는지 검사
 *    (2) 권한 검사
 *
 *    replication 안됨
 * 
 *    ALTER TABLE권한은 막혀있음! 오직 COMPACT만 됨
 *    따라서 'qdbAlter::checkOperatable()' 함수 호출할 필요없음
 *    만약 ALTER TABLE 구문으로 Queue Table을 변경하고자 한다면
 *    이 부분을 열도록 해야함
 * 
 ***********************************************************************/    
#define IDE_FN "qdq::validateAlterQueueCompact"

    qdTableParseTree    * sParseTree;
    UInt                  sTableType;
    qcuSqlSourceInfo      sqlInfo;

    sParseTree = (qdTableParseTree *)aStatement->myPlan->parseTree;

    /* BUG-30059 */
    if ( qdbCommon::containDollarInName( &(sParseTree->tableName) ) == ID_TRUE )
    {
        sqlInfo.setSourceInfo(
            aStatement,
            &(sParseTree->tableName) );

        IDE_RAISE( CANT_USE_RESERVED_WORD );
    }

    //----------------------------
    // Queue 가 존재하는지 검사
    //----------------------------
    if ( qdbCommon::checkTableInfo( aStatement,
                                    sParseTree->userName,
                                    sParseTree->tableName,
                                    &(sParseTree->userID),
                                    &(sParseTree->tableInfo),
                                    &(sParseTree->tableHandle),
                                    &(sParseTree->tableSCN))
         == IDE_SUCCESS )
    {
        // tableInfo가 있는 경우
        if( sParseTree->tableInfo->tableType == QCM_QUEUE_TABLE )
        {
            // Queue Table인지 검사
        }
        else
        {
            IDE_RAISE(ERR_NOT_FOUND_QUEUE);
        }
    }
    else
    {
        IDE_RAISE(ERR_NOT_FOUND_QUEUE);
    }

    /* BUG-
       ALTER TABLE 권한은 막혀있음
    IDE_TEST( qdbAlter::checkOperatable( aStatement,
                                         sParseTree->tableInfo )
              != IDE_SUCCESS );
    */
    
    //----------------------------
    // 테이블에 LOCK(IS)
    //----------------------------
    IDE_TEST( qcm::lockTableForDDLValidation(aStatement,
                                             sParseTree->tableHandle,
                                             sParseTree->tableSCN)
              != IDE_SUCCESS );

    if ( QCG_GET_SESSION_USER_ID(aStatement) != QC_SYSTEM_USER_ID )
    {
        // sParseTree->userID is a owner of table
        IDE_TEST_RAISE(sParseTree->userID == QC_SYSTEM_USER_ID,
                       ERR_NOT_ALTER_META);
    }
    else
    {
        if ( ( aStatement->session->mQPSpecific.mFlag
               & QC_SESSION_ALTER_META_MASK )
             == QC_SESSION_ALTER_META_DISABLE )
        {
            IDE_RAISE(ERR_NOT_ALTER_META);
        }
    }

    //----------------------------
    // 권한 검사
    //----------------------------

    IDE_TEST( qdpRole::checkDDLAlterTablePriv( aStatement,
                                               sParseTree->tableInfo )
              != IDE_SUCCESS );

    //----------------------------
    // PROJ-1723 [MDW/INTEGRATOR] Altibase Plugin 개발
    //   DDL Statement Text의 로깅
    //----------------------------
    
    if (QCU_DDL_SUPPLEMENTAL_LOG == 1)
    {
        IDE_TEST( qciMisc::writeDDLStmtTextLog( aStatement,
                                                sParseTree->tableInfo->tableOwnerID,
                                                sParseTree->tableInfo->name )
                  != IDE_SUCCESS );
    }

    //----------------------------
    // Queue Table은 MEMORY 기반 테이블 이어야만 함
    //----------------------------
    sTableType = sParseTree->tableInfo->tableFlag & SMI_TABLE_TYPE_MASK;
    IDE_TEST_RAISE( (sTableType != SMI_TABLE_MEMORY), 
                    ERR_NO_MEMORY_OR_VOLATILE_TABLE );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_FOUND_QUEUE)
    {
        IDE_SET(ideSetErrorCode(qpERR_ABORT_QDQ_NOT_FOUND_QUEUE));
    }
    IDE_EXCEPTION(ERR_NOT_ALTER_META);
    {
        IDE_SET(ideSetErrorCode(qpERR_ABORT_QDD_NO_DROP_META_TABLE));
    }
    IDE_EXCEPTION(ERR_NO_MEMORY_OR_VOLATILE_TABLE)
    {
        IDE_SET(ideSetErrorCode(qpERR_ABORT_QDB_NO_MEMORY_OR_VOLATILE_TABLE));
    }
    IDE_EXCEPTION( CANT_USE_RESERVED_WORD );
    {
        (void)sqlInfo.init(aStatement->qmeMem);
        IDE_SET(
            ideSetErrorCode(qpERR_ABORT_QDB_RESERVED_WORD_IN_OBJECT_NAME,
                            sqlInfo.getErrMessage() ));
        (void)sqlInfo.fini();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;

#undef IDE_FN
}

IDE_RC qdq::executeCompactQueue(qcStatement * aStatement)
{
/***********************************************************************
 *
 * Description : ALTER QUEUE queue_name COMPACT 구문의 execution
 *
 * Implementation :
 *
 ***********************************************************************/       
#define IDE_FN "qdq::executeCompactQueue"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY("qdq::executeCompactQueue"));

    IDE_TEST( qdbAlter::executeCompactTable(aStatement) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;


#undef IDE_FN
}

IDE_RC qdq::executeStart(qcStatement * /*aStatement*/)
{
#define IDE_FN "qdq::executeStart"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY("qdq::executeStart"));

    return IDE_SUCCESS;

#undef IDE_FN
}

IDE_RC qdq::executeStop(qcStatement * /*aStatement*/)
{
#define IDE_FN "qdq::executeStart"

    return IDE_SUCCESS;

#undef IDE_FN
}
