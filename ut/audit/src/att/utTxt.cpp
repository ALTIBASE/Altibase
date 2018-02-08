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
 
/*******************************************************************************
 * $Id: utTxt.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utTxt.h>

#define SQL_C_NATIVE                    20008

IDL_EXTERN_C_BEGIN
#if defined(HP_HPUX) || defined(IA64_HP_HPUX)
    SInt _init(void) { return 0;}
    SInt _fini(void) { return 0;}
#endif

    /* static initializer aka _init */
    IDE_RC _dbd_initialize(void *)
    {
        return IDE_SUCCESS;
    }

    Connection * _new_connection(dbDriver * dbd)
    {
        return new utTxtConnection(dbd);
    }


    SInt sqlTypeToAltibase(SInt type)
    {
        switch (type)
        {
            case SQL_CHAR       :
            case SQL_VARCHAR    :{type   = SQL_C_CHAR   ;}break;
            case SQL_FLOAT      :
            case SQL_NUMERIC    :
            case SQL_DECIMAL    :{type = SQL_C_NATIVE   ;} break;
            case SQL_REAL       :{type = SQL_C_FLOAT    ;} break;
            case SQL_DOUBLE     :{type = SQL_C_DOUBLE   ;} break;
            case SQL_TINYINT    :{type = SQL_C_STINYINT ;} break;
            case SQL_BIT        :{type = SQL_C_BIT      ;} break;
            case SQL_SMALLINT   :{type = SQL_C_SSHORT   ;} break;
            case SQL_INTEGER    :{type = SQL_C_SLONG    ;} break;
            case SQL_BIGINT     :{type = SQL_C_SBIGINT  ;} break;

            case SQL_DATE       :
            case SQL_TIME       :
            case SQL_TIMESTAMP  :{type = SQL_C_NATIVE   ;} break;
            
            case SQL_VARBYTE    :
            case SQL_BYTES      :{type = SQL_C_BINARY    ;} break;
            // case SQL_BYTES      :{type = SQL_C_BYTES    ;} break;

            case SQL_NIBBLE     :{type = SQL_C_BINARY   ;} break;
            // case SQL_NIBBLE     :{type = SQL_C_NIBBLE   ;} break;

            case SQL_BINARY     :
            case SQL_BLOB       :
            case SQL_GEOMETRY   : type = SQL_C_BINARY   ; break;
            default://SQL_INTERVAL, NULL, etc.
                type = SQL_C_NATIVE;
        }
        return type;
    }
IDL_EXTERN_C_END
