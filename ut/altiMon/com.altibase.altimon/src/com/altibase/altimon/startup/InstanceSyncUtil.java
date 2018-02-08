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
 
package com.altibase.altimon.startup;

import java.io.File;
import java.io.IOException;

import com.altibase.altimon.logging.AltimonLogger;

public class InstanceSyncUtil {
    private final String FILE_NAME = "uniqueinstance";
    private File lockFile = new File(FILE_NAME);

    /**
     * To recognize weather file exists or not.
     * @return Returns true if file exist, false otherwise. 
     */
    private boolean isExist()
    {
        boolean result = false;

        if(!(lockFile==null) && lockFile.isFile())
        {
            result = true;
        }

        return result; 
    }

    /**
     * If altimon is loaded, it will be guaranteed for a unique instance.
     * This method creates a temporary file to check weather altimon is running or not.
     * @return Returns true if a temporary file has already been created, false otherwise.
     */
    public boolean createSync()
    {
        boolean result = false;

        if(!isExist())
        {			
            try {
                lockFile.createNewFile();
                result = true;
            } catch (IOException e) {}			
        }

        return result;		
    }

    /**
     * When altimon is unloaded, it has to remove temporary file.
     * This method remove a temporary file.
     * @return Returns true if a temporary file is deleted successfully, false otherwise.
     */
    public boolean destroySync()
    {
        boolean result = false;

        if(isExist())
        {
            if(lockFile.delete())
            {				
                result = true;
            }			
        }

        return result;
    }
}
