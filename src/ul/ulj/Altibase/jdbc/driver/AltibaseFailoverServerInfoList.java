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

import java.util.ArrayList;

/**
 * {@link AltibaseFailoverServerInfo}를 위한 클래스.
 */
final class AltibaseFailoverServerInfoList extends ArrayList<AltibaseFailoverServerInfo>
{
    private static final long serialVersionUID = 8898419134711793282L;

    /**
     * alternate servers string 형태의 문자열로 변환한다.
     */
    @Override
    public String toString()
    {
        StringBuffer sBuf = new StringBuffer("(");
        for (int i = 0; i < this.size(); i++)
        {
            sBuf.append(get(i).toString());
            if (i > 0)
            {
                sBuf.append(',');
            }
        }
        sBuf.append(')');

        return sBuf.toString();
    }

    public boolean add(String aServer, int aPort, String aDbName)
    {
        return this.add(new AltibaseFailoverServerInfo(aServer, aPort, aDbName));
    }

    public void add(int aIndex, String aServer, int aPort, String aDbName)
    {
        this.add(aIndex, new AltibaseFailoverServerInfo(aServer, aPort, aDbName));
    }
}
