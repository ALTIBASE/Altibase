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
 
package com.altibase.altimon.data;

import com.altibase.altimon.datastructure.ArrayQueue;
import com.altibase.altimon.datastructure.Queue;
import com.altibase.altimon.metric.QueueManager;

public abstract class AbstractStoreBehavior extends Thread implements StoreBehaviorInterface {

    protected String mType;
    protected Queue mQueue;

    public void startBehavior() {
        if(!this.isAlive()) {
            this.start();
        }
        else
        {
            mQueue = QueueManager.getInstance().getQueue();
            this.notify();
        }
    }

    public void stopBehavior() {
        //if(this.isAlive()) {
        mQueue = new ArrayQueue();			
        //}
    }
}
