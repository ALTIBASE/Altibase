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

/*
 * NAME: utmScript4Ilo.cpp
 * 
 * DESCRIPTION: BUG-44234 Code Refactoring
 *   iloader 실행 스크립트 출력을 위한 함수 모음
 */

#include <utm.h>
#include <utmExtern.h>
#include <utmScript4Ilo.h>

/* BUG-34502: handling quoted identifiers */
const SChar *gUserOptionFormatStr[] = {
    " -u %s",
    " -u "UTM_BEGIN_QUOT"%s"UTM_END_QUOT
};
const SChar *gPwdOptionFormatStr[] = {
    " -p %s",
    " -p "UTM_BEGIN_QUOT"%s"UTM_END_QUOT
};

static const SChar *gTableOptionFormatStr[] = {
    " -T %s",
    " -T "UTM_BEGIN_QUOT"%s"UTM_END_QUOT
};

static iloOption gIloOptions[4] = {
    { "-f", "fmt" },
    { "-d", "dat" },
    { "-log", "log" },
    { "-bad", "bad" }
};

void printFormOutScript( FILE  *aIlOutFp,
                         SInt   aNeedQuote4User,
                         SInt   aNeedQuote4Pwd,
                         SInt   aNeedQuote4Tbl,
                         SInt   aNeedQuote4File,
                         SChar *aUser,
                         SChar *aPasswd,
                         SChar *aTable,
                         idBool aIsPartOpt )
{
    /* formout in run_il_out.sh */
    idlOS::fprintf(aIlOutFp, ILO_PRODUCT_NAME" %s", gProgOption.mOutConnectStr);
    idlOS::fprintf(aIlOutFp, gUserOptionFormatStr[aNeedQuote4User], aUser);
    idlOS::fprintf(aIlOutFp, gPwdOptionFormatStr[aNeedQuote4Pwd], aPasswd);
    idlOS::fprintf(aIlOutFp, " formout"); 

    /* BUG-44243 Code refactoring of iloader option printing.
     * formout 커맨드에서는 파일 이름에 parallel number가 필요하지 않음 */
    printOptionWithFile( aIlOutFp, aNeedQuote4File, ILO_FMT,
                         aUser, aTable, NULL, ILO_NO_PARALLEL );

    /* -T table_name */
    idlOS::fprintf(aIlOutFp, gTableOptionFormatStr[aNeedQuote4Tbl], aTable);

    if( aIsPartOpt == ID_TRUE )
    {
        idlOS::fprintf(aIlOutFp, " -PARTITION");
    }
    if ( gProgOption.mbExistNLS == ID_TRUE )
    {
        idlOS::fprintf(aIlOutFp, " -NLS_USE %s", gProgOption.mNLS);
    }
    idlOS::fprintf(aIlOutFp, "\n");
}

