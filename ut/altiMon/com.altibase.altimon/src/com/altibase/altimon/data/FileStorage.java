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

public class FileStorage extends AbstractHistoryStorage{

    private static final long serialVersionUID = 4858896798320542697L;

    public FileStorage(String aType) {
        mType = aType;
    }

    public void saveData(List aData) {
        FileManager.getInstance().saveHistoryData(aData);
    }

    public String getType() {
        return mType;
    }
}
