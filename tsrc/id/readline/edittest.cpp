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
 
#include <stdio.h>
#include <assert.h>
#include <editline/readline.h>

//  static EditLine *gEdObj = NULL;

int main(int argc, char **argv)

{

    assert(rl_initialize() == 0);

    while(1)
    {
        char *sInput;

        sInput = readline("test>> ");
        if (sInput != NULL)
        {
            fprintf(stdout, "INPUT> [%s]\n", sInput);
        }
        else
        {
            fprintf(stdout, "INPUT> None\n");
        }
    }
    
    
    
//      gEdObj = el_init(argv[0], stdin, stdout, stderr);

//      assert(gEdObj != NULL);

//      while(1)
//      {
//          SInt sLen;
//          SChar *sInput;
        
//          sInput = el_gets(gEdObj, &sLen);

//          if (sInput != NULL)
//          {
//              fprintf(stdout, "INPUT> [%s]\n", sInput);
//          }
//          else
//          {
//              fprintf(stdout, "INPUT> None\n");
//          }
//      }
    
    return 0;
    
}



