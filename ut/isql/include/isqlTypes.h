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
 * $Id$
 **********************************************************************/

#ifndef _O_ISQLTYPES_H_
#define _O_ISQLTYPES_H_ 1

#include <idl.h>
#include <sqlcli.h>
#include <ulo.h>
#include <ute.h>
#include <iSQL.h>
#include <iSQLProperty.h>
#include <iSQLProgOption.h>

class isqlType
{
public:
    isqlType();
    ~isqlType();

    void    freeMem();
    void    SetName( SChar *aName );
    SChar  *GetName()                         { return mName; }
    void    SetSqlType( SInt aSqlType );
    SInt    GetSqlType()                      { return mSqlType; }
    void    SetCType( SShort aCType );
    SShort  GetCType()                        { return mCType; }
    void    SetPrecision( SInt aPrecision );

    SQLLEN *GetIndicator()                    { return &mInd; }
    SQLLEN  GetLen()                          { return mInd; }
    SInt    GetDisplaySize()                  { return mDisplaySize; }

    /* BUG-34447 COLUMN col FORMAT fmt */
    void    SetColumnFormat( SChar * aFmt, UChar * aToken );

    virtual IDE_RC  Init();
    virtual void    Reformat();
    virtual SInt    AppendToBuffer( SChar *aBuf, SInt *aBufLen );
    virtual SInt    AppendAllToBuffer( SChar *aBuf );

    virtual void   *GetBuffer()               { return mValue; }
    virtual SInt    GetBufferSize()           { return mValueSize; }

protected:
    virtual IDE_RC  initBuffer() = 0;
    virtual void    initDisplaySize() = 0;

protected:
    /* BUG-43911 Refactoring: Buffer for SQLBindCol */
    SChar *mValue;
    SQLLEN mInd;
    SInt   mValueSize;

    SChar  mName[UT_MAX_NAME_BUFFER_SIZE];
    SChar *mFmt;      /* BUG-34447 COLUMN col FORMAT fmt */
    UChar *mFmtToken; /* BUG-34447 COLUMN col FORMAT fmt */

    SInt   mSqlType;
    SShort mCType;
    SInt   mPrecision;

    SInt   mDisplaySize;
    SInt   mCurrLen;
    SChar *mCurr;
};

class isqlBit : public isqlType
{
public:
    isqlBit();

    void    Reformat();
    SInt    AppendToBuffer( SChar *aBuf, SInt *aBufLen );
    SInt    AppendAllToBuffer( SChar *aBuf );

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();

private:
    void ToChar( UChar  *aRaw,
                 SChar  *aCVal,
                 SQLLEN *aCValLen );
};

class isqlBlob : public isqlType
{
public:
    isqlBlob();

    void    Reformat();
    SInt    AppendToBuffer( SChar *aBuf, SInt *aBufLen );
    SInt    AppendAllToBuffer( SChar *aBuf );

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();
};

class isqlBytes : public isqlType
{
public:
    isqlBytes();

    void    Reformat();
    SInt    AppendToBuffer( SChar *aBuf, SInt *aBufLen );
    SInt    AppendAllToBuffer( SChar *aBuf );

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();
};

class isqlVarchar : public isqlType
{
public:
    isqlVarchar();

    SInt    AppendToBuffer( SChar *aBuf, SInt *aBufLen );
    SInt    AppendAllToBuffer( SChar *aBuf );

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();
};

class isqlChar : public isqlVarchar
{
public:
    void    Reformat();
};

class isqlClob : public isqlType
{
public:
    isqlClob();

    void    Reformat();

    void   *GetBuffer()               { return &mLobLocator; }
    SInt    GetBufferSize()           { return ID_SIZEOF(ULong); }

    idBool  IsNull();
    void    SetNull();
    void    SetLobValue();

    IDE_RC  InitLobBuffer( SInt aSize );
    ULong   GetLocator()             { return mLobLocator; }
    void   *GetLobBuffer()           { return mValue; }

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();

private:
    ULong   mLobLocator;
};

class isqlDate : public isqlType
{
public:
    isqlDate();

    IDE_RC  Init();

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();
};

class isqlDouble : public isqlType
{
public:
    isqlDouble();

    void    Reformat();

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();

private:
    SChar   mBuf[DOUBLE_DISPLAY_SIZE + 1];
};

class isqlInteger : public isqlType
{
public:
    isqlInteger();


protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();
};

class isqlLong : public isqlType
{
public:
    isqlLong();


protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();
};

class isqlNull : public isqlType
{
public:
    isqlNull();


protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();
};

class isqlNumeric : public isqlType
{
public:
    isqlNumeric();

    void    Reformat();
    SInt    AppendToBuffer( SChar *aBuf, SInt *aBufLen );

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();

private:
    void    ReformatNumber( SChar  *aCValue,
                            SQLLEN *aLen,
                            SInt    aWidth );
    SInt    GetExponent( SChar *aCValue );
};

class isqlReal : public isqlType
{
public:
    isqlReal();

    void    Reformat();

protected:
    IDE_RC  initBuffer();
    void    initDisplaySize();

private:
    SChar   mBuf[REAL_DISPLAY_SIZE + 1];
};

#endif /* _O_ISQLTYPES_H_ */
