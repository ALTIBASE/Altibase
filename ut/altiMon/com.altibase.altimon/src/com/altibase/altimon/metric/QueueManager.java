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
 
package com.altibase.altimon.metric;

import java.util.Collections;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.altibase.altimon.datastructure.ArrayQueue;
import com.altibase.altimon.datastructure.Queue;
import com.altibase.altimon.logging.AltimonLogger;

public class QueueManager {
    public static QueueManager uniqueInstance = new QueueManager();

    private Queue mQueue = new ArrayQueue();

    private Map mQueueList4RealtimeProblem = Collections.synchronizedMap(new Hashtable());

    private QueueManager() {}

    public static QueueManager getInstance() {
        return uniqueInstance;
    }

    public Queue getQueue() {
        return mQueue;
    }	

    public Queue getQueue(String id) {
        if(mQueueList4RealtimeProblem.containsKey(id)) 
        {
            return (Queue)mQueueList4RealtimeProblem.get(id);
        }
        else
        {
            mQueueList4RealtimeProblem.put(id, new ArrayQueue());
            return (Queue)mQueueList4RealtimeProblem.get(id);
        }
    }

    public void transitAlert(List alarmResult) {
        StringBuffer alarmSeq = new StringBuffer();		
        alarmSeq.append(alarmResult.get(Metric.ALARM_DATE_IDX).toString()).append("|")
        .append(alarmResult.get(Metric.ALARM_NAME_IDX).toString()).append("|")
        .append(alarmResult.get(Metric.ALARM_VALUE_IDX).toString()).append("|")
        .append(alarmResult.get(Metric.ALARM_TYPE_IDX).toString());

        Iterator iter = mQueueList4RealtimeProblem.values().iterator();
        while(iter.hasNext()) {			
            Queue queue = (Queue)iter.next();
            if(queue.length()>1) {
                AltimonLogger.theLogger.info("Realtime Alarm Queue has been removed.");				
                queue.makeEmpty();
                iter.remove();
            }
            else
            {				
                queue.enqueue(alarmSeq.toString());
            }
        }
    }
}
