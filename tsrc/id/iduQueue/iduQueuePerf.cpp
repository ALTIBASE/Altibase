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
#include <ide.h>
#include <idp.h>
#include <idu.h>
#include <idtBaseThread.h>


#define QUEUE_CAPACITY       200000000


iduQueue gQueue;

idBool   gStop = ID_FALSE;


class enqueueThread : public idtBaseThread
{
public:
    vULong mCount;

    enqueueThread() : idtBaseThread(THR_BOUND)
    {
        mCount = 0;
    }

    void run()
    {
        IDE_ASSERT(ideAllocErrorSpace() == IDE_SUCCESS);

        while (gStop != ID_TRUE)
        {
            IDE_ASSERT(iduQueueEnqueue(&gQueue, (void *)(++mCount)) == IDE_SUCCESS);
        }
    }
};

class dequeueThread : public idtBaseThread
{
public:
    UInt mCount;

    dequeueThread() : idtBaseThread(THR_BOUND)
    {
        mCount = 0;
    }

    void run()
    {
        void   *sValue;
        IDE_RC  sRet;

        IDE_ASSERT(ideAllocErrorSpace() == IDE_SUCCESS);

        while (gStop != ID_TRUE)
        {
            sRet = iduQueueDequeue(&gQueue, &sValue);

            if (sRet == IDE_SUCCESS)
            {
                mCount++;
            }
            else
            {
                idlOS::thr_yield();
            }
        }
    }
};


int main(int argc, char *argv[])
{
    UInt            sQueueType;
    UInt            sEnqueueThreadCount;
    UInt            sDequeueThreadCount;
    UInt            sTimeout;
    enqueueThread **sEnqueueThread;
    dequeueThread **sDequeueThread;
    UInt            sEnqueueCount = 0;
    UInt            sDequeueCount = 0;
    UInt            i;

    if (argc < 5)
    {
        idlOS::printf("iduQueuePerf QUEUE_TYPE ENQUEUE_THREAD_COUNT DEQUEUE_THREAD_COUNT TIMEOUT_IN_SEC\n");
        exit(1);
    }

    sQueueType          = atoi(argv[1]);
    sEnqueueThreadCount = atoi(argv[2]);
    sDequeueThreadCount = atoi(argv[3]);
    sTimeout            = atoi(argv[4]);

    IDE_ASSERT(idp::initialize() == IDE_SUCCESS);

    iduProperty::load();

    IDE_ASSERT(iduMutexMgr::initializeStatic() == IDE_SUCCESS);

    IDE_ASSERT(iduQueueInitialize(&gQueue,
                                  (UInt)QUEUE_CAPACITY,
                                  (iduQueueType)sQueueType,
                                  IDU_MEM_ID) == IDE_SUCCESS);

    sEnqueueThread = (enqueueThread **)idlOS::malloc(sEnqueueThreadCount * sizeof(enqueueThread *));
    sDequeueThread = (dequeueThread **)idlOS::malloc(sDequeueThreadCount * sizeof(dequeueThread *));

    for (i = 0; i < sDequeueThreadCount; i++)
    {
        sDequeueThread[i] = new dequeueThread();
        IDE_ASSERT(sDequeueThread[i]->start() == IDE_SUCCESS);
    }
    for (i = 0; i < sEnqueueThreadCount; i++)
    {
        sEnqueueThread[i] = new enqueueThread();
        IDE_ASSERT(sEnqueueThread[i]->start() == IDE_SUCCESS);
    }

    idlOS::sleep(sTimeout);

    gStop = ID_TRUE;

    for (i = 0; i < sEnqueueThreadCount; i++)
    {
        IDE_ASSERT(idlOS::thr_join(sEnqueueThread[i]->getHandle(), NULL) == IDE_SUCCESS);
        idlOS::printf("ENQUEUE[%3" ID_UINT32_FMT "] = %" ID_vULONG_FMT "\n", i, sEnqueueThread[i]->mCount);
        sEnqueueCount += sEnqueueThread[i]->mCount;
        delete sEnqueueThread[i];
    }
    for (i = 0; i < sDequeueThreadCount; i++)
    {
        IDE_ASSERT(idlOS::thr_join(sDequeueThread[i]->getHandle(), NULL) == IDE_SUCCESS);
        idlOS::printf("DEQUEUE[%3" ID_UINT32_FMT "] = %" ID_UINT32_FMT "\n", i, sDequeueThread[i]->mCount);
        sDequeueCount += sDequeueThread[i]->mCount;
        delete sDequeueThread[i];
    }

    idlOS::printf("-----------------\n");
    idlOS::printf("ENQUEUE SUM  = %" ID_UINT32_FMT "\n", sEnqueueCount);
    idlOS::printf("DEQUEUE SUM  = %" ID_UINT32_FMT "\n", sDequeueCount);

    idlOS::free(sEnqueueThread);
    idlOS::free(sDequeueThread);

    IDE_ASSERT(iduQueueFinalize(&gQueue) == IDE_SUCCESS);

    return 0;
}
