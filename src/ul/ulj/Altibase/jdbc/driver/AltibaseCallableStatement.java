/*
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

package Altibase.jdbc.driver;

import java.io.InputStream;
import java.io.Reader;
import java.math.BigDecimal;
import java.net.URL;
import java.sql.Array;
import java.sql.Blob;
import java.sql.CallableStatement;
import java.sql.Clob;
import java.sql.Date;
import java.sql.Ref;
import java.sql.SQLException;
import java.sql.Time;
import java.sql.Timestamp;
import java.sql.Types;
import java.util.Calendar;
import java.util.Map;

import Altibase.jdbc.driver.datatype.Column;
import Altibase.jdbc.driver.ex.Error;
import Altibase.jdbc.driver.ex.ErrorDef;

public class AltibaseCallableStatement extends AltibasePreparedStatement implements CallableStatement
{
    protected int mLastFetchedColumnIndex;

    AltibaseCallableStatement(AltibaseConnection aConnection, String aSql, int aResultSetType, int aResultSetConcurrency, int aResultSetHoldability) throws SQLException
    {
        super(aConnection, aSql, aResultSetType, aResultSetConcurrency, aResultSetHoldability);

        mLastFetchedColumnIndex = 0;
    }

    protected Column getColumnForOutType(int aIndex, int aSqlType) throws SQLException
    {
        throwErrorForClosed();

        // BUG-42424 deferred prepare
        if (mIsDeferred)
        {
            addMetaColumnInfo(aIndex, aSqlType, getDefaultPrecisionForDeferred(aSqlType, null));
        }

        Column sColumn = getColumn(aIndex, aSqlType);
        sColumn.getColumnInfo().addOutType();
        return sColumn;
    }

    private Column getOutBoundColumn(int aIndex) throws SQLException
    {
        throwErrorForClosed();
        if (aIndex <= 0 || aIndex > mBindColumns.size())
        {
            Error.throwSQLException(ErrorDef.INVALID_COLUMN_INDEX, "1 ~ " + mBindColumns.size(),
                                    String.valueOf(aIndex));
        }

        if (mBindColumns.get(aIndex - 1) == null)
        {
            Error.throwSQLException(ErrorDef.NOT_OUT_TYPE_PARAMETER, String.valueOf(aIndex));
        }
        if (!((Column)mBindColumns.get(aIndex - 1)).getColumnInfo().hasOutType())
        {
            Error.throwSQLException(ErrorDef.NOT_OUT_TYPE_PARAMETER, String.valueOf(aIndex));
        }

        mLastFetchedColumnIndex = aIndex;
        return (Column)mBindColumns.get(aIndex  -1);
    }

    private int getColumnIndex(String aName) throws SQLException
    {
        for (int i = 0; i < mBindColumns.size(); i++)
        {
            if (((Column)mBindColumns.get(i)).getColumnInfo().getColumnName().equalsIgnoreCase(aName))
            {
                return i;
            }
        }
        Error.throwSQLException(ErrorDef.INVALID_COLUMN_NAME, aName);
        return -1;
    }

    public Array getArray(int aIndex) throws SQLException
    {
        Error.throwSQLException(ErrorDef.UNSUPPORTED_FEATURE, "Array type");
        return null;
    }

    public Array getArray(String aParamName) throws SQLException
    {
        return getArray(getColumnIndex(aParamName));
    }

    public BigDecimal getBigDecimal(int aIndex, int aScale) throws SQLException
    {
        BigDecimal sResult = getBigDecimal(aIndex);
        return sResult.setScale(aScale, BigDecimal.ROUND_HALF_EVEN);        
    }

    public BigDecimal getBigDecimal(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getBigDecimal();
    }

    public BigDecimal getBigDecimal(String aParamName) throws SQLException
    {
        return getBigDecimal(getColumnIndex(aParamName));
    }

    public Blob getBlob(int aIndex) throws SQLException
    {
        AltibaseBlob sBlob = (AltibaseBlob)getOutBoundColumn(aIndex).getBlob();
        if (sBlob != null)
        {
            sBlob.open(mConnection.channel());
        }
        return sBlob;
    }

    public Blob getBlob(String aParamName) throws SQLException
    {
        return getBlob(getColumnIndex(aParamName));
    }

    public boolean getBoolean(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getBoolean();
    }

    public boolean getBoolean(String aParamName) throws SQLException
    {
        return getBoolean(getColumnIndex(aParamName));
    }

    public byte getByte(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getByte();
    }

    public byte getByte(String aParamName) throws SQLException
    {
        return getByte(getColumnIndex(aParamName));
    }

    public byte[] getBytes(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getBytes();
    }

    public byte[] getBytes(String aParamName) throws SQLException
    {
        return getBytes(getColumnIndex(aParamName));
    }

    public Clob getClob(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getClob();
    }

    public Clob getClob(String aParamName) throws SQLException
    {
        return getClob(getColumnIndex(aParamName));
    }

    public Date getDate(int aIndex, Calendar aCalendar) throws SQLException
    {
        return getOutBoundColumn(aIndex).getDate(aCalendar);
    }

    public Date getDate(int aIndex) throws SQLException
    {
        return getDate(aIndex, null);
    }

    public Date getDate(String aParamName, Calendar aCalendar) throws SQLException
    {
        return getDate(getColumnIndex(aParamName), aCalendar);
    }

    public Date getDate(String aParamName) throws SQLException
    {
        return getDate(getColumnIndex(aParamName));
    }

    public double getDouble(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getDouble();
    }

    public double getDouble(String aParamName) throws SQLException
    {
        return getDouble(getColumnIndex(aParamName));
    }

    public float getFloat(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getFloat();
    }

    public float getFloat(String aParamName) throws SQLException
    {
        return getFloat(getColumnIndex(aParamName));
    }

    public int getInt(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getInt();
    }

    public int getInt(String aParamName) throws SQLException
    {
        return getInt(getColumnIndex(aParamName));
    }

    public long getLong(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getLong();
    }

    public long getLong(String aParamName) throws SQLException
    {
        return getLong(getColumnIndex(aParamName));
    }

    public Object getObject(int aIndex, Map aMap) throws SQLException
    {
        Error.throwSQLException(ErrorDef.UNSUPPORTED_FEATURE, "User defined type");
        return null;
    }

    public Object getObject(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getObject();
    }

    public Object getObject(String aParamName, Map aMap) throws SQLException
    {
        return getObject(getColumnIndex(aParamName), aMap);
    }

    public Object getObject(String aParamName) throws SQLException
    {
        return getObject(getColumnIndex(aParamName));
    }

    public Ref getRef(int aIndex) throws SQLException
    {
        Error.throwSQLException(ErrorDef.UNSUPPORTED_FEATURE, "Ref type");
        return null;
    }

    public Ref getRef(String aParamName) throws SQLException
    {
        return getRef(getColumnIndex(aParamName));
    }

    public short getShort(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getShort();
    }

    public short getShort(String aParamName) throws SQLException
    {
        return getShort(getColumnIndex(aParamName));
    }

    public String getString(int aIndex) throws SQLException
    {
        return getOutBoundColumn(aIndex).getString();
    }

    public String getString(String aParamName) throws SQLException
    {
        return getString(getColumnIndex(aParamName));
    }

    public Time getTime(int aIndex, Calendar aCalendar) throws SQLException
    {
        return getOutBoundColumn(aIndex).getTime(aCalendar);
    }

    public Time getTime(int aIndex) throws SQLException
    {
        return getTime(aIndex, null);
    }

    public Time getTime(String aParamName, Calendar aCalendar) throws SQLException
    {
        return getTime(getColumnIndex(aParamName), aCalendar);
    }

    public Time getTime(String aParamName) throws SQLException
    {
        return getTime(getColumnIndex(aParamName));
    }

    public Timestamp getTimestamp(int aIndex, Calendar aCalendar) throws SQLException
    {
        return getOutBoundColumn(aIndex).getTimestamp(aCalendar);
    }

    public Timestamp getTimestamp(int aIndex) throws SQLException
    {
        return getTimestamp(aIndex, null);
    }

    public Timestamp getTimestamp(String aParamName, Calendar aCalendar) throws SQLException
    {
        return getTimestamp(getColumnIndex(aParamName), aCalendar);
    }

    public Timestamp getTimestamp(String aParamName) throws SQLException
    {
        return getTimestamp(getColumnIndex(aParamName));
    }

    public URL getURL(int aIndex) throws SQLException
    {
        Error.throwSQLException(ErrorDef.UNSUPPORTED_FEATURE, "URL type");
        return null;
    }

    public URL getURL(String aParamName) throws SQLException
    {
        return getURL(getColumnIndex(aParamName));
    }

    public void registerOutParameter(int aIndex, int aSqlType, int aScale) throws SQLException
    {
        registerOutParameter(aIndex, aSqlType);

        if (aSqlType == Types.NUMERIC || aSqlType == Types.DECIMAL)
        {
            ((Column)mBindColumns.get(aIndex - 1)).getColumnInfo().modifyScale(aScale);
        }
    }

    public void registerOutParameter(int aIndex, int aSqlType, String aTypeName) throws SQLException
    {
        // REF, STRUCT, DISTINCT를 지원하지 않기 때문에 aTypeName을 무시한다.
        registerOutParameter(aIndex, aSqlType);
    }

    public void registerOutParameter(int aIndex, int aSqlType) throws SQLException
    {
        getColumnForOutType(aIndex, aSqlType);
    }

    public void registerOutParameter(String aParamName, int aSqlType, int aScale) throws SQLException
    {
        registerOutParameter(getColumnIndex(aParamName), aSqlType, aScale);
    }

    public void registerOutParameter(String aParamName, int aSqlType, String aTypeName) throws SQLException
    {
        registerOutParameter(getColumnIndex(aParamName), aSqlType, aTypeName);
    }

    public void registerOutParameter(String aParamName, int aSqlType) throws SQLException
    {
        registerOutParameter(getColumnIndex(aParamName), aSqlType);
    }

    public void setAsciiStream(String aParamName, InputStream aValue, int aLength) throws SQLException
    {
        setAsciiStream(getColumnIndex(aParamName), aValue, aLength);
    }

    public void setBigDecimal(String aParamName, BigDecimal aValue) throws SQLException
    {
        setBigDecimal(getColumnIndex(aParamName), aValue);
    }

    public void setBinaryStream(String aParamName, InputStream aValue, int aLength) throws SQLException
    {
        setBinaryStream(getColumnIndex(aParamName), aValue, aLength);
    }

    public void setBoolean(String aParamName, boolean aValue) throws SQLException
    {
        setBoolean(getColumnIndex(aParamName), aValue);
    }

    public void setByte(String aParamName, byte aValue) throws SQLException
    {
        setByte(getColumnIndex(aParamName), aValue);
    }

    public void setBytes(String aParamName, byte[] aValue) throws SQLException
    {
        setBytes(getColumnIndex(aParamName), aValue);
    }

    public void setCharacterStream(String aParamName, Reader aValue, int aLength) throws SQLException
    {
        setCharacterStream(getColumnIndex(aParamName), aValue, aLength);
    }

    public void setDate(String aParamName, Date aValue, Calendar aCalendar) throws SQLException
    {
        setDate(getColumnIndex(aParamName), aValue, aCalendar);
    }

    public void setDate(String aParamName, Date aValue) throws SQLException
    {
        setDate(getColumnIndex(aParamName), aValue);
    }

    public void setDouble(String aParamName, double aValue) throws SQLException
    {
        setDouble(getColumnIndex(aParamName), aValue);
    }

    public void setFloat(String aParamName, float aValue) throws SQLException
    {
        setFloat(getColumnIndex(aParamName), aValue);
    }

    public void setInt(String aParamName, int aValue) throws SQLException
    {
        setInt(getColumnIndex(aParamName), aValue);
    }

    public void setLong(String aParamName, long aValue) throws SQLException
    {
        setLong(getColumnIndex(aParamName), aValue);
    }

    public void setNull(String aParamName, int aSqlType, String aTypeName) throws SQLException
    {
        setNull(getColumnIndex(aParamName), aSqlType, aTypeName);
    }

    public void setNull(String aParamName, int aSqlType) throws SQLException
    {
        setNull(getColumnIndex(aParamName), aSqlType);
    }

    public void setObject(String aParamName, Object aValue, int aSqlType, int aScale) throws SQLException
    {
        setObject(getColumnIndex(aParamName), aValue, aSqlType, aScale);
    }

    public void setObject(String aParamName, Object aValue, int aSqlType) throws SQLException
    {
        setObject(getColumnIndex(aParamName), aValue, aSqlType);
    }

    public void setObject(String aParamName, Object aValue) throws SQLException
    {
        setObject(getColumnIndex(aParamName), aValue);
    }

    public void setShort(String aParamName, short aValue) throws SQLException
    {
        setShort(getColumnIndex(aParamName), aValue);
    }

    public void setString(String aParamName, String aValue) throws SQLException
    {
        setString(getColumnIndex(aParamName), aValue);
    }

    public void setTime(String aParamName, Time aValue, Calendar aCalendar) throws SQLException
    {
        setTime(getColumnIndex(aParamName), aValue, aCalendar);
    }

    public void setTime(String aParamName, Time aValue) throws SQLException
    {
        setTime(getColumnIndex(aParamName), aValue);
    }

    public void setTimestamp(String aParamName, Timestamp aValue, Calendar aCalendar) throws SQLException
    {
        setTimestamp(getColumnIndex(aParamName), aValue, aCalendar);
    }

    public void setTimestamp(String aParamName, Timestamp aValue) throws SQLException
    {
        setTimestamp(getColumnIndex(aParamName), aValue);
    }

    public void setURL(String aParamName, URL aValue) throws SQLException
    {
        setURL(getColumnIndex(aParamName), aValue);
    }

    public boolean wasNull() throws SQLException
    {
        throwErrorForClosed();
        if (mLastFetchedColumnIndex == 0)
        {
            Error.throwSQLException(ErrorDef.WAS_NULL_CALLED_BEFORE_CALLING_GETXXX);
        }
        return ((Column)mBindColumns.get(mLastFetchedColumnIndex - 1)).isNull();
    }
}
