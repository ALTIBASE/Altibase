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
 
#include <utString.h>
#include <actTest.h>


#define MAX_BUF_SIZE (100)

/* unittest
 * utString::eraseWhiteSpace
 * utString::removeLastCR
 */


void testEraseWhiteSpace()
{
    acp_char_t   sStr1[MAX_BUF_SIZE];
    acp_char_t   sStr2[MAX_BUF_SIZE];
    acp_char_t   sStr3[MAX_BUF_SIZE];
    acp_char_t   sStr4[MAX_BUF_SIZE];
    acp_char_t   sStr5[MAX_BUF_SIZE];
    acp_char_t   sStr6[MAX_BUF_SIZE];

    acpCStrCpy( sStr1,
                MAX_BUF_SIZE,
                (acp_char_t *)"abc",
                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) );
    acpCStrCpy( sStr2,
                MAX_BUF_SIZE,
                (acp_char_t *)"  abc",
                acpCStrLen((acp_char_t *)"  abc", ACP_SINT16_MAX) );
    acpCStrCpy( sStr3,
                MAX_BUF_SIZE,
                (acp_char_t *)"abc  ",
                acpCStrLen((acp_char_t *)"abc  ", ACP_SINT16_MAX) );
    acpCStrCpy( sStr4,
                MAX_BUF_SIZE,
                (acp_char_t *)"  abc  ",
                acpCStrLen((acp_char_t *)"  abc  ", ACP_SINT16_MAX) );
    acpCStrCpy( sStr5,
                MAX_BUF_SIZE,
                (acp_char_t *)"  abc  \r\r",
                acpCStrLen((acp_char_t *)"  abc  \r\r", ACP_SINT16_MAX) );
    acpCStrCpy( sStr6,
                MAX_BUF_SIZE,
                (acp_char_t *)"",
                acpCStrLen((acp_char_t *)"", ACP_SINT16_MAX) );

    utString::eraseWhiteSpace(sStr1);
    ACT_CHECK( 0 == acpCStrCmp( sStr1,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );
    utString::eraseWhiteSpace(sStr2);
    ACT_CHECK( 0 == acpCStrCmp( sStr2,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );
    utString::eraseWhiteSpace(sStr3);
    ACT_CHECK( 0 == acpCStrCmp( sStr3,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );
    utString::eraseWhiteSpace(sStr4);
    ACT_CHECK( 0 == acpCStrCmp( sStr4,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );
    utString::eraseWhiteSpace(sStr5);
    ACT_CHECK( 0 == acpCStrCmp( sStr5,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );
    utString::eraseWhiteSpace(sStr6);
    ACT_CHECK( 0 == acpCStrCmp( sStr6,
                                (acp_char_t *)"",
                                acpCStrLen((acp_char_t *)"", ACP_SINT16_MAX) ) );

}

void testRemoveLastCR()
{
    acp_char_t   sStr1[MAX_BUF_SIZE];
    acp_char_t   sStr2[MAX_BUF_SIZE];
    acp_char_t   sStr3[MAX_BUF_SIZE];
    acp_char_t   sStr4[MAX_BUF_SIZE];


    acpCStrCpy( sStr1,
                MAX_BUF_SIZE,
                (acp_char_t *)"abc",
                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) );
    acpCStrCpy( sStr2,
                MAX_BUF_SIZE,
                (acp_char_t *)"abc\n",
                acpCStrLen((acp_char_t *)"abc\n", ACP_SINT16_MAX) );
    acpCStrCpy( sStr3,
                MAX_BUF_SIZE,
                (acp_char_t *)"abc\t",
                acpCStrLen((acp_char_t *)"abc\t", ACP_SINT16_MAX) );
    acpCStrCpy( sStr4,
                MAX_BUF_SIZE,
                (acp_char_t *)"",
                acpCStrLen((acp_char_t *)"", ACP_SINT16_MAX) );

    utString::removeLastCR(sStr1);
    ACT_CHECK( 0 == acpCStrCmp( sStr1,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );
    utString::removeLastCR(sStr2);
    ACT_CHECK( 0 == acpCStrCmp( sStr2,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );
    utString::removeLastCR(sStr3);
    ACT_CHECK( 0 == acpCStrCmp( sStr3,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );
    utString::removeLastCR(sStr4);
    ACT_CHECK( 0 == acpCStrCmp( sStr4,
                                (acp_char_t *)"",
                                acpCStrLen((acp_char_t *)"", ACP_SINT16_MAX) ) );

}

acp_sint32_t main( void )
{
    ACT_TEST_BEGIN();
    testEraseWhiteSpace();
    testRemoveLastCR();
    ACT_TEST_END();

    return 0;
}

