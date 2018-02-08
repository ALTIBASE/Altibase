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
 
package com.altibase.altimon.util;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.altibase.altimon.file.FileManager;

public class StringUtil {
    public static final StringUtil uniqueInstance = new StringUtil();

    public static final String LINE_SEPARATOR = System.getProperty("line.separator");
    public static final String FILE_SEPARATOR = System.getProperty("file.separator");	

    private StringUtil() {}

    public static StringUtil getInstance() {
        return uniqueInstance;
    }

    public String getStringFromFile(String fileName)
    {
        return new String(FileManager.getInstance().getFileToByteArray(fileName));
    }

    /**
     * If you pass the regular expression as a Pattern class, you can get String which you want to find from text 
     * 
     * @param text This param is original text which has property type characters just like A=B.
     * @param find The matching pattern to find target string from text param.
     * @return Target string
     * @throws ArrayIndexOutOfBoundsException
     */
    public String getPropValueFromCharseq(CharSequence text, Pattern find) throws ArrayIndexOutOfBoundsException {
        String retVal = null;

        Matcher matcher = find.matcher(text);

        if(matcher.find())
        {	
            retVal = matcher.group().split("=")[1].trim();		// It could occur boundary exception	
        }

        return retVal;
    }

    public static boolean isNotBlank(String userName) {
        if(userName!=null && !userName.contains(" ")) {
            return true;
        }
        else
        {
            return false;
        }
    }
}
