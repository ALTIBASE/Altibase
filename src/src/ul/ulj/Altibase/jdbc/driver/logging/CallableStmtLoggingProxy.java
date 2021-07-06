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

package Altibase.jdbc.driver.logging;

import java.sql.Connection;

/**
 * java.sql.CallableStatement 인터페이스를 hooking하는 Proxy클래스
 * 
 * @author yjpark
 *
 */
public class CallableStmtLoggingProxy extends PreparedStmtLoggingProxy
{
    public CallableStmtLoggingProxy(Object aTarget, Connection aConn)
    {
        super(aTarget, "CallableStatement", aConn);
    }
}
