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
 
// Decompiled by Jad v1.5.8g. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://www.kpdus.com/jad.html
// Decompiler options: packimports(3) 
// Source File Name:   CSVWriter.java

package com.altibase.altimon.file.csv;

import java.io.*;
import java.util.Iterator;
import java.util.List;

public class CSVWriter
implements Closeable, Flushable
{

    public CSVWriter(Writer writer)
    {
        this(writer, ',');
    }

    public CSVWriter(Writer writer, char separator)
    {
        this(writer, separator, '"');
    }

    public CSVWriter(Writer writer, char separator, char quotechar)
    {
        this(writer, separator, quotechar, '"');
    }

    public CSVWriter(Writer writer, char separator, char quotechar, char escapechar)
    {
        this(writer, separator, quotechar, escapechar, "\n");
    }

    public CSVWriter(Writer writer, char separator, char quotechar, String lineEnd)
    {
        this(writer, separator, quotechar, '"', lineEnd);
    }

    public CSVWriter(Writer writer, char separator, char quotechar, char escapechar, String lineEnd)
    {
        rawWriter = writer;
        pw = new PrintWriter(writer);
        this.separator = separator;
        this.quotechar = quotechar;
        this.escapechar = escapechar;
        this.lineEnd = lineEnd;
    }

    public void writeAll(List allLines, boolean applyQuotesToAll)
    {
        String line[];
        for(Iterator i$ = allLines.iterator(); i$.hasNext(); writeNext(line, applyQuotesToAll))
            line = (String[])i$.next();

    }

    public void writeAll(List allLines)
    {
        String line[];
        for(Iterator i$ = allLines.iterator(); i$.hasNext(); writeNext(line))
            line = (String[])i$.next();

    }

    public void writeNext(String nextLine[], boolean applyQuotesToAll)
    {
        if(nextLine == null)
            return;
        StringBuilder sb = new StringBuilder(128);
        for(int i = 0; i < nextLine.length; i++)
        {
            if(i != 0)
                sb.append(separator);
            String nextElement = nextLine[i];
            if(nextElement == null)
                continue;
            Boolean stringContainsSpecialCharacters = Boolean.valueOf(stringContainsSpecialCharacters(nextElement));
            if((applyQuotesToAll || stringContainsSpecialCharacters.booleanValue()) && quotechar != 0)
                sb.append(quotechar);
            if(stringContainsSpecialCharacters.booleanValue())
                sb.append(processLine(nextElement));
            else
                sb.append(nextElement);
            if((applyQuotesToAll || stringContainsSpecialCharacters.booleanValue()) && quotechar != 0)
                sb.append(quotechar);
        }

        sb.append(lineEnd);
        pw.write(sb.toString());
    }

    public void writeNext(String nextLine[])
    {
        writeNext(nextLine, true);
    }

    private boolean stringContainsSpecialCharacters(String line)
    {
        return line.indexOf(quotechar) != -1 || line.indexOf(escapechar) != -1 || line.indexOf(separator) != -1 || line.contains("\n") || line.contains("\r");
    }

    protected StringBuilder processLine(String nextElement)
    {
        StringBuilder sb = new StringBuilder(128);
        for(int j = 0; j < nextElement.length(); j++)
        {
            char nextChar = nextElement.charAt(j);
            if(escapechar != 0 && nextChar == quotechar)
            {
                sb.append(escapechar).append(nextChar);
                continue;
            }
            if(escapechar != 0 && nextChar == escapechar)
                sb.append(escapechar).append(nextChar);
            else
                sb.append(nextChar);
        }

        return sb;
    }

    public void flush()
    throws IOException
    {
        pw.flush();
    }

    public void close()
    throws IOException
    {
        flush();
        pw.close();
        rawWriter.close();
    }

    public boolean checkError()
    {
        return pw.checkError();
    }

    public void flushQuietly()
    {
        try
        {
            flush();
        }
        catch(IOException e) { }
    }

    public static final int INITIAL_STRING_SIZE = 128;
    public static final char DEFAULT_ESCAPE_CHARACTER = 34;
    public static final char DEFAULT_SEPARATOR = 44;
    public static final char DEFAULT_QUOTE_CHARACTER = 34;
    public static final char NO_QUOTE_CHARACTER = 0;
    public static final char NO_ESCAPE_CHARACTER = 0;
    public static final String DEFAULT_LINE_END = "\n";
    private Writer rawWriter;
    private PrintWriter pw;
    private char separator;
    private char quotechar;
    private char escapechar;
    private String lineEnd;
}
