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
 
package com.altibase.altimon.properties;

import com.altibase.altimon.util.StringUtil;

public enum DatabaseType {
    HDB {
        @Override public String getDriveClassName() {
            return "Altibase.jdbc.driver.AltibaseDriver";
        }

        @Override public String getDriveFileName() {
            return StringUtil.FILE_SEPARATOR + "lib" + StringUtil.FILE_SEPARATOR + "Altibase.jar";
        }

        @Override public String getHomeDir() {
            return System.getenv("ALTIBASE_HOME");
        }

        @Override public String getExecutionFilePath() {
            return new StringBuffer(StringUtil.FILE_SEPARATOR).append("bin").append(StringUtil.FILE_SEPARATOR).append("altibase").toString();
        }
    },
    XDB {
        @Override public String getDriveClassName() {
            return "Altibase_xdb.jdbc.driver.AltibaseDriver";
        }

        @Override public String getDriveFileName() {
            return StringUtil.FILE_SEPARATOR + "lib" + StringUtil.FILE_SEPARATOR + "xdbAltibase.jar";
        }

        @Override public String getHomeDir() {
            return System.getenv("ALTIBASE_XDB_HOME");
        }

        @Override public String getExecutionFilePath() {
            return new StringBuffer(StringUtil.FILE_SEPARATOR).append("bin").append(StringUtil.FILE_SEPARATOR).append("xdbaltibase").toString();
        }
    }/*,
	MYSQL {
		@Override public String getDriveClassName() {
			return "com.mysql.jdbc.Driver";
		}
	},
	Oracle {
		@Override public String getDriveClassName() {
			return "oracle.jdbc.driver.OracleDriver";
		}
	},
	MSSQL {
		@Override public String getDriveClassName() {
			return "com.microsoft.sqlserver.jdbc.SQLServerDriver";
		}
	},
	Derby {
		@Override public String getDriveClassName() {
			return "org.apache.derby.jdbc.ClientDriver";
		}
	}*/;

	public abstract String getDriveClassName();
	public abstract String getDriveFileName();
	public abstract String getHomeDir();
	public abstract String getExecutionFilePath();
}
