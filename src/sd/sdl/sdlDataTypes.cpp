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
 

/***********************************************************************
 * $Id$ sdlDataTypes.c
 **********************************************************************/

#include <idl.h>
#include <mtdTypeDef.h>
#include <sdlDataTypes.h>

sdlDataTypeMap gSdlDataTypeTable[ULN_MTYPE_MAX] =
{
    { ULN_MTYPE_NULL,         MTD_NULL_ID,         SDL_TYPE( NULL )         }, //01
    { ULN_MTYPE_CHAR,         MTD_CHAR_ID,         SDL_TYPE( CHAR )         }, //02
    { ULN_MTYPE_VARCHAR,      MTD_VARCHAR_ID,      SDL_TYPE( VARCHAR )      }, //03
    { ULN_MTYPE_NUMBER,       MTD_NUMBER_ID,       SDL_TYPE( NUMBER )       }, //04
    { ULN_MTYPE_NUMERIC,      MTD_NUMERIC_ID,      SDL_TYPE( NUMERIC )      }, //05
    { ULN_MTYPE_BIT,          MTD_BIT_ID,          SDL_TYPE( BIT )          }, //06
    { ULN_MTYPE_SMALLINT,     MTD_SMALLINT_ID,     SDL_TYPE( SMALLINT )     }, //07
    { ULN_MTYPE_INTEGER,      MTD_INTEGER_ID,      SDL_TYPE( INTEGER )      }, //08
    { ULN_MTYPE_BIGINT,       MTD_BIGINT_ID,       SDL_TYPE( BIGINT )       }, //09
    { ULN_MTYPE_REAL,         MTD_REAL_ID,         SDL_TYPE( REAL )         }, //10
    { ULN_MTYPE_FLOAT,        MTD_FLOAT_ID,        SDL_TYPE( FLOAT )        }, //11
    { ULN_MTYPE_DOUBLE,       MTD_DOUBLE_ID,       SDL_TYPE( DOUBLE )       }, //12
    { ULN_MTYPE_BINARY,       MTD_BINARY_ID,       SDL_TYPE( BINARY )       }, //13
    { ULN_MTYPE_VARBIT,       MTD_VARBIT_ID,       SDL_TYPE( VARBIT )       }, //14
    { ULN_MTYPE_NIBBLE,       MTD_NIBBLE_ID,       SDL_TYPE( NIBBLE )       }, //15
    { ULN_MTYPE_BYTE,         MTD_BYTE_ID,         SDL_TYPE( BYTE )         }, //16
    { ULN_MTYPE_VARBYTE,      MTD_VARBYTE_ID,      SDL_TYPE( VARBYTE )      }, //17
    { ULN_MTYPE_TIMESTAMP,    MTD_DATE_ID,         SDL_TYPE( TIMESTAMP )    }, //18
    { ULN_MTYPE_DATE,         MTD_DATE_ID,         SDL_TYPE( DATE )         }, //19
    { ULN_MTYPE_TIME,         MTD_DATE_ID,         SDL_TYPE( TIME )         }, //20
    { ULN_MTYPE_INTERVAL,     MTD_INTERVAL_ID,     SDL_TYPE( INTERVAL )     }, //21
    { ULN_MTYPE_BLOB,         MTD_BLOB_LOCATOR_ID, SDL_TYPE( BLOB )         }, //22
    { ULN_MTYPE_CLOB,         MTD_CLOB_LOCATOR_ID, SDL_TYPE( CLOB )         }, //23
    { ULN_MTYPE_BLOB_LOCATOR, MTD_BLOB_LOCATOR_ID, SDL_TYPE( BLOB_LOCATOR ) }, //24
    { ULN_MTYPE_CLOB_LOCATOR, MTD_CLOB_LOCATOR_ID, SDL_TYPE( CLOB_LOCATOR ) }, //25
    { ULN_MTYPE_GEOMETRY,     MTD_GEOMETRY_ID,     SDL_TYPE( GEOMETRY )     }, //26
    { ULN_MTYPE_NCHAR,        MTD_NCHAR_ID,        SDL_TYPE( NCHAR )        }, //27
    { ULN_MTYPE_NVARCHAR,     MTD_NVARCHAR_ID,     SDL_TYPE( NVARCHAR )     }  //28
};

UShort mtdType_to_SDLMType(UInt aMTDType)
{
    UShort sCnt  = 0;
    UShort sType = SDL_MTYPE_MAX;

    for ( sCnt = 0 ; sCnt < ULN_MTYPE_MAX ; sCnt++ )
    {
        if ( gSdlDataTypeTable[sCnt].mMTD_Type == aMTDType )
        {
            sType = gSdlDataTypeTable[sCnt].mSDL_MTYPE;
            break;
        }
    }

    return sType;
}

UInt mtype_to_MTDType(UShort aMType)
{
    UInt sType = MTD_UNDEF_ID;
    if ( aMType < ULN_MTYPE_MAX )
    {
        sType = gSdlDataTypeTable[aMType].mMTD_Type;
    }
    else
    {
        /* Do Nothing. */
    }
    return sType;
}

UInt sdlMType_to_MTDType(UShort aSdlMType)
{
    UShort sUlnType = SDL_MTYPE_MAX;

    ULN_TYPE( sUlnType, aSdlMType );

    return mtype_to_MTDType( sUlnType );
}
