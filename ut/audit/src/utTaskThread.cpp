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
 
/*******************************************************************************
 * $Id: utTaskThread.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/
#include <uto.h>
#include <utAtb.h>

IDE_RC utTaskThread::initialize(utProperties * aProp)
{

    mScanner = NULL;
    mConnA   = NULL;
    mConnB   = NULL;
    *_error  = '\0';

    IDE_TEST( aProp == NULL );
    mProp = aProp;

    mScanner = new utScanner();
    IDE_TEST(  mScanner == NULL );

    if( aProp->mTimeInterval > 0)
    {
        mCheckTime.set( aProp->mTimeInterval / 1000,
                       (aProp->mTimeInterval % 1000) * 1000);
    }
    else
    {
        mCheckTime.set(0,0);
    }

    /* Master Init connection */
    mConnA = mProp->newMaConn();

    IDE_TEST_RAISE( mConnA == NULL, err_con_a);
    IDE_TEST_RAISE( mConnA->initialize(_error, sizeof(_error)
                   ) != IDE_SUCCESS, err_con_a);
    IDE_TEST_RAISE( mConnA->connect() != IDE_SUCCESS, err_con_a );

    /* slave init connection */
    mConnB = mProp->newSlConn();

    IDE_TEST_RAISE( mConnB == NULL , err_con_b);
    IDE_TEST_RAISE( mConnB->initialize(_error, sizeof(_error)
                    ) != IDE_SUCCESS, err_con_b);
    IDE_TEST_RAISE( mConnB->connect() != IDE_SUCCESS, err_con_b );

    IDE_TEST( mScanner->initialize(
                mConnA,
                mConnB,
                aProp->mCountToCommit,
                _error) != IDE_SUCCESS);

    mScanner->setLogDir(aProp->getLogDir());
    mScanner->setReport(mProp->mVerbose   );

    return IDE_SUCCESS;
    IDE_EXCEPTION( err_con_a );
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Connect_TO_Master_Error,
                        "Wrong master URL" );
        uteSprintfErrorCode(_error, ERROR_BUFSIZE, &gErrorMgr);
    }
    IDE_EXCEPTION( err_con_b );
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Connect_TO_Slaver_Error,
                        "Wrong slave URL");
        uteSprintfErrorCode(_error, ERROR_BUFSIZE, &gErrorMgr);
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC utTaskThread::finalize()
{
    if(mScanner)
    {
        // BUG-40205 insure++ warning
        mScanner->finalize();
        delete mScanner;
        mScanner = NULL;
    }
    if(mConnA)
    {
        // BUG-40205 insure++ warning
        mConnA->finalize();
        delete mConnA;
        mConnA = NULL;
    }
    if(mConnB)
    {
        // BUG-40205 insure++ warning
        mConnB->finalize();
        delete mConnB;
        mConnB = NULL;
    }
    return IDE_SUCCESS;
}

void utTaskThread::run()
{
    utTableProp *sTab = NULL;

    IDE_TEST( mProp->getTabProp( &sTab) != IDE_SUCCESS);

    while(sTab)
    {
        /* set meta for operation */
        IDE_TEST(mScanner->setTable(sTab)!= IDE_SUCCESS);

        /* TASK-4212: audit툴의 대용량 처리시 개선 */
        IDE_TEST(mScanner->prepare(mProp) != IDE_SUCCESS );

        /* Execution cycle ( read from CSV file, then compare rows each other )  */
        IDE_TEST(mScanner->execute()     != IDE_SUCCESS);

        /* Print result of job */
        IDE_TEST(mScanner->printResult(mProp->getFLog()) != IDE_SUCCESS);

        mConnA->commit();
        mConnB->commit();

        /* get next table */
        IDE_TEST( mProp->getTabProp( &sTab) != IDE_SUCCESS);

        if(mProp->mTimeInterval > 0 )
        {
            idlOS::sleep(mCheckTime);
        }
    }

    return;

    IDE_EXCEPTION_END;

    mProp->log("FATAL[ TASK ] Process failure! [SCANER]: %s\n",mScanner->error());

    return;
}

SChar*  utTaskThread::error()
{
    if( mConnA ) mConnA->error();
    if( mConnB ) mConnB->error();
    return _error;
}
