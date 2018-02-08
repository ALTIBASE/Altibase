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
 
#include <idl.h>

SChar *gValue[] =
{
    "0",
    "1",
    "10K",
    "-5",
    "10M",
    "100m",
    "1G",
    "2g",
    
    "  512",
    "512  ",
    "  512M ",
    "13k  ",
    " 128  ",
    
    "-128K",
    "-128k",
    "utim123",
    "-123s",
    "-12 3s",
    "-18M 11",
    "13dmd",
    "1 28",
    " 12 8   ",
    " 12 8m   ",
    " 12 8k   ",
    
    NULL,
    
};

int main()
{
    int i;

    idBool sValid;
    
    for (i = 0; gValue[i] != NULL; i++)
    {
        SLong val;

        val = idlVA::fstrToLong(gValue[i], &sValid);

        if (sValid == ID_TRUE)
        {
            idlOS::fprintf(stderr,
                           "[%s] : %"ID_INT64_FMT"\n",
                           gValue[i], val);
        }
        else
        {
            idlOS::fprintf(stderr,
                           "[%s] : INVALID !!\n", gValue[i]);
        }
        
    }
    

    return 0;
}
