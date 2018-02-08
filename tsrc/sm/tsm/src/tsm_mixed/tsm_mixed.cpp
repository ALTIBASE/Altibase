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
 * $Id: tsm_mixed.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <testRecovery.h>
#include <testCursor.h>
#include <testAger.h>
#include <testRebuildIndex.h>
#include <testRefineDB.h>
#include <tsmLobAPI.h>
#include <tsm_mixed.h>

typedef IDE_RC (*tsmFuncType)(void);

typedef struct tsmFuncInfo
{
    idBool       mExecute;
    const SChar *mName;
    tsmFuncType  mFuncPtr;
} tsmFuncInfo;

IDE_RC testDummy()
{
    return IDE_SUCCESS;
}

IDE_RC abnormalExit()
{

#if defined( DEBUG_PAGE_ALLOC_FREE )
    idlOS::fprintf(stdout, "Dumping all FPLs before abnormal Exit\n");
    smmFPLManager::dumpAllFPLs();
#endif
    
    idlOS::exit(-1);
    return IDE_FAILURE;
}

/* ------------------------------------------------
 *  [] 테스팅 모듈 어레이 리스트
 *     추가영역 : 1. 함수 프로토타입
 *                2. 함수 포인터 배열
 * ----------------------------------------------*/

IDE_RC testInit();
IDE_RC testSelectiveLoading(); // <== here : 1
IDE_RC testCursor(); 
IDE_RC testDDL();
IDE_RC testDML();
IDE_RC testIsolation();
IDE_RC testAPI(); 
IDE_RC testGamestar(); 
IDE_RC testAttach1(); 
IDE_RC testAttach2();
IDE_RC testSequence1();
IDE_RC testSequence2();
IDE_RC testSequenceMulti();
IDE_RC testXA();
IDE_RC testXA2();
IDE_RC testLOBInterface();
IDE_RC testLOBByTableCursor1();
IDE_RC testLOBByTableCursor2();
IDE_RC testLOBInterface1();
IDE_RC testLOBInterface2();
IDE_RC testLOBInterface3();
IDE_RC testLOBInterface4();
IDE_RC testLOBFunction();
IDE_RC testLOBConcurrency();
IDE_RC testLOBRecovery1();
IDE_RC testLOBRecovery2();
IDE_RC testLOBStress1();
IDE_RC testLOBStress2();
IDE_RC testLOBException1();
IDE_RC testLOBException2();
IDE_RC testLOBException3();
IDE_RC testLOBException4();
IDE_RC testLOBException5();

struct tsmFuncInfo gTestFuncArray[] = 
{
#ifdef NOTDEF    
    { 
        ID_TRUE, "attach1", testAttach1 
    },
    {
        ID_TRUE, "normal_shutdown", NULL  // 정상종료 
    },

    { 
        ID_TRUE, "attach2", testAttach2
    },
    {
        ID_TRUE, "normal_shutdown", NULL  // 정상종료 
    },

#endif    
    /* ------------- shutdown 되지 않는 테스트 시작 ------------
     *  - 주의 : 이 이후에 추가되는 테스트는 절대로 exit 할 수 없음!!
     * ---------------------------------------------------------*/
    
    /* --------
     * [1] ddl & dml & etc Test : 성준화
     * -------*/
    { 
        ID_TRUE, "tsm_cursor", testCursor
    },

   
    {
        ID_TRUE, "tsm_ddl", testDDL
    },
    
