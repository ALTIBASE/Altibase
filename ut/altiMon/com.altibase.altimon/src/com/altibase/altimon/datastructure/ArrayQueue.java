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
 
package com.altibase.altimon.datastructure;

import org.apache.log4j.*;

import com.altibase.altimon.logging.AltimonLogger;

public class ArrayQueue implements Queue
{
    private Object [ ] arrayQueue;
    private int        currentLength;
    private int        front;
    private int        back;

    private final int DEFAULT_CAPACITY = 10;

    public ArrayQueue( )
    {
        arrayQueue = new Object[ DEFAULT_CAPACITY ];
        makeEmpty( );
    }

    public boolean isEmpty( )
    {
        return currentLength == 0;
    }

    public void makeEmpty( )
    {
        currentLength = 0;
        front = 0;
        back = -1;
    }

    public synchronized Object dequeue() throws InterruptedException
    {
        while( isEmpty( ) ){        	
            wait();
        }

        currentLength--;

        Object returnValue = arrayQueue[ front ];
        arrayQueue[ front ] = null;
        front = increase( front );        
        notifyAll();
        return returnValue;
    }

    public Object getFront( )
    {
        if( isEmpty( ) )
            throw new UnderflowException( "ArrayQueue getFront" );
        return arrayQueue[ front ];
    }

    public synchronized void enqueue( Object x )
    {
        if( currentLength == arrayQueue.length )
            doubleQueue( );
        back = increase( back );
        arrayQueue[ back ] = x;
        currentLength++;
        notifyAll();
    }

    private int increase( int x )
    {
        if( ++x == arrayQueue.length )
            x = 0;
        return x;
    }

    private void doubleQueue( )
    {
        Logger theLogger = Logger.getLogger(AltimonLogger.class.getName());
        theLogger.info("altimon Internal Queue size increased.");

        Object [ ] newArray;

        newArray = new Object[ arrayQueue.length * 2 ];

        // Copy elements that are logically in the queue
        for( int i = 0; i < currentLength; i++, front = increase( front ) )
        {
            newArray[ i ] = arrayQueue[ front ];
            arrayQueue[ front ] = null;
        }

        arrayQueue = newArray;        
        front = 0;
        back = currentLength - 1;
    }

    public int length() {
        return currentLength;
    }   
}
