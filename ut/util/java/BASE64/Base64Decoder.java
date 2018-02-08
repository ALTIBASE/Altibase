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
 
import java.io.*;

public class Base64Decoder
{

    public Base64Decoder(String s)
    {
        in = null;
        out = null;
        byte abyte0[] = null;
        try
        {
            abyte0 = s.getBytes("ISO-8859-1");
        }
        catch(UnsupportedEncodingException unsupportedencodingexception) { }
        in = new ByteArrayInputStream(abyte0);
        out = new ByteArrayOutputStream();
    }

    public byte[] decodeBase64()
        throws IOException
    {
        process();
        return ((ByteArrayOutputStream)out).toByteArray();
    }

    private void printHex(int i)
    {
        int j = (i & 0xf0) >> 4;
        int k = i & 0xf;
        System.out.print((new Character((char)(j <= 9 ? 48 + j : (65 + j) - 10))).toString() + (new Character((char)(k <= 9 ? 48 + k : (65 + k) - 10))).toString());
    }

    private void printHex(byte abyte0[], int i, int j)
    {
        while(i < j) 
        {
            printHex(abyte0[i++]);
            System.out.print(" ");
        }

        System.out.println("");
    }

    private void printHex(String s)
    {
        byte abyte0[];
        try
        {
            abyte0 = s.getBytes("ISO-8859-1");
        }
        catch(UnsupportedEncodingException unsupportedencodingexception)
        {
            abyte0 = null;
        }
        printHex(abyte0, 0, abyte0.length);
    }

    private final int get1(byte abyte0[], int i)
    {
        return (abyte0[i] & 0x3f) << 2 | (abyte0[i + 1] & 0x30) >>> 4;
    }

    private final int get2(byte abyte0[], int i)
    {
        return (abyte0[i + 1] & 0xf) << 4 | (abyte0[i + 2] & 0x3c) >>> 2;
    }

    private final int get3(byte abyte0[], int i)
    {
        return (abyte0[i + 2] & 0x3) << 6 | abyte0[i + 3] & 0x3f;
    }

    private final int check(int i)
    {
        if(i >= 65 && i <= 90)
            return i - 65;
        if(i >= 97 && i <= 122)
            return (i - 97) + 26;
        if(i >= 48 && i <= 57)
            return (i - 48) + 52;
        switch(i)
        {
        case 61: // '='
            return 65;

        case 43: // '+'
            return 62;

        case 47: // '/'
            return 63;

        }
        return -1;
    }

    private void process()
        throws IOException
    {
        byte abyte0[] = new byte[1024];
        byte abyte1[] = new byte[4];
        int i = -1;
        int j = 0;
        while((i = in.read(abyte0)) > 0) 
        {
label0:
            for(int k = 0; k < i;)
            {
                while(j < 4) 
                {
                    if(k >= i)
                        break label0;
                    int l = check(abyte0[k++]);
                    if(l >= 0)
                        abyte1[j++] = (byte)l;
                }

                if(abyte1[2] == 65)
                {
                    out.write(get1(abyte1, 0));
                    return;
                }
                if(abyte1[3] == 65)
                {
                    out.write(get1(abyte1, 0));
                    out.write(get2(abyte1, 0));
                    return;
                }
                out.write(get1(abyte1, 0));
                out.write(get2(abyte1, 0));
                out.write(get3(abyte1, 0));
                j = 0;
            }

        }

        out.flush();
    }

    InputStream in;
    OutputStream out;
    private static final int BUFFER_SIZE = 1024;
}