    {
        ID_TRUE, "tsm_dml", testDML
    },
    {
        ID_TRUE, "tsm_Isolation", testIsolation
    },
    {
        ID_TRUE, "tsm_insert_commit1", testInsertCommitRefineDB1
    },

    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_insert_commit2", testRefineDB2
    },
    {
        ID_TRUE, "tsm_insert_rollback1", testInsertRollbackRefineDB1
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_insert_rollback2", testRefineDB2
    },
    {
        ID_TRUE, "tsm_update_commit1", testUpdateCommitRefineDB1
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_update_commit2", testRefineDB2
    },
    {
        ID_TRUE, "tsm_update_rollback1", testUpdateRollbackRefineDB1
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_update_rollback2", testRefineDB2
    },
    {
        ID_TRUE, "tsm_delete_commit_1", testDeleteCommitRefineDB1
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_delete_commit_2", testRefineDB2
    },
    {
        ID_TRUE, "tsm_delete_rollback_1", testDeleteRollbackRefineDB1
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_delete_rollback_2", testRefineDB2
    },
    /* --------
     * [2] Ager Test : 최현수
     * -------*/
    {
        ID_TRUE, "tsm_ager", testAger
    },
    /* --------
     * [3] Index Rebuild Test : 최현수
     * -------*/
    {
        ID_TRUE, "tsm_rebuild_index_stage1", testRebuildIndexStage1
    },
    {
        ID_TRUE, "normal_shutdown", NULL  // 정상종료 
    },
    {
        ID_TRUE, "tsm_rebuild_index_stage2", testRebuildIndexStage2
    },
    /* --------
     * [4] Selective Loading : 김성진
     * -------*/
    { 
        ID_TRUE, "tsm_selective_loading", testSelectiveLoading 
    },
    /* --------
     * [6] sequence : 김대일
     * -------*/
    { 
        ID_TRUE, "tsm_sequence1", testSequence1 
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    { 
        ID_TRUE, "tsm_sequence2", testSequence2
    },
    { 
        ID_TRUE, "tsm_sequence_multi", testSequenceMulti
    },
    {
        ID_TRUE, "normal_shutdown", NULL  // 정상종료 
    },
    /* --------
     * [5] Global Transaction : msjung
     * -------*/
    { 
        ID_TRUE, "tsm_xa", testXA 
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    { 
        ID_TRUE, "tsm_xa2", testXA2 
    },
    {
        ID_TRUE, "normal_shutdown", abnormalExit
    },

    {
        ID_TRUE, "tsm_recovery_normal", testRecovery_normal
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_recovery_makeState", testRecovery_makeState
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_recovery_makeSure", testRecovery_makeSure
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    { 
        ID_TRUE, "tsm_LOB_tablecursor1", testLOBByTableCursor1
    },
    { 
        ID_TRUE, "tsm_LOB_tablecursor2", testLOBByTableCursor2
    },
    {
        ID_TRUE, "tsm_LOB_interface1", testLOBInterface1
    },
    {
        ID_TRUE, "tsm_LOB_interface2", testLOBInterface2
    },
    {
        ID_TRUE, "tsm_LOB_interface3", testLOBInterface3
    },
    {
        ID_TRUE, "tsm_LOB_interface4", testLOBInterface4
    },
    {
        ID_TRUE, "tsm_LOB_exception1", testLOBException1
    },
    {
        ID_TRUE, "tsm_LOB_exception2", testLOBException2
    },
    {
        ID_TRUE, "tsm_LOB_exception3", testLOBException3
    },
    {
        ID_TRUE, "tsm_LOB_exception4", testLOBException4
    },
    {
        ID_TRUE, "tsm_LOB_exception5", testLOBException5
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_LOB_function", testLOBFunction
    },
    {
        ID_TRUE, "tsm_LOB_concurrency", testLOBConcurrency
    },
    {
        ID_TRUE, "tsm_LOB_recovery1", testLOBRecovery1
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_LOB_recovery2", testLOBRecovery2
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_LOB_stress1", testLOBStress1
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },
    {
        ID_TRUE, "tsm_LOB_stress2", testLOBStress2
    },
    {
        ID_TRUE, "abnormal_shutdown", abnormalExit
    },

    /* --------
     * [7] Recovery : 김대일
     * -------*/
    // 아래 line을 삭제할 수 없음 : find 할 경우 name == NULL을 끝으로 인식!
    {
        ID_TRUE, NULL, NULL
    }
};

struct tsmFuncInfo gTestFuncArray_dura[] =
{

    /* --------
     * [1] ddl & dml & etc Test : 성준화
     * -------*/
    {
        ID_TRUE, "tsm_cursor", testCursor
    },

    {
        ID_TRUE, "tsm_ddl", testDDL
    },

    {
        ID_TRUE, "tsm_dml", testDML
    },
    {
        ID_TRUE, "tsm_Isolation", testIsolation
    },
    {
        ID_TRUE, "tsm_insert_commit1", testInsertCommitRefineDB1
    },
    {
        ID_TRUE, "tsm_insert_rollback1", testInsertRollbackRefineDB1
    },
    {
        ID_TRUE, "tsm_update_commit1", testUpdateCommitRefineDB1
    },
    {
        ID_TRUE, "tsm_update_rollback1", testUpdateRollbackRefineDB1
    },
    {
        ID_TRUE, "tsm_delete_commit_1", testDeleteCommitRefineDB1
    },
    {
        ID_TRUE, "tsm_delete_rollback_1", testDeleteRollbackRefineDB1
    },
    /* --------
     * [2] Ager Test : 최현수
     * -------*/
    {
        ID_TRUE, "tsm_ager", testAger
    },
    /* --------                                                       
     * [3] Index Rebuild Test : 최현수                                
     * ------- */
    {
        ID_TRUE, "tsm_rebuild_index_stage1", testRebuildIndexStage1
    },
    /* --------
     * [6] sequence : 김대일
     * -------*/
    {
        ID_TRUE, "tsm_sequence1", testSequence1
    },
    {
        ID_TRUE, "tsm_sequence2", testSequence2
    },
    {
        ID_TRUE, "tsm_sequence_multi", testSequenceMulti
    },
    {
        ID_TRUE, "tsm_recovery_normal", testRecovery_normal
    },
    {
        ID_TRUE, "normal_shutdown", abnormalExit
    },
    /* --------
     * [7] Recovery : 김대일
     * -------*/
    // 아래 line을 삭제할 수 없음 : find 할 경우 name == NULL을 끝으로 인식!
    {
        ID_TRUE, NULL, NULL
    }
};

static IDE_RC  doTestFuncsFromStartPoint(tsmFuncInfo aTestFuncArray[],
                                         SInt aStartIndex)
{
    tsmFuncType sFunc;
    const  SChar *sName;    
    while(1)
    {
        
        sFunc = aTestFuncArray[aStartIndex].mFuncPtr;
        sName = aTestFuncArray[aStartIndex].mName;

        if (sFunc != NULL)
        {
            if( aTestFuncArray[aStartIndex].mExecute == ID_TRUE )
            {
                idlOS::fprintf(TSM_OUTPUT, "\n\n  [Start] Testing of ** %s"
                               " ******************** \n", sName );
                idlOS::fflush(TSM_OUTPUT);
                IDE_CLEAR();
                IDE_TEST_RAISE(sFunc() != IDE_SUCCESS,
                               function_error);
                idlOS::fprintf(TSM_OUTPUT, "\n  [SUCCESS] Testing of ** %s"
                               " ******************** \n", sName );
                idlOS::fflush(TSM_OUTPUT);
            }
        }
        else
        {
            break;
        }
        aStartIndex++;
    }//while(1)
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(function_error);
    {
        idlOS::fprintf(TSM_OUTPUT, "[FAILURE] Error in Test Module [%s] \n",
                       sName);
    }
    IDE_EXCEPTION_END;
    ideDump();

    return IDE_FAILURE;
}

static SInt  findTestStartIndex(tsmFuncInfo aTestFuncArray[],
                                SChar* aStartFuncName)
{
    SInt sStartIndex;
    const  SChar *sName;
    
    for (sStartIndex = 0; ; sStartIndex++)
    {
        sName = aTestFuncArray[sStartIndex].mName;
        IDE_TEST_RAISE(sName == NULL, no_such_func_error);
        if (idlOS::strcmp(sName, aStartFuncName) == 0)
        {
            break;
        }//if
    }//for
    return sStartIndex;
    
    IDE_EXCEPTION(no_such_func_error);
    {
        idlOS::fprintf(TSM_OUTPUT, "[ERROR] No Such Test Module [%s]..\n",
                       aStartFuncName);
    }
    IDE_EXCEPTION_END;
    return(-1);    
}

/* ------------------------------------------------
 *  [] Mixed 리스트
 * ----------------------------------------------*/

int main(SInt argc, SChar **argv)
{
    SInt   sStartIndex = 0;
    SInt   sOptionArg;
    SChar  sStartFuncName[256];
    SChar  *sEnvStr;

    gVerbose = ID_TRUE;
    gIndex   = ID_TRUE;

    idlOS::memset(sStartFuncName, 0, 256);
    
    while ( (sOptionArg = idlOS::getopt(argc, argv, "d:s:")) != EOF)
    {
        switch(sOptionArg)
        {
         case 'd':
         {
             tsmFuncInfo* sTestFunc;
             
             for( sTestFunc = gTestFuncArray;
                  sTestFunc->mName != NULL;
                  sTestFunc++ )
             {
                 if( idlOS::strcmp( sTestFunc->mName, optarg ) == 0 )
                 {
                     sTestFunc->mExecute = ID_FALSE;
                 }
             }
             break;
         }
         case 's':  // 테스팅 주제 설정
             idlOS::strncpy(sStartFuncName, optarg, 255);
//               idlOS::strncpy(sStartFuncName, argv[2], 255);
            break;
        }//switch
    } //while

    IDE_TEST(tsmInit() != IDE_SUCCESS);

    IDE_TEST(smiStartup(SMI_STARTUP_PRE_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_CONTROL,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_META,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);

    IDE_TEST(qcxInit() != IDE_SUCCESS);

    sEnvStr = getenv("TSM_DURABILITY_LEVEL");  

    if ( sEnvStr == NULL || strcmp(sEnvStr, "1") != 0 )               
    { 
        if (sStartFuncName[0] != 0) // 시작될 함수가 입력됨
        {
            sStartIndex = findTestStartIndex(gTestFuncArray,
                                             sStartFuncName);                                             
            IDE_TEST_RAISE(sStartIndex== -1,no_such_func_error);
            
        }//if
        IDE_TEST_RAISE(doTestFuncsFromStartPoint(gTestFuncArray,sStartIndex)
                       != IDE_SUCCESS,function_error);
        
    }//if sEnvStr == NULL || strcmp(sEnvStr, "1") != 0
    else
    {
        if (sStartFuncName[0] != 0) // 시작될 함수가 입력됨
        {
            sStartIndex = findTestStartIndex(gTestFuncArray_dura,
                                             sStartFuncName);
            IDE_TEST_RAISE(sStartIndex== -1,no_such_func_error);

        }//if
        IDE_TEST_RAISE(doTestFuncsFromStartPoint(gTestFuncArray_dura,
                                           sStartIndex) != IDE_SUCCESS,
                       function_error);
        
    }//else   

    IDE_TEST(smiStartup(SMI_STARTUP_SHUTDOWN,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    
    IDE_TEST(tsmFinal() != IDE_SUCCESS);

    return 0;

    IDE_EXCEPTION(no_such_func_error);
    {
        idlOS::fprintf(TSM_OUTPUT, "[ERROR] No Such Test Module [%s]..\n",
                       sStartFuncName);
    }
    IDE_EXCEPTION(function_error);
    {
        idlOS::fprintf(TSM_OUTPUT, "[FAILURE] Error in Test Module [%s] \n",
                       sStartFuncName);
    }
    IDE_EXCEPTION_END;
    ideDump();
    return 1;
}