void printOutScript( FILE  *aIlOutFp,
                     SInt   aNeedQuote4User,
                     SInt   aNeedQuote4Pwd,
                     SInt   aNeedQuote4File,
                     SChar *aUser,
                     SChar *aPasswd,
                     SChar *aTable,
                     SChar *aPartName )
{
    /* data out in run_il_out.sh*/
    idlOS::fprintf(aIlOutFp, ILO_PRODUCT_NAME" %s", gProgOption.mOutConnectStr);
    idlOS::fprintf(aIlOutFp, gUserOptionFormatStr[aNeedQuote4User], aUser);
    idlOS::fprintf(aIlOutFp, gPwdOptionFormatStr[aNeedQuote4Pwd], aPasswd);
    idlOS::fprintf(aIlOutFp, " %s", UTM_STR_OUT); 

    /* BUG-44243 Code refactoring of iloader option printing.
     * out 커맨드에서는 파일 이름에 parallel number가 필요하지 않음 */
    printOptionWithFile( aIlOutFp, aNeedQuote4File, ILO_FMT,
                         aUser, aTable, aPartName, ILO_NO_PARALLEL );
    printOptionWithFile( aIlOutFp, aNeedQuote4File, ILO_DAT,
                         aUser, aTable, aPartName, ILO_NO_PARALLEL );

    /* iloader option (data out) in run_il_out.sh */
    if ( gProgOption.mbExistIloFTerm == ID_TRUE )
    {
        idlOS::fprintf(aIlOutFp, " -t \"%s\"", gProgOption.mIloFTerm);
    }
    if ( gProgOption.mbExistIloRTerm == ID_TRUE )
    {
        idlOS::fprintf(aIlOutFp, " -r \"%s\"", gProgOption.mIloRTerm);
    }

    /* BUG-43571 Support -parallel, -commit and -array options of iLoader */
    if ( gProgOption.mbExistIloParallel == ID_TRUE )
    {
        idlOS::fprintf(aIlOutFp, " -parallel %"ID_INT32_FMT,
                       gProgOption.mIloParallel);
    }
    if ( gProgOption.mbExistIloArray == ID_TRUE )
    {
        idlOS::fprintf(aIlOutFp, " -array %"ID_INT32_FMT,
                       gProgOption.mIloArray);
    }
    if ( gProgOption.mbExistIloAsyncPrefetch == ID_TRUE )
    {
        idlOS::fprintf(aIlOutFp, " -async_prefetch %s",
                       gProgOption.mAsyncPrefetchType);
    }

    if (gProgOption.IsLog() == ID_TRUE)
    {
        printOptionWithFile( aIlOutFp, aNeedQuote4File, ILO_LOG,
                             aUser, aTable, aPartName, ILO_NO_PARALLEL );
    }
    else
    {
        /* Do nothing */
    }

    // BUG-40271 Replace the default character set from predefined value (US7ASCII) to DB character set.
    if ( gProgOption.mbExistNLS == ID_TRUE )
    {
        idlOS::fprintf(aIlOutFp, " -NLS_USE %s", gProgOption.mNLS);
    }
    else
    {
        /* Do nothing */
    }

    idlOS::fprintf(aIlOutFp, "\n");
}

