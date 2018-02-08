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
 
#include <idl.h>
#include <iduVersion.h>

int main ( void )
{

#ifdef ALTIADAPTER

    idlOS::printf ( "#define OA_VERSION "
                    "\"Adapter for ALTIBASE version %s\\n\"\\\n \"%s %s\"\n",
                    iduGetVersionString(),
                    iduGetSystemInfoString(),
                    iduGetProductionTimeString() );

#elif JDBCADAPTER

    idlOS::printf ( "#define OA_VERSION "
                    "\"Adapter for JDBC version %s\\n\"\\\n \"%s %s\"\n",
                    iduGetVersionString(),
                    iduGetSystemInfoString(),
                    iduGetProductionTimeString() );

#else

    idlOS::printf ( "#define OA_VERSION "
                    "\"ALTIBASE Adapter for Oracle version %s\\n\"\\\n \"%s %s\"\n",
                    iduGetVersionString(),
                    iduGetSystemInfoString(),
                    iduGetProductionTimeString() );

#endif
    return 0;
}

