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
 
#include <actTest.h>
#include <idTypes.h>
#include <iloCommandCompiler.h>


#define MAX_QUERYBUF_SIZE (100)

/* unittest
 * iloCommandCompiler::IsNullCommand
 */


void testIsNullCommand()
{
    iloCommandCompiler  sIloCommandCompiler;
    acp_char_t   sStr1[MAX_QUERYBUF_SIZE];
    acp_char_t   sStr2[MAX_QUERYBUF_SIZE];
    acp_char_t   sStr3[MAX_QUERYBUF_SIZE];
    acp_char_t   sStr4[MAX_QUERYBUF_SIZE];
    acp_char_t   sStr5[MAX_QUERYBUF_SIZE];

    acpCStrCpy( sStr1,
                MAX_QUERYBUF_SIZE,
                (acp_char_t *)"command",
                acpCStrLen((acp_char_t *)"command", ACP_SINT16_MAX) );
    acpCStrCpy( sStr2,
                MAX_QUERYBUF_SIZE,
                (acp_char_t *)" ",
                acpCStrLen((acp_char_t *)" ", ACP_SINT16_MAX) );
    acpCStrCpy( sStr3,
                MAX_QUERYBUF_SIZE,
                (acp_char_t *)"\t",
                acpCStrLen((acp_char_t *)"\t", ACP_SINT16_MAX) );
    acpCStrCpy( sStr4,
                MAX_QUERYBUF_SIZE,
                (acp_char_t *)"\n",
                acpCStrLen((acp_char_t *)"\n", ACP_SINT16_MAX) );
    acpCStrCpy( sStr5,
                MAX_QUERYBUF_SIZE,
                (acp_char_t *)" \t\ncommand",
                acpCStrLen((acp_char_t *)" \t\ncommand", ACP_SINT16_MAX) );

    ACT_CHECK( 0 == sIloCommandCompiler.IsNullCommand(sStr1) );
    ACT_CHECK( 1 == sIloCommandCompiler.IsNullCommand(sStr2) );
    ACT_CHECK( 1 == sIloCommandCompiler.IsNullCommand(sStr3) );
    ACT_CHECK( 1 == sIloCommandCompiler.IsNullCommand(sStr4) );
    ACT_CHECK( 0 == sIloCommandCompiler.IsNullCommand(sStr5) );

}

acp_sint32_t main( void )
{
    ACT_TEST_BEGIN();
    testIsNullCommand();
    ACT_TEST_END();

    return 0;
}
