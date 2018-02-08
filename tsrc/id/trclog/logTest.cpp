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
#include <idp.h>
#include <iduProperty.h>
#include <ide.h>
#include <ideMsgLog.h>
#include <idErrorCode.h>

IDE_RC ide__logBody (const SChar *format, va_list aList )
{
    (void)vfprintf (stderr, format, aList);
    return IDE_SUCCESS;
}


void ide__logMessage(ideLogModule aModule, UInt aLevel, const SChar *aFormat, va_list aList)
{
    
    ide__logBody(aFormat, aList);
}

IDE_RC ide__logInternal(ideLogModule aModule, UInt aLevel, const SChar *aFormat, va_list aList)
{
    ide__logMessage(aModule, aLevel, aFormat, aList);
    
    return IDE_SUCCESS;
    
}

inline IDE_RC ide__log(UInt aChkFlag, ideLogModule aModule, UInt aLevel, const SChar *aFormat, ... )
{
#if !defined(ITRON)
    va_list     ap;
    
    if (aChkFlag != 0)
    {
        va_start (ap, aFormat);
        ide__logInternal(aModule, aLevel, aFormat, ap);
    }
    va_end (ap);
    
#endif

    return IDE_SUCCESS;
}


int main()
{
    int i;
    
    ide__log(IDE_SERVER_0,
                "uint=%u\n "
                "ulong=%llu\n "
                "uint=%u\n "
                "uint=%llu\n ",
                (UInt)12, (ULong)32,
                (UInt)64, (ULong)128 );

    
    ideLog::initializeStaticBoot();
    
    idp::initialize();
    
    ideLog::initializeStaticModule();

    iduProperty::load();
    
    {// msb load
        SChar filename[512];
        idlOS::snprintf(filename, ID_SIZEOF(filename), "%s%cmsg%cE_%s_%s.msb",
                        idp::getHomeDir(),
                        IDL_FILE_SEPARATOR,
                        IDL_FILE_SEPARATOR,
                        "ID",
                        "US7ASCII");
        IDE_ASSERT(ideRegistErrorMsb(filename) == IDE_SUCCESS);
    }
    
    ideLog::log(IDE_SERVER_0, "single log test ");
    
    ideLog::log(IDE_SERVER_0,
                "uint=%u\n "
                "ulong=%llu\n "
                "uint=%u\n "
                "uint=%llu\n ",
                (UInt)12, (ULong)32,
                (UInt)64, (ULong)128 );
        
    {
        IDE_SET(ideSetErrorCode( idERR_ABORT_idaInvalidParameter ));
        
        ideLog::logErrorMsg(IDE_SERVER_0);
    }
    
    ideLog::logCallStack(IDE_SERVER_0);

    IDE_ASSERT(idp::update("SERVER_MSGLOG_FLAG", (UInt)1, 0, NULL)
               == IDE_SUCCESS);
        
    ideLog::logCallStack(IDE_SERVER_1);

    /* ------------------------------------------------
     *  SM
     * ----------------------------------------------*/

    ideLog::logCallStack(IDE_SM_0);

    IDE_ASSERT(idp::update("SM_MSGLOG_FLAG", (UInt)1, 0, NULL)
               == IDE_SUCCESS);
        
    ideLog::logCallStack(IDE_SM_1);


    if (IDE_TRC_SM_0)
    {
        ideLog::logOpen(IDE_SM_0);
        ideLog::logMessage(IDE_SM_0, "Multiple Message  Testing!!\n");
        ideLog::logMessage(IDE_SM_0, "Multiple Message  Testing!!\n");
        ideLog::logMessage(IDE_SM_0, "Multiple Message  Testing!!\n");
        ideLog::logClose(IDE_SM_0);
    }
    
    if (IDE_TRC_SM_1)
    {
        ideLog::logOpen(IDE_SM_1);
        ideLog::logMessage(IDE_SM_1, "Level 1 Logging Testing!!");
        ideLog::logClose(IDE_SM_1);
    }


    for (i = 0; i < 10; i++)
    {
        ideLog::log(IDE_SERVER_0, "test");
        idlOS::sleep(1);
    }

    return 0;
}
