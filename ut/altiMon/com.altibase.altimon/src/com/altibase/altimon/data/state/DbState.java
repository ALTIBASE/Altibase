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
 
package com.altibase.altimon.data.state;

import java.util.List;

import com.altibase.altimon.connection.Session;

public interface DbState {
    public static final int IDX_VALUE = 1;
    public static final int IDX_TIME = 2;

    public static final int IDX_ALARM_NAME = 1;
    public static final int IDX_ALARM_VALUE = 2;
    public static final int IDX_ALARM_TIME = 3;

    public void insertData(List aData);
    public boolean isConnected();
    public void setSession(Session session);
    public void insertAltimonId();
    public String getMetaId();
    public boolean hasMetaId();
}
