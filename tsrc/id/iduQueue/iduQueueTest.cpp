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
#include <sys/time.h>


#define QUEUE_CAPACITY       5000000

#define ENQUEUE_THREAD_COUNT 5
#define ENQUEUE_LOOP_COUNT   1000000

#define DEQUEUE_THREAD_COUNT 5
#define DEQUEUE_LOOP_COUNT   1000000


iduQueue gQueue;


class enqueueThread : public idtBaseThread
{
public:
    enqueueThread() : idtBaseThread(THR_BOUND)
    {
    }

    void run()
    {
        vULong i;

        IDE_ASSERT(ideAllocErrorSpace() == IDE_SUCCESS);

        for (i = 0; i < ENQUEUE_LOOP_COUNT; i++)
        {
            IDE_ASSERT(iduQueueEnqueue(&gQueue, (void *)(i + 1)) == IDE_SUCCESS);
        }
    }
};

class dequeueThread : public idtBaseThread
{
public:
    dequeueThread() : idtBaseThread(THR_BOUND)
    {
    }

    void run()
    {
        void *sValue;
        UInt  i;

        IDE_ASSERT(ideAllocErrorSpace() == IDE_SUCCESS);

        for (i = 0; i < DEQUEUE_LOOP_COUNT; i++)
        {
            while (iduQueueDequeue(&gQueue, &sValue) != IDE_SUCCESS)
            {
                idlOS::thr_yield();
            }

//             idlOS::printf("%" ID_vULONG_FMT "\n", (vULong)sValue);
        }
    }
};


int main(int argc, char *argv[])
{
    enqueueThread  sEnqueueThread[ENQUEUE_THREAD_COUNT];
    dequeueThread  sDequeueThread[DEQUEUE_THREAD_COUNT];
    UInt           i;
    struct timeval tval1;
    struct timeval tval2;

    if (argc < 2)
    {
        idlOS::fprintf(stderr, "iduQueueTest QUEUE_TYPE\n");
        exit(1);
    }

    IDE_ASSERT(idp::initialize() == IDE_SUCCESS);

    iduProperty::load();

    IDE_ASSERT(iduMutexMgr::initializeStatic() == IDE_SUCCESS);

    IDE_ASSERT(iduQueueInitialize(&gQueue,
                                  (UInt)QUEUE_CAPACITY,
                                  (iduQueueType)atoi(argv[1]),
                                  IDU_MEM_ID) == IDE_SUCCESS);

    gettimeofday(&tval1, NULL);

    for (i = 0; i < ENQUEUE_THREAD_COUNT; i++)
    {
        IDE_ASSERT(sEnqueueThread[i].start() == IDE_SUCCESS);
    }
    for (i = 0; i < DEQUEUE_THREAD_COUNT; i++)
    {
        IDE_ASSERT(sDequeueThread[i].start() == IDE_SUCCESS);
    }

    for (i = 0; i < ENQUEUE_THREAD_COUNT; i++)
    {
        IDE_ASSERT(idlOS::thr_join(sEnqueueThread[i].getHandle(), NULL) == IDE_SUCCESS);
    }
    for (i = 0; i < DEQUEUE_THREAD_COUNT; i++)
    {
        IDE_ASSERT(idlOS::thr_join(sDequeueThread[i].getHandle(), NULL) == IDE_SUCCESS);
    }

    gettimeofday(&tval2, NULL);

    if (tval1.tv_usec > tval2.tv_usec)
    {
        tval2.tv_sec  -= 1;
        tval2.tv_usec += 1000000;
    }

    idlOS::fprintf(stderr, "%" ID_vULONG_FMT ".%" ID_vULONG_FMT "sec\n", tval2.tv_sec - tval1.tv_sec, tval2.tv_usec - tval1.tv_usec);

    IDE_ASSERT(iduQueueFinalize(&gQueue) == IDE_SUCCESS);

    return 0;
}
