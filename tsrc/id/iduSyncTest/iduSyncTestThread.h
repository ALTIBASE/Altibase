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
 
#ifndef _IDU_SYNC_TEST_THREAD_H_
#define _IDU_SYNC_TEST_THREAD_H_ 1

#include <idl.h>
#include <idtBaseThread.h>
#include <iduFile.h>

class iduSyncTestThread : public idtBaseThread
{    
public:
    iduSyncTestThread();    
    ~iduSyncTestThread();

    IDE_RC initialize(SInt aThrIdx);
    IDE_RC writeBlock();
    IDE_RC destroy();
    virtual void   run();    

private:
    iduFile mDataFile;
    iduFile mLogFile;    
    SInt    mThrIdx;    
    SChar   mDataFileName[ID_MAX_FILE_NAME];
    SChar   mLogFileName[ID_MAX_FILE_NAME];  
};

#endif  /* _IDU_SYNC_TEST_THREAD_H_ */
