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
 * $Id: utAtb.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utAtb.h>


    /* static connector allocator */
    Connection * _new_connection(dbDriver * dbd)
    {
        return new utAtbConnection(dbd);
    }

    SInt sqlTypeToAltibase(SInt type)
    {
        //ulnGetDefaultCTypdForSqlType을 그대로 복사해옴.
        switch(type)
        {
            case SQL_CHAR:
            case SQL_VARCHAR:
            case SQL_LONGVARCHAR:
            case SQL_DECIMAL:
            case SQL_NUMERIC:
            case SQL_FLOAT: //BUG-19366
            case SQL_CLOB:
                     type = SQL_C_CHAR;
                     break;
            case SQL_BIT:
                     type = SQL_C_BIT;
                     break;
            case SQL_TINYINT:
                     type = SQL_C_STINYINT;
                     break;
            case SQL_SMALLINT:
                     type = SQL_C_SSHORT;
                     break;
            case SQL_INTEGER:
                     type = SQL_C_SLONG;
                     break;
            case SQL_BIGINT:
                     type = SQL_C_SBIGINT;
                     break;
            case SQL_REAL:
                     type = SQL_C_FLOAT;
                     break;
            case SQL_DOUBLE:
                     type = SQL_C_DOUBLE;
                     break;
            case SQL_BINARY:
            case SQL_VARBINARY:
            case SQL_LONGVARBINARY:
            case SQL_BLOB:
                     type = SQL_C_BINARY;
                     break;
            case SQL_DATE:
            case SQL_TIME:
            case SQL_TIMESTAMP:
            case SQL_TYPE_TIMESTAMP:
                     type = SQL_C_TYPE_TIMESTAMP;
                     break;
            case SQL_INTERVAL_DAY:
            case SQL_INTERVAL_HOUR:
            case SQL_INTERVAL_MINUTE:
            case SQL_INTERVAL_SECOND:
            case SQL_INTERVAL_DAY_TO_HOUR:
            case SQL_INTERVAL_DAY_TO_MINUTE:
            case SQL_INTERVAL_DAY_TO_SECOND:
            case SQL_INTERVAL_HOUR_TO_MINUTE:
            case SQL_INTERVAL_HOUR_TO_SECOND:
            case SQL_INTERVAL_MINUTE_TO_SECOND:
                     type = SQL_C_CHAR;
                     break;
            case SQL_INTERVAL_YEAR:
            case SQL_INTERVAL_MONTH:
            case SQL_INTERVAL_YEAR_TO_MONTH:
                     type = SQL_C_CHAR;
                     break;
            case SQL_GUID:
                     type = SQL_C_GUID;
                     break;
            case SQL_GEOMETRY :
                     type = SQL_C_BINARY;
                     break;
            default : //SQL_INTERVAL, SQL_NULL, SQL_BYTES, SQL_NIBBLE, etc.
                     type = SQL_C_CHAR;
                     break;

           /*
            case SQL_CHAR       :
            case SQL_VARCHAR    :{type = SQL_C_CHAR   ;}break;
            case SQL_FLOAT      :{type = SQL_C_DOUBLE   ;} break;
            case SQL_NUMERIC    :
            case SQL_DECIMAL    :{type = SQL_C_CHAR     ;} break;
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

            case SQL_BYTES      :{type = SQL_C_BINARY    ;} break;
            // case SQL_BYTES      :{type = SQL_C_BYTES    ;} break;

            case SQL_NIBBLE     :{type = SQL_C_BINARY   ;} break;
            // case SQL_NIBBLE     :{type = SQL_C_NIBBLE   ;} break;

            case SQL_BINARY     :
            case SQL_BLOB       :
            case SQL_GEOMETRY   : type = SQL_C_BINARY   ; break;
            default://SQL_INTERVAL, NULL, etc.
                type = SQL_C_NATIVE;
          */
        }
        return type;
    }
