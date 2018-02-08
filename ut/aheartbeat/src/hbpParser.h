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
 
#ifndef _HBP_PARSER_H_
#define _HBP_PARSER_H_ 1

#include <hbp.h>


/* string 의 delimiter를 사용하여 하나의 token을 뱉는 함수
 *
 * aSrcStr (input) - HBP_setting_file에서 읽어온 string ( \n 포함 )
 * aOffset (output) - 다음 token을 가져올 위치 
 * aDstStr (output) - token이저장되는 곳
 */
acp_sint32_t hbpGetToken( acp_char_t       *aSrcStr,
                          acp_sint32_t      aSrcLength,
                          acp_sint32_t    *aOffset,
                          acp_char_t      *aDstStr );


/* aHBPInfo에 읽어온 host 파싱해서 넣는 함수
 * 
 * aStr (input) - HBP_setting_file에서 읽어온 string ( \n 포함 )
 * aHBPInfo (output) - 이 Array에 setting 정보를 넣는다.
 */
ACI_RC hbpAddHostToHBPInfo( acp_char_t* aStr, HBPInfo* aHBPInfo );

acp_size_t hbpGetInformationStrLen( acp_char_t *aLine );

acp_size_t hbpGetEqualOffset( acp_char_t *aLine, acp_size_t aLineLen );

ACI_RC hbpParseUserInfo( acp_char_t *aLine,
                         acp_char_t *aUserName,
                         acp_char_t *aPassWord );

/* does aString read from setting file have information?
 * aStr (input) - HBP_setting_file에서 읽어온 string ( \n 포함 )
 */
acp_bool_t hbpIsInformationLine( acp_char_t* aStr );

/* HBP_setting_file에서 status를 뽑아 StatusArray에 넣는 함수
 *
 * aFilePtr   (input)     - status 파일 포인터를 넘겨준다. 
 * aCount     (output)     - status count+ 1 (column..)
 * aInfoArray (output)    - HBP 정보를 담고있는 Array.
 */
ACI_RC hbpSetHBPInfoArray( acp_std_file_t   *aFilePtr,
                           HBPInfo          *aInfoArray,
                           acp_sint32_t     *aCount );
/* HBP_setting_file에서 status를 뽑아 print
 *
 * aFilePtr   (input)     - status 파일 포인터를 넘겨준다.
 * aCount     (input)     - status count+1 (column..)
 * */
void hbpPrintInfo( HBPInfo* aInfoArray, acp_sint32_t aCount );

/* read File and initialize HBPInfo
 *
 * aHBPInfo       (output)    - HBP 정보를 담고있는 Array.
 * aCount         (output)    - status count+ 1 (column..)
 */
ACI_RC hbpInitializeHBPInfo( HBPInfo *aHBPInfo, acp_sint32_t *aCount );

void hbpFinalizeHBPInfo( HBPInfo  *aHBPInfo, acp_sint32_t aCount );

void logMessage( ace_msgid_t aMessageId, ... );

/* set user info
 * aUserName    (input)
 * aPassWord    (input)
 */
ACI_RC hbpSetUserInfo( acp_char_t *aUserName, acp_char_t *aPassWord );

acp_bool_t hbpIsValidIPFormat(acp_char_t * aIP);


/* Env */

/* get My HBP ID from Environment variable
 *
 * aID       (output)    - HBP ID
 */
ACI_RC hbpGetMyHBPID( acp_sint32_t *aID );

void hbpGetVersionInfo( acp_char_t *aBannerContents, acp_sint32_t *aBannerLen );
#endif
