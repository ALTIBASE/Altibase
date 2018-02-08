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
 
package com.altibase.altimon.connection;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Set;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class JarClassLoader extends ClassLoader {

    private Hashtable jarContents;

    public boolean isOpened()
    {
        return (jarContents != null)? true:false;
    }

    public void open(String jarFileName) throws IOException   
    {   
        // first get sizes of all files in the jar file   
        Hashtable sizes = new Hashtable();   

        ZipFile zf = new ZipFile(jarFileName);   

        Enumeration e = zf.entries();   
        while (e.hasMoreElements())   
        {   
            ZipEntry ze = (ZipEntry)e.nextElement();   
            // make sure we have only slashes, we prefer unix, not windows   
            sizes.put(ze.getName().replace('\\', '/'),    
                    new Integer((int)ze.getSize()));   
        }   

        zf.close();   

        // second get contents of the jar file and place each   
        // entry in a hashtable for later use   
        jarContents= new Hashtable();   

        JarInputStream jis =    
            new JarInputStream   
            (new BufferedInputStream   
                    (new FileInputStream(jarFileName)));   

        JarEntry je;   
        while ((je = jis.getNextJarEntry()) != null)   
        {   
            // make sure we have only slashes, we prefer unix, not windows   
            String name = je.getName().replace('\\', '/');   

            // get entry size from the entry or from our hashtable   
            int size;   
            if ((size = (int)je.getSize()) < 0)   
                size = ((Integer)sizes.get(name)).intValue();   

            // read the entry   
            byte[] ba = new byte[size];   
            int bytes_read = 0;   
            while (bytes_read != size)   
            {   
                int r = jis.read(ba, bytes_read, size - bytes_read);   
                if (r < 0)   
                    break;   
                bytes_read += r;   
            }   
            if (bytes_read != size)   
                throw new IOException("cannot read entry");   

            jarContents.put(name, ba);   
        }   

        jis.close();   
    }   

    public void close()
    {
        int i, size;

        if (jarContents == null)
            return;

        Set set = jarContents.keySet();

        size = set.size();
        String[] keys = (String[]) set.toArray(new String[set.size()]);

        for(i = 0; i < size; i++)
        {
            jarContents.remove(keys[i]);
        }

        set = null;
        jarContents = null;

    }	

    /**  
     * Looks among the contents of the jar file (cached in memory)  
     * and tries to find and define a class, given its name.  
     *  
     * @param className   the name of the class  
     * @return   a Class object representing our class  
     * @exception ClassNotFoundException   the jar file did not contain  
     * a class named <CODE>className</CODE>  
     */  
    public Class findClass(String className) throws ClassNotFoundException   
    {   
        String classPath = className.replace('.', '/') + ".class"; 		

        byte[] classBytes = (byte[])jarContents.get(classPath);   

        if (classBytes == null)   
            throw new ClassNotFoundException();   

        return defineClass(className, classBytes, 0, classBytes.length);   
    }   


    public synchronized InputStream getResourceAsStream(String name) {
        InputStream input = getParent().getResourceAsStream(name);
        if (input != null)
            return input;

        return new ByteArrayInputStream((byte[])jarContents.get(name));
    }

}
