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
 
#include <idl.h>
#include <idu.h>
#include <idp.h>

#include <idl.h>
#include <ide.h>
#include <idtBaseThread.h>
#include <iduMutex.h>
#include <iduProperty.h>

#define LOOP_COUNT 10000000
#define MAX_TEST_THREAD 20

iduMutex     gMutex;
UInt         gCount = 0;
UInt         gMiss = 0;

void smDriverMessage(const SChar *aFormat, ...)
{
    va_list     ap;
    
    va_start (ap, aFormat);
    idlOS::fprintf(stdout, "      "); // make space 
    (void)vfprintf (stdout, aFormat, ap);
    va_end (ap);
    idlOS::fflush(stdout);
}

//=======================================================================

typedef enum
{
    LT_READ,
    LT_WRITE_WRITE, /* double write check */
    LT_WRITE_PLUS,
    LT_WRITE_MINUS,
    LT_TRY_READ,
    LT_TRY_WRITE,
    LT_RANDOM
} threadType;

class latchThread : public idtBaseThread
{
    threadType mType;
    UInt       mCount;
    UInt       mMiss;
public:
    latchThread() {}
    ~latchThread(){};

    void   setCount(UInt aCount) { mCount = aCount; }
    UInt   getMiss()             { return mMiss; }
    IDE_RC initialize(threadType aType, UInt aCount);
    IDE_RC destroy();
    virtual void run();
    
};

IDE_RC latchThread::initialize(threadType aType, UInt aCount)
{
    setFlag(THR_BOUND | THR_JOINABLE);

    mType  = aType;

    mCount = aCount;

    mMiss  = 0;

    return IDE_SUCCESS;
}

IDE_RC latchThread::destroy()
{
    return IDE_SUCCESS;
}

void latchThread::run()
{
    UInt   i;
    idBool sFlag;

    IDE_ASSERT(ideAllocErrorSpace() == IDE_SUCCESS);
    for (i = 0; i < mCount; i++)
    {
        switch(mType)
        {
        case LT_READ:
            break;

        case LT_WRITE_PLUS:
            IDE_TEST(gMutex.lock() != IDE_SUCCESS);
            gCount++;
            IDE_TEST(gMutex.unlock() != IDE_SUCCESS);
            break;
            
        case LT_WRITE_MINUS:
            IDE_TEST(gMutex.lock() != IDE_SUCCESS);
            gCount--;
            IDE_TEST(gMutex.unlock() != IDE_SUCCESS);
            break;
            
        case LT_TRY_WRITE:
            IDE_TEST(gMutex.trylock(sFlag) != IDE_SUCCESS);

            if (sFlag == ID_FALSE)
            {
                mMiss++;
            }
            else
            {
                IDE_ASSERT(0); // can't occur
            }
            break;
        }
    }
    return;
    IDE_EXCEPTION_END;
    smDriverMessage("FATAL ERROR");
}

//=======================================================================

