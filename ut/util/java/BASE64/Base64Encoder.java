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

public class Base64Encoder
{

    public Base64Encoder(byte abyte0[])
    {
        in = null;
        out = null;
        in = new ByteArrayInputStream(abyte0);
        out = new ByteArrayOutputStream();
    }

    public String encodeBase64()
        throws IOException
    {
        process();
        return ((ByteArrayOutputStream)out).toString();
    }

    private void process()
        throws IOException
    {
        byte abyte0[] = new byte[1024];
        int i = -1;
        int j = 0;
        int k = 0;
        while((i = in.read(abyte0, j, 1024 - j)) > 0) 
            if(i >= 3)
            {
                i += j;
                for(j = 0; j + 3 <= i; j += 3)
                {
                    int l = get1(abyte0, j);
                    int j1 = get2(abyte0, j);
                    int k1 = get3(abyte0, j);
                    int l1 = get4(abyte0, j);
                    switch(k)
                    {
                    case 73: // 'I'
                        out.write(bytsEncoding[l]);
                        out.write(bytsEncoding[j1]);
                        out.write(bytsEncoding[k1]);
                        out.write(10);
                        out.write(bytsEncoding[l1]);
                        k = 1;
                        break;

                    case 74: // 'J'
                        out.write(bytsEncoding[l]);
                        out.write(bytsEncoding[j1]);
                        out.write(10);
                        out.write(bytsEncoding[k1]);
                        out.write(bytsEncoding[l1]);
                        k = 2;
                        break;

                    case 75: // 'K'
                        out.write(bytsEncoding[l]);
                        out.write(10);
                        out.write(bytsEncoding[j1]);
                        out.write(bytsEncoding[k1]);
                        out.write(bytsEncoding[l1]);
                        k = 3;
                        break;

                    case 76: // 'L'
                        out.write(10);
                        out.write(bytsEncoding[l]);
                        out.write(bytsEncoding[j1]);
                        out.write(bytsEncoding[k1]);
                        out.write(bytsEncoding[l1]);
                        k = 4;
                        break;

                    default:
                        out.write(bytsEncoding[l]);
                        out.write(bytsEncoding[j1]);
                        out.write(bytsEncoding[k1]);
                        out.write(bytsEncoding[l1]);
                        k += 4;
                        break;

                    }
                }

                for(int i1 = 0; i1 < 3; i1++)
                    abyte0[i1] = i1 >= i - j ? 0 : abyte0[j + i1];

                j = i - j;
            }
            else
            {
                j += i;
            }

        switch(j)
        {
        case 1: // '\001'
            out.write(bytsEncoding[get1(abyte0, 0)]);
            out.write(bytsEncoding[get2(abyte0, 0)]);
            out.write(61);
            out.write(61);
            break;

        case 2: // '\002'
            out.write(bytsEncoding[get1(abyte0, 0)]);
            out.write(bytsEncoding[get2(abyte0, 0)]);
            out.write(bytsEncoding[get3(abyte0, 0)]);
            out.write(61);
            break;

        }
    }

    private final int get1(byte abyte0[], int i)
    {
        return (abyte0[i] & 0xfc) >> 2;
    }

    private final int get2(byte abyte0[], int i)
    {
        return (abyte0[i] & 0x3) << 4 | (abyte0[i + 1] & 0xf0) >>> 4;
    }

    private final int get3(byte abyte0[], int i)
    {
        return (abyte0[i + 1] & 0xf) << 2 | (abyte0[i + 2] & 0xc0) >>> 6;
    }

    private static final int get4(byte abyte0[], int i)
    {
        return abyte0[i + 2] & 0x3f;
    }

    InputStream in;
    OutputStream out;
    private static final int BUFFER_SIZE = 1024;
    private static byte bytsEncoding[] = {
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 
        75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 
        85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 
        101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 
        111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 
        121, 122, 48, 49, 50, 51, 52, 53, 54, 55, 
        56, 57, 43, 47, 61
    };

}
