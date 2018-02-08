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
 * $Id $
 **********************************************************************/

#include <idl.h>
#include <iduMemMgr.h>
#include <ute.h>
#include <utpDef.h>
#include <utpCommandOptions.h>
#include <utpStatistics.h>
#include <utpDump.h>

uteErrorMgr     gErrorMgr;

// proj_2160 cm_type removal
const SChar *gUsageBasic =
"Usage: altiProfile [-h]\n"
/* Default of -len is changed to 0 from 160 in PROJ-2520.
"                   [-len length] FILE\n"
*/
"                   [-stat query/session] FILE1 [FILE2 [FILE3] ...]\n";

const SChar *gUsageTry =
"Try 'altiProfile -h' for more information\n";

const SChar *gUsageHelp =
"   FILE       : Original data file which you want to see as text   \n"
/* Default of -len is changed to 0 from 160 in PROJ-2520.
"   -len       : Specify the max length of each variable-length data\n"
"                to dump. Default value is 160 in bytes and \n"
"                if you want to dump it all, simply input 0.\n"
*/
"   -stat      : Build statistics by extracting data regarding SQL statements,\n"
"                and then print them in text and CSV file formats.\n";

int main(SInt argc, SChar **argv)
{
    IDE_TEST_RAISE(utpCommandOptions::parse(argc, argv) != IDE_SUCCESS,
                   err_invalid_option);

    switch (utpCommandOptions::mCommandType)
    {
    case UTP_CMD_HELP:
        idlOS::fprintf(stderr, "%s", gUsageBasic);
        idlOS::fprintf(stderr, "%s", gUsageHelp);
        break;

    case UTP_CMD_DUMP:
        IDE_TEST_RAISE(utpCommandOptions::mArgc < 1, err_invalid_option);
        IDE_TEST(utpDump::run(utpCommandOptions::mArgc,
                    &(argv[utpCommandOptions::mArgIdx]),
                    utpCommandOptions::mBindMaxLen)
                 != IDE_SUCCESS);
        break;

    case UTP_CMD_STAT_QUERY:
    case UTP_CMD_STAT_SESSION:
        IDE_TEST_RAISE(utpCommandOptions::mArgc < 1, err_invalid_option);
        IDE_TEST_RAISE(utpCommandOptions::mStatFormat == UTP_STAT_NONE,
                       err_invalid_option);

        IDE_TEST(utpStatistics::run(utpCommandOptions::mArgc,
                    &(argv[utpCommandOptions::mArgIdx]),
                    utpCommandOptions::mCommandType,
                    utpCommandOptions::mStatFormat)
                 != IDE_SUCCESS);
        break;

    default:
        IDE_RAISE(err_invalid_option);
        break;
    }

    return EXIT_SUCCESS;

    IDE_EXCEPTION(err_invalid_option);
    {
        /*
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Invalid_Option_Error, "");
        utePrintfErrorCode(stderr, &gErrorMgr);
        */

        idlOS::fprintf(stderr, "%s", gUsageBasic);
        idlOS::fprintf(stderr, "%s", gUsageTry);
    }
    IDE_EXCEPTION_END;

    return EXIT_FAILURE;
}
