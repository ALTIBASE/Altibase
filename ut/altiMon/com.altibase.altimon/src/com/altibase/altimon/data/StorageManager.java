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

import com.altibase.altimon.file.FileManager;
import com.altibase.altimon.properties.DirConstants;

public class StorageManager{

    private static StorageManager uniqueInstance = new StorageManager();

    public static final String FILE_TYPE_STORAGE = "FILE";
    public static final String DB_TYPE_STORAGE = "DB";

    private AbstractHistoryStorage currentStorage = new FileStorage(StorageManager.FILE_TYPE_STORAGE);
    private AbstractStoreBehavior storeBehavior;

    private StorageManager() {};

    public static StorageManager getInstance() {
        return uniqueInstance;
    }

    public boolean isEmpty() {
        if(currentStorage==null) {
            return true;
        }
        else
        {
            return false;
        }
    }

    public boolean isFileType() {

        if(currentStorage == null) {
            return false;
        }

        if(currentStorage instanceof FileStorage) {
            return true;
        }
        else {
            return false;
        }
    }

    public void saveData(List aData) {
        currentStorage.saveData(aData);
    }

    public boolean isConnected() {

        if(isEmpty())
        {
            return false;
        }

        if(currentStorage.getType().equals(DB_TYPE_STORAGE)) {
            return HistoryDb.getInstance().isConnected();
        }
        else
        {
            return true;
        }
    }

    public boolean connect() {
        if(currentStorage.getType().equals(DB_TYPE_STORAGE))
        {
            return HistoryDb.getInstance().connect();
        }
        else
        {
            return true;
        }
    }

    public void removeStorage() {
        this.currentStorage = null;
    }

    private void initStoreBehavior() {
        storeBehavior = new StoreBehaviorImpl();
    }

    public void startBehavior() {
        initStoreBehavior();
        storeBehavior.startBehavior();
    }

    public void stopBehavior() {
        storeBehavior.stopBehavior();
    }
}