void printInScript( FILE  *aIlInFp,
                    SInt   aNeedQuote4User,
                    SInt   aNeedQuote4Pwd,
                    SInt   aNeedQuote4File,
                    SChar *aUser,
                    SChar *aPasswd,
                    SChar *aTable,
                    SChar *aPartName )
{
    SInt i;
    SInt sParallelBase = 0;
    SInt sParallelCnt = (gProgOption.mbExistIloParallel == ID_TRUE)?
                        gProgOption.mIloParallel : 1;

    /* BUG-44243 Code refactoring of iloader option printing */
    if (sParallelCnt > 1)
    {
        sParallelBase = 0;
    }
    else
    {
        sParallelBase = ILO_NO_PARALLEL;
    }
    for (i=0; i<sParallelCnt; i++)
    {
        idlOS::fprintf(aIlInFp, ILO_PRODUCT_NAME" %s", gProgOption.mInConnectStr);
        idlOS::fprintf(aIlInFp, gUserOptionFormatStr[aNeedQuote4User], aUser);
        idlOS::fprintf(aIlInFp, gPwdOptionFormatStr[aNeedQuote4Pwd], aPasswd);
        idlOS::fprintf(aIlInFp, " %s", UTM_STR_IN); 

        /* BUG-44243 Code refactoring of iloader option printing.
         * in 커맨드에서는 파일 이름에 parallel number가 필요하지만, 
         * -f 옵션에서는 필요없음 */
        printOptionWithFile( aIlInFp, aNeedQuote4File, ILO_FMT,
                             aUser, aTable, aPartName, ILO_NO_PARALLEL );
        printOptionWithFile( aIlInFp, aNeedQuote4File, ILO_DAT,
                             aUser, aTable, aPartName, sParallelBase + i );

        /* iloader option (data in) in run_il_in.sh */
        if ( gProgOption.mbExistIloFTerm == ID_TRUE )
        {
            idlOS::fprintf(aIlInFp, " -t \"%s\"", gProgOption.mIloFTerm);
        }
        if ( gProgOption.mbExistIloRTerm == ID_TRUE )
        {
            idlOS::fprintf(aIlInFp, " -r \"%s\"", gProgOption.mIloRTerm);
        }

        /* BUG-40470 Support -errors option of iLoader */
        if ( gProgOption.mbExistIloErrCnt == ID_TRUE )
        {
            idlOS::fprintf(aIlInFp, " -errors %"ID_INT32_FMT,
                           gProgOption.mIloErrCnt);
        }

        /* BUG-43571 Support -parallel, -commit and -array options of iLoader */
        if ( gProgOption.mbExistIloParallel == ID_TRUE )
        {
            idlOS::fprintf(aIlInFp, " -parallel %"ID_INT32_FMT,
                           gProgOption.mIloParallel);
        }
        if ( gProgOption.mbExistIloCommit == ID_TRUE )
        {
            idlOS::fprintf(aIlInFp, " -commit %"ID_INT32_FMT,
                           gProgOption.mIloCommit);
        }
        if ( gProgOption.mbExistIloArray == ID_TRUE )
        {
            idlOS::fprintf(aIlInFp, " -array %"ID_INT32_FMT,
                           gProgOption.mIloArray);
        }

        if (gProgOption.IsLog() == ID_TRUE)
        {
            printOptionWithFile( aIlInFp, aNeedQuote4File, ILO_LOG,
                                 aUser, aTable, aPartName, sParallelBase + i );
        }
        else
        {
            /* Do nothing */
        }

        if (gProgOption.IsBad() == ID_TRUE)
        {
            printOptionWithFile( aIlInFp, aNeedQuote4File, ILO_BAD,
                                 aUser, aTable, aPartName, sParallelBase + i );
        }
        else
        {
            /* Do nothing */
        }

        idlOS::fprintf(aIlInFp, "\n");
    }
}

/*
 * BUG-44234 Code Refactoring
 *   iloader 실행 스크립트에 파일 이름과 함께 출력되는 옵션을 위한 함수
     ex) -d user_table.dat
         -d user_table.dat0
         -d user_table.dat.part
         -d user_table.dat.part0
 */
void printOptionWithFile( FILE          *aFp,
                          SInt           aNeedQuote,
                          IloOptionType  aOption,
                          SChar         *aUser,
                          SChar         *aTable,
                          SChar         *aPartition,
                          SInt           aParallel )
{
    /* print option. ex) -d, -log, ... */
    idlOS::fprintf( aFp, " %s ", gIloOptions[aOption].mOption );

    /* print quotation mark */
    if ( aNeedQuote == ID_TRUE )
    {
        idlOS::fprintf( aFp, UTM_BEGIN_QUOT );
    }
    else
    {
        /* Do nothing */
    }

    /* print user_table.ext ex) user_table.dat, user_table.log, ... */
    idlOS::fprintf( aFp, "%s_%s.%s", aUser, aTable, gIloOptions[aOption].mFileExt );

    /* print partition name. ex) .part */
    if ( aPartition != NULL )
    {
        idlOS::fprintf( aFp, ".%s", aPartition );
    }
    else
    {
        /* Do nothing */
    }

    /* print parallel number. ex) 0, 1, ... */
    if ( aParallel != ILO_NO_PARALLEL )
    {
        idlOS::fprintf( aFp, "%"ID_INT32_FMT, aParallel );
    }
    else
    {
        /* Do nothing */
    }

    /* print quotation mark */
    if ( aNeedQuote == ID_TRUE )
    {
        idlOS::fprintf( aFp, UTM_END_QUOT );
    }
    else
    {
        /* Do nothing */
    }
}

