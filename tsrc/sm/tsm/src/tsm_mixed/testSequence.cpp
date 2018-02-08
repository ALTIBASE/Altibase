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
 * $Id: testSequence.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>

#define TSQ_USER_ID    (1)
#define TSQ_TABLE_NAME ((SChar*)"TSQ_S1")

#define THREADS1 0
#define THREADS2 0

static PDL_sema_t gThreads1;
static PDL_sema_t gThreads2;

static UInt gThreads1Data;
static UInt gThreads2Data;

IDE_RC testSequenceMulti();
void * sequence_A(void *data);
void * sequence_B(void *data);

IDE_RC testSequence1()
{
    smiTrans      sTrans;
    smiStatement *sRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    SLong         sValue = 0;
    UInt          i = 0;

    gVerbose = ID_TRUE;
    
    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS);

    IDE_TEST(qcxCreateSequence(sRootStmt,
                               TSQ_USER_ID,
                               TSQ_TABLE_NAME,
                               -1000,
                               10,
                               10,
                               1000,
                               -1000)
             != IDE_SUCCESS);
    
    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    IDE_TEST(sTrans.initialize()!= IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL)!= IDE_SUCCESS);

    IDE_TEST(qcxReadSequence(sRootStmt,
                             TSQ_USER_ID,
                             TSQ_TABLE_NAME,
                             SMI_SEQUENCE_CURR,
                             &sValue)
             == IDE_SUCCESS);


    tsmLog("#### Begin Read Sequence Value\n");
    for(i = 0; i < 50; i++)
    {
        if(qcxReadSequence(sRootStmt,
                           TSQ_USER_ID,
                           TSQ_TABLE_NAME,
                           SMI_SEQUENCE_NEXT,
                           &sValue)
           != IDE_SUCCESS)
        {
            ideDump();
            break;
        }
        
        
        tsmLog("Sequence Value:%"ID_INT64_FMT"\n", sValue);
    }
    tsmLog("#### End Read Sequence Value\n");
    
    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    gVerbose = ID_FALSE;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testSequence2()
{
    smiTrans      sTrans;
    smiStatement *sRootStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    SLong         sValue = 0;
    UInt          i = 0;

    IDE_TEST(sTrans.initialize()!= IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL)!= IDE_SUCCESS);

    tsmLog("#### Begin Read Sequence Value\n");
    for(i = 0; i < 10000; i++)
    {
        if(qcxReadSequence(sRootStmt,
                           TSQ_USER_ID,
                           TSQ_TABLE_NAME,
                           SMI_SEQUENCE_NEXT,
                           &sValue)
           != IDE_SUCCESS)
        {
            ideDump();
            break;
        }
        
        
        tsmLog("Sequence Value:%"ID_INT64_FMT"\n", sValue);
    }
    tsmLog("#### End Read Sequence Value\n");
    
    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    IDE_TEST(sTrans.initialize()!= IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL)!= IDE_SUCCESS);

    IDE_TEST(qcxAlterSequence(sRootStmt,
                              TSQ_USER_ID,
                              TSQ_TABLE_NAME,
                              100,
                              10000,
                              10000,
                              -1000)
             != IDE_SUCCESS);

    
    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    IDE_TEST(sTrans.initialize()!= IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL)!= IDE_SUCCESS);

    tsmLog("#### Begin Read Sequence Value\n");
    for(i = 0; i < 10000; i++)
    {
        if(qcxReadSequence(sRootStmt,
                           TSQ_USER_ID,
                           TSQ_TABLE_NAME,
                           SMI_SEQUENCE_NEXT,
                           &sValue)
           != IDE_SUCCESS)
        {
            ideDump();
            break;
        }
        
        
        tsmLog("Sequence Value:%"ID_INT64_FMT"\n", sValue);
    }
    tsmLog("#### End Read Sequence Value\n");
    
    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);
    
    IDE_TEST(sTrans.initialize()!= IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL)!= IDE_SUCCESS);

    IDE_TEST(qcxDropSequence(sRootStmt,
                             TSQ_USER_ID,
                             TSQ_TABLE_NAME)
             != IDE_SUCCESS);

    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    gVerbose = ID_FALSE;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testSequenceMulti()
{
    SLong           sFlags    = THR_JOINABLE;
    SLong           sPriority = PDL_DEFAULT_THREAD_PRIORITY;
    size_t          sStackSize = 1024*1024;
    PDL_hthread_t   sArrThreadHandle[2];
    PDL_thread_t    sArrTID[2];
    UInt            i;
    
    IDE_TEST(PDL_OS::sema_init(&gThreads1, THREADS1)
             != 0 );
    IDE_TEST(PDL_OS::sema_init(&gThreads2, THREADS2)
             != 0 );

    IDE_TEST(idlOS::thr_create(sequence_A,
                               &gThreads1Data,
                               sFlags,
                               &sArrTID[0],
                               &sArrThreadHandle[0], 
                               sPriority,
                               NULL,
                               sStackSize) != IDE_SUCCESS);
    
    IDE_TEST(idlOS::thr_create(sequence_B,
                               &gThreads2Data,
                               sFlags,
                               &sArrTID[1],
                               &sArrThreadHandle[1], 
                               sPriority,
                               NULL,
                               sStackSize) != IDE_SUCCESS );
    for( i = 0; i < 2; i++ )
    {
        IDE_TEST(idlOS::thr_join( sArrThreadHandle[i], NULL ) != 0);
    } 

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void * sequence_A(void * /*data*/)
{
    smiTrans      sTrans;
    smiStatement *sRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    SLong         sValue = 0;
    UInt          i = 0;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS);

    IDE_TEST(qcxCreateSequence(sRootStmt,
                               TSQ_USER_ID,
                               TSQ_TABLE_NAME,
                               -1000,
                               10,
                               10,
                               1000,
                               -1000)
             != IDE_SUCCESS);

    IDE_TEST(PDL_OS::sema_post(&gThreads1) != 0);
    IDE_TEST(PDL_OS::sema_wait(&gThreads2) != 0);
    
    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);


    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS);

    for(i = 0; i < 10; i++)
    {
        IDE_TEST(qcxReadSequence(sRootStmt,
                                 TSQ_USER_ID,
                                 TSQ_TABLE_NAME,
                                 SMI_SEQUENCE_NEXT,
                                 &sValue)
                 != IDE_SUCCESS);

        tsmLog("Sequence Value:%"ID_INT64_FMT"\n", sValue);
        
        IDE_TEST(PDL_OS::sema_post(&gThreads1) != 0);
        IDE_TEST(PDL_OS::sema_wait(&gThreads2) != 0);
    }

    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS);

    IDE_TEST(qcxDropSequence(sRootStmt,
                             TSQ_USER_ID,
                             TSQ_TABLE_NAME)
             != IDE_SUCCESS);

    IDE_TEST(PDL_OS::sema_post(&gThreads1) != 0);

    idlOS::sleep(5);

    IDE_TEST(sTrans.rollback() != IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    IDE_TEST(PDL_OS::sema_wait(&gThreads2) != 0);
    
    IDE_TEST(sTrans.initialize()!= IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL)!= IDE_SUCCESS);

    IDE_TEST(qcxDropSequence(sRootStmt,
                             TSQ_USER_ID,
                             TSQ_TABLE_NAME)
             != IDE_SUCCESS);

    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    IDE_TEST(PDL_OS::sema_post(&gThreads1) != 0);
    
    return NULL;

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    ideDump();

    return NULL;
}

