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
 
package com.altibase.altimon.main;

public class Version
{
    /**
     * Main Class for activating application
     * @param args
     */
    public static void main(String[] args) {
        String versionNumber = "";
        try {
            java.io.File file = new java.io.File("altimon.jar");
            java.util.jar.JarFile jar = new java.util.jar.JarFile(file);
            java.util.jar.Manifest manifest = jar.getManifest();

            java.util.jar.Attributes attributes = manifest.getMainAttributes();
            if (attributes!=null){
                java.util.Iterator it = attributes.keySet().iterator();
                while (it.hasNext()){
                    java.util.jar.Attributes.Name key = (java.util.jar.Attributes.Name) it.next();
                    String keyword = key.toString();
                    if (keyword.equals("Bundle-Version")){
                        versionNumber = (String) attributes.get(key);
                        break;
                    }
                }
            }
            jar.close();
        } catch (java.io.IOException e) {
        }

        System.out.println("Altibase Version: " + versionNumber);
    }
}
