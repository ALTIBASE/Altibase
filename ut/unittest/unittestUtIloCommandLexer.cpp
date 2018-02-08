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
 
#include <iloApi.h>
#include <actTest.h>

#define MAX_STR_SIZE (100)

/* unittest
 * SeparatorCopy
 */

extern void SeparatorCopy(char *szTar, char *szSrc, iloBool isEnclosure);

void testSeparatorCopy()
{
    acp_char_t   sStr1[MAX_STR_SIZE];
    acp_char_t   sStr2[MAX_STR_SIZE];
    acp_char_t   sStr3[MAX_STR_SIZE];
    acp_char_t   sStr4[MAX_STR_SIZE];
    acp_char_t   sRes[MAX_STR_SIZE];

    acpCStrCpy( sStr1,
                MAX_STR_SIZE,
                (acp_char_t *)"abc",
                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) );
    acpCStrCpy( sStr2,
                MAX_STR_SIZE,
                (acp_char_t *)"\"abc\"",
                acpCStrLen((acp_char_t *)"\"abc\"", ACP_SINT16_MAX) );
    acpCStrCpy( sStr3,
                MAX_STR_SIZE,
                (acp_char_t *)"%n%t%r",
                acpCStrLen((acp_char_t *)"%n%t%r", ACP_SINT16_MAX) );
    acpCStrCpy( sStr4,
                MAX_STR_SIZE,
                (acp_char_t *)"\"%n%t%r\"",
                acpCStrLen((acp_char_t *)"\"%n%t%r\"", ACP_SINT16_MAX) );

    SeparatorCopy(sRes, sStr1, ILO_TRUE);
    ACT_CHECK( 0 == acpCStrCmp( sRes,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );

    SeparatorCopy(sRes, sStr2, ILO_TRUE);
    ACT_CHECK( 0 == acpCStrCmp( sRes,
                                (acp_char_t *)"\"abc\"",
                                acpCStrLen((acp_char_t *)"\"abc\"", ACP_SINT16_MAX) ) );

    SeparatorCopy(sRes, sStr3, ILO_TRUE);
    ACT_CHECK( 0 == acpCStrCmp( sRes,
                                (acp_char_t *)"\n\t\r",
                                acpCStrLen((acp_char_t *)"\n\t\r", ACP_SINT16_MAX) ) );

    SeparatorCopy(sRes, sStr4, ILO_TRUE);
    ACT_CHECK( 0 == acpCStrCmp( sRes,
                                (acp_char_t *)"\"\n\t\r\"",
                                acpCStrLen((acp_char_t *)"\"\n\t\r\"", ACP_SINT16_MAX) ) );

    SeparatorCopy(sRes, sStr1, ILO_FALSE);
    ACT_CHECK( 0 == acpCStrCmp( sRes,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );

    SeparatorCopy(sRes, sStr2, ILO_FALSE);
    ACT_CHECK( 0 == acpCStrCmp( sRes,
                                (acp_char_t *)"abc",
                                acpCStrLen((acp_char_t *)"abc", ACP_SINT16_MAX) ) );

    SeparatorCopy(sRes, sStr3, ILO_FALSE);
    ACT_CHECK( 0 == acpCStrCmp( sRes,
                                (acp_char_t *)"\n\t\r",
                                acpCStrLen((acp_char_t *)"\n\t\r", ACP_SINT16_MAX) ) );

    SeparatorCopy(sRes, sStr4, ILO_FALSE);
    ACT_CHECK( 0 == acpCStrCmp( sRes,
                                (acp_char_t *)"\n\t\r",
                                acpCStrLen((acp_char_t *)"\n\t\r", ACP_SINT16_MAX) ) );
}

acp_sint32_t main( void )
{
    ACT_TEST_BEGIN();
    testSeparatorCopy();
    ACT_TEST_END();

    return 0;
}