void * sequence_B(void * /*data*/)
{
    smiTrans      sTrans;
    smiStatement *sRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    SLong         sValue = 0;
    UInt          i = 0;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    IDE_TEST(sTrans.initialize()!= IDE_SUCCESS);
    IDE_TEST(sTrans.begin(&sRootStmt, NULL)!= IDE_SUCCESS);

    IDE_TEST(PDL_OS::sema_wait(&gThreads1) != 0);
    
    IDE_TEST(qcxAlterSequence(sRootStmt,
                              TSQ_USER_ID,
                              TSQ_TABLE_NAME,
                              100,
                              10000,
                              10000,
                              -1000)
             == IDE_SUCCESS);

    IDE_TEST(PDL_OS::sema_post(&gThreads2) != 0);

    for(i = 0; i < 11; i++)
    {
        IDE_TEST(PDL_OS::sema_wait(&gThreads1) != 0);

        if ( i != 10 )
        {
            IDE_TEST(qcxReadSequence(sRootStmt,
                                     TSQ_USER_ID,
                                     TSQ_TABLE_NAME,
                                     SMI_SEQUENCE_NEXT,
                                     &sValue)
                     != IDE_SUCCESS);

            tsmLog("Sequence Value:%"ID_INT64_FMT"\n", sValue);
        }
        else
        {
            IDE_TEST(qcxReadSequence(sRootStmt,
                                     TSQ_USER_ID,
                                     TSQ_TABLE_NAME,
                                     SMI_SEQUENCE_NEXT,
                                     &sValue)
                     == IDE_SUCCESS);
            ideDump();
            tsmLog("Sequence Value:[already dropped Sequence]\n");
        }
        
        IDE_TEST(PDL_OS::sema_post(&gThreads2) != 0);
    }
    
    IDE_TEST(sTrans.commit(&sDummySCN)!= IDE_SUCCESS);
    IDE_TEST(sTrans.destroy()!= IDE_SUCCESS);

    IDE_TEST(PDL_OS::sema_wait(&gThreads1) != 0);
    
    return NULL;

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    ideDump();

    return NULL;
    
}