static IDE_RC initialize()
{
    idlOS::memset(&gMutex, 0xff, sizeof(gMutex));
    IDE_TEST(gMutex.initialize("native",
                               IDU_MUTEX_KIND_NATIVE) != IDE_SUCCESS);
    
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static IDE_RC destroy()
{
    IDE_TEST(gMutex.destroy() != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

UInt gSeed = 0;

static IDE_RC  singleTest()
{
    UInt i;

    smDriverMessage("Begin of Single Test\n");

    for (i = 0; i < 10000; i++)
    {
        IDE_TEST(gMutex.lock() != IDE_SUCCESS);
        gSeed++;
        
        IDE_TEST(gMutex.unlock() != IDE_SUCCESS);
    }
    if (gSeed == 10000)
    {
        smDriverMessage("Success of Single Test\n");
    }
    else
    {
        smDriverMessage("Failure of Single Test\n");
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

latchThread gTest[MAX_TEST_THREAD];

static idBool test()
{
    UInt i;
	PDL_Time_Value pdl_start_time, pdl_end_time, v_timeval;

	pdl_start_time = idlOS::gettimeofday();
    // 1. single test
    IDE_TEST(singleTest() != IDE_SUCCESS);
    
	pdl_end_time = idlOS::gettimeofday();
    v_timeval = pdl_end_time - pdl_start_time;
    printf("1. Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());

    // 3. two thread plus test
    smDriverMessage("Test W-W(plus/plus) test \n");
    IDE_TEST(gTest[0].initialize(LT_WRITE_PLUS, LOOP_COUNT) != IDE_SUCCESS);
    IDE_TEST(gTest[1].initialize(LT_WRITE_PLUS, LOOP_COUNT) != IDE_SUCCESS);

    IDE_TEST(gTest[0].start() != IDE_SUCCESS);
    IDE_TEST(gTest[1].start() != IDE_SUCCESS);
    IDE_TEST(gTest[0].waitToStart(0) != IDE_SUCCESS);
    IDE_TEST(gTest[1].waitToStart(0) != IDE_SUCCESS);
    
	pdl_start_time = idlOS::gettimeofday();

    IDE_ASSERT(idlOS::thr_join(gTest[0].getHandle(), NULL) == 0);
    IDE_ASSERT(idlOS::thr_join(gTest[1].getHandle(), NULL) == 0);
    
	pdl_end_time = idlOS::gettimeofday();
    v_timeval = pdl_end_time - pdl_start_time;
    printf("2. Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    
    if (gCount != LOOP_COUNT * 2)
    {
        smDriverMessage("Failure of W-W(plus/plus) test (%"ID_UINT32_FMT")\n", gCount);
    }
    else
    {
        smDriverMessage("Success\n");
    }
    
    // 4. 2 thread plus/minus test
    smDriverMessage("Test W-W(plus/minus) test \n");

    gCount = 0;

    IDE_TEST(gTest[0].initialize(LT_WRITE_PLUS,  LOOP_COUNT) != IDE_SUCCESS);
    IDE_TEST(gTest[1].initialize(LT_WRITE_MINUS, LOOP_COUNT) != IDE_SUCCESS);

    IDE_TEST(gTest[0].start() != IDE_SUCCESS);
    IDE_TEST(gTest[0].waitToStart(0) != IDE_SUCCESS);
    IDE_TEST(gTest[1].start() != IDE_SUCCESS);
    IDE_TEST(gTest[1].waitToStart(0) != IDE_SUCCESS);

	pdl_start_time = idlOS::gettimeofday();
    
    IDE_ASSERT(idlOS::thr_join(gTest[0].getHandle(), NULL) == 0);
    IDE_ASSERT(idlOS::thr_join(gTest[1].getHandle(), NULL) == 0);
    
	pdl_end_time = idlOS::gettimeofday();
    v_timeval = pdl_end_time - pdl_start_time;
    printf("3. Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    
    if (gCount != 0)
    {
        smDriverMessage("Failure of W-W(plus/minus) test (%"ID_UINT32_FMT")\n", gCount);
    }
    else
    {
        smDriverMessage("Success\n");
    }
    
    // 6. n thread try write 
    smDriverMessage("Test tryWrite test \n");

    gMiss = 0;

    IDE_TEST(gMutex.lock() != IDE_SUCCESS);

	pdl_start_time = idlOS::gettimeofday();
    for (i = 0; i < MAX_TEST_THREAD; i++)
    {
        IDE_TEST(gTest[i].initialize(LT_TRY_WRITE,  LOOP_COUNT) != IDE_SUCCESS);
        IDE_TEST(gTest[i].start() != IDE_SUCCESS);
        IDE_TEST(gTest[i].waitToStart(0) != IDE_SUCCESS);
    }

    for (i = 0; i < MAX_TEST_THREAD; i++)
    {
        IDE_ASSERT(idlOS::thr_join(gTest[i].getHandle(), NULL) == 0);
        gMiss += gTest[i].getMiss();
    }

	pdl_end_time = idlOS::gettimeofday();
    v_timeval = pdl_end_time - pdl_start_time;
    printf("4. Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    if (gMiss != LOOP_COUNT * MAX_TEST_THREAD)
    {
        smDriverMessage("Failure of tryRead test (%"ID_UINT32_FMT")\n", gMiss);
    }
    else
    {
        smDriverMessage("Success\n");
    }

    IDE_TEST(gMutex.unlock() != IDE_SUCCESS);



    return ID_TRUE;
    IDE_EXCEPTION_END;
    ideDump();
    
    return ID_FALSE;
}

// registration of the testing code 

int main()
{
    if (idp::initialize() != IDE_SUCCESS)
    {
        idlOS::fprintf(stderr, "ERR=>%s\n", idp::getErrorBuf());
        idlOS::exit(0);
    }

    iduProperty::load();
    
    idlOS::fprintf(stderr, "MUTEX TYPE = %d\n",
                   iduProperty::getMutexType());
        
    
    IDE_ASSERT(iduMutexMgr::initializeStatic() == IDE_SUCCESS);

    IDE_ASSERT(initialize() == IDE_SUCCESS);
    IDE_ASSERT(test() == ID_TRUE);
    IDE_ASSERT(destroy() == IDE_SUCCESS);
}

