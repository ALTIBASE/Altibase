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
#include <iduLatch.h>
#include <iduProperty.h>

#define LOOP_COUNT 100
#define MAX_TEST_THREAD 200
#define MAX_LATCH_CNT   50


iduLatchObj gLatch[MAX_LATCH_CNT];
UInt         gCount = 0;
UInt         gMiss = 0;

#define BUSY_WAIT()  { int i, j; for (i = 0; i < (idlOS::rand() % 100000) + 200; i++) j += i; }


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
    int   k, i, j;
    idBool sFlag;

    UInt   sLN[(MAX_LATCH_CNT / 2)];
    
    IDE_ASSERT(ideAllocErrorSpace() == IDE_SUCCESS);
    for (k = 0; k < mCount; k++)
    {
        for (i = 0, j = 0; i < (MAX_LATCH_CNT / 2); i++)
        {
            j += (idlOS::rand() % 3);
            sLN[i] = j ;

            if (sLN[i] >= MAX_LATCH_CNT)
            {
                sLN[i] = MAX_LATCH_CNT; // end of array
            }
        }
#ifdef DBG        
        for (i = 0, j = 0; i < (MAX_LATCH_CNT / 2); i++)
        {
            idlOS::fprintf(stderr, "array[%d]=(%d) ", i, sLN[i] );
        }
        idlOS::fprintf(stderr, "\n");
#endif
        // HOLD MODE
        for (i = 0; i < (MAX_LATCH_CNT / 2); i++)
        {
            if (sLN[i] == MAX_LATCH_CNT)
            {
                break;
            }
            
            IDE_TEST(iduLatch::lockWrite(&gLatch[sLN[i]]) != IDE_SUCCESS);
            BUSY_WAIT();
        }
        
        for (i = (MAX_LATCH_CNT / 2) - 1; i >= 0; i--)
        {
            if (sLN[i] != MAX_LATCH_CNT)
            {
                IDE_TEST(iduLatch::unlock(&gLatch[sLN[i]]) != IDE_SUCCESS);
            }
        }
    }
    return;
    IDE_EXCEPTION_END;
    smDriverMessage("FATAL ERROR");
}

//=======================================================================

static IDE_RC initialize()
{
    UInt i;
    
    for (i = 0; i < MAX_LATCH_CNT; i++)
    {
        IDE_TEST(iduLatch::initialize(&gLatch[i]) != IDE_SUCCESS);
    }
    
        
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static IDE_RC destroy()
{
    //IDE_TEST(iduLatch::destroy(&gLatch) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

latchThread gTest[MAX_TEST_THREAD];


#define T_BEGIN() pdl_start_time = idlOS::gettimeofday()
#define T_END()   pdl_end_time = idlOS::gettimeofday()
#define T_PRINT(a) v_timeval = pdl_end_time - pdl_start_time;\
                   printf(#a" Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());

static idBool test()
{
    UInt i;
	PDL_Time_Value pdl_start_time, pdl_end_time, v_timeval;

    smDriverMessage("Test Random test \n");
    idlOS::srand(idlOS::time(NULL));

    for (i = 0; i < MAX_TEST_THREAD; i++)
    {
        IDE_TEST(gTest[i].initialize(LT_RANDOM,  LOOP_COUNT) != IDE_SUCCESS);
        IDE_TEST(gTest[i].start() != IDE_SUCCESS);
        IDE_TEST(gTest[i].waitToStart(0) != IDE_SUCCESS);
    }
    
    T_BEGIN();

    for (i = 0; i < MAX_TEST_THREAD; i++)
    {
        IDE_ASSERT(idlOS::thr_join(gTest[i].getHandle(), NULL) == 0);
    }

    T_END();
    T_PRINT(random);
    
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
    
    idlOS::fprintf(stderr, "LATCH TYPE = %d\n",
                   iduProperty::getLatchType());
        
    
    IDE_ASSERT(iduMutexMgr::initializeStatic() == IDE_SUCCESS);

    IDE_ASSERT(initialize() == IDE_SUCCESS);
    IDE_ASSERT(test() == ID_TRUE);
    IDE_ASSERT(destroy() == IDE_SUCCESS);
}

