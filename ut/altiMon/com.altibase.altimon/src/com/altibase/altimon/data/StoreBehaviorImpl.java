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

import java.util.List;

import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.QueueManager;

public class StoreBehaviorImpl extends AbstractStoreBehavior {

    public StoreBehaviorImpl() {
        this.mQueue = QueueManager.getInstance().getQueue();
    }

    public void run() {
        List result = null;

        try{
            while(true) {
                result = (List)mQueue.dequeue();

                StorageManager.getInstance().saveData(result);

                result.clear();
            }
        }
        catch(InterruptedException e)
        {
            String symptom = "[Symptom    ] : Thread has been stopped storing because an InterruptedException has occurred.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
    }
}
