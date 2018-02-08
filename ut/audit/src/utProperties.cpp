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
 
/*******************************************************************************
 * $Id: utProperties.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utProperties.h>
#include <iduVersion.h>

IDL_EXTERN_C SChar* str_case_str(const SChar*, const SChar*);
bool mAtcIsFirst;
bool utProperties::mVerbose = false;

static const SChar *HelpMessage =
"===================================================================== \n"
"                     altiComp HELP Screen                             \n"
"===================================================================== \n"
"  Usage   : altiComp [-h]\n"
"                     [-v] [-V] [-z]\n"
"                     [-f configuration_file_path]\n"
"                     [-m master_database_connection_string]\n"
"                     [-s slave_database_connection_string]\n"
"                     [-x commit_count]\n"
"                     [-t max_thread_count]\n"
"                     [-d log_directory_path]\n"
"                     [sync|diff]\n"
"            -h  : Show this help screen\n"
"            -v  : Verbosely list records affected during sync operation\n"
"            -V  : Print program version\n"
"            -f  : Specify configuration file path\n"
"            -m  : Specify connection string for master database (DB_MASTER)\n"
"            -s  : Specify connection string for slave database (DB_SLAVE)\n"
"            -x  : Specify the number of records to commit at once (COUNT_TO_COMMIT)\n"
"            -t  : Specify the maximum number of threads to be used (MAX_THREAD)\n"
"            -d  : Specify directory path for log files (LOG_DIR)\n"
"            sync|diff  : Specify altiComp operation (OPERATION)\n"
"===================================================================== \n"
;

inline const SChar * strnull(const SChar * s)
{
    if(s == NULL)
    {
        return "\'\'";
    }
    return s;
}

IDE_RC utProperties::initialize(int argc, char **argv)
{
#define IDE_FN "iIDE_RC utProperties::initialize(int argc, char **argv)"
    SChar * sProgram = _basename(argv[0]);
    SChar   tmp[512] = {'\0'};
    SInt    opr,i;

    SChar * sMaURL = NULL;
    SChar * sSlURL = NULL;

    IDE_TEST(idlOS::mutex_init(&mLock,USYNC_THREAD) != 0);

    flog           = stderr;
    logFName       = NULL;
    log_level      =    0;
    logDir         = NULL;
    mDSNMaster = mDSNSlave = NULL;
    mTimeInterval  = -1;
    mCountToCommit = -1;

    for(i = 0; i < DML_MAX; i++)
    {
        mDML[i]= true;
    }

    mSize          =    0;
    mTab           = NULL;
    memp           = NULL;
    mMode          = DUMMY;
    mMaxThread =    -1; // undef, unlimited
    mVerbose   = false; // write report for SYNC
    mAtcIsFirst = true;

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    mMaxArrayFetch =  0;

    while((opr = idlOS::getopt(argc, argv, "t:hvVf:d:?:m:s:x:")) != EOF)
    {
        switch(opr)
        {
            case 'd': // work logdir
                IDE_TEST(setProperty(&logDir ,
                                     "LOG_DIR",
                                     "LOG_DIR",
                                     optarg)
                         != IDE_SUCCESS);
                break;
            case 'v': // verbose execution
                mVerbose = true;
                break;
            case 'V': // Version show
                /* 
                 * BUG-32566
                 *
                 * iloader와 같이 Version 출력되도록 수정
                 */
                printVersion();
                exit(0);
            case 't': // tread count
                IDE_TEST(setProperty(&mMaxThread,
                                     "MAX_THREAD",
                                     "MAX_THREAD",
                                     optarg)
                         != IDE_SUCCESS);
                break;
            case 'f': // config file/script
                idlOS::strncpy(tmp, optarg, sizeof(tmp));
                break;
            case 'h':
            case '?':
                printUsage();
                exit(0);
            case 'm':
                sMaURL = optarg;
                break;
            case 's':
                sSlURL = optarg;
                break;
            case 'x':
                IDE_TEST(setProperty(&mCountToCommit,
                                     "COUNT_TO_COMMIT",
                                     "COUNT_TO_COMMIT",
                                     optarg)
                         != IDE_SUCCESS);
                break;
            default:
                printUsage();
                IDE_RAISE(err_param);
        }//endswitch
    }//endwhile

    for(i = optind; i<argc; i++)
    {
        if(idlOS::strcasecmp(argv[i],"SYNC") == 0)
        {
            mMode = SYNC;
            /* default value */
            mDML[SU] = true;
            mDML[SI] = true;
            mDML[MI] = true;
            mDML[SD] = false;
        }
        else if(idlOS::strcasecmp(argv[i],"DIFF") == 0)
        {
            mMode = DIFF;

            mDML[SU] = true;
            mDML[SI] = true;
            mDML[MI] = true;
            mDML[SD] = false;
        }
    }
    // ** default config file ** //
    if(*tmp == '\0')
    {
        idlOS::strncpy(tmp, sProgram, sizeof(tmp));
        idlOS::strcat(tmp, ".cfg");
    }

    IDE_TEST(prepare(sProgram,tmp) != IDE_SUCCESS);

    /* overwrite master URL */
    if(sMaURL != NULL)
    {
        mDSNMaster = sMaURL;
    }

    /* overwrite Slave URL */
    if(sSlURL != NULL)
    {
        mDSNSlave = sSlURL;
    }


    if(logDir != NULL)
    {
        idlOS::strncpy(tmp,logDir, sizeof(tmp));
    }
    else
    {
        SChar *s,*p = NULL;

        idlOS::strncpy(tmp,argv[0],sizeof(tmp) );

        for(s=tmp;*s; s++)
            if(*s=='/')p=s;
        IDE_TEST(p == NULL);
        *p = '\0';
    }

    idlOS::strcat(tmp,"/");

    if(logFName == NULL)
    {
        idlOS::strcat (tmp,sProgram);
        idlOS::strcat (tmp,".log");
    }
    else
    {
        idlOS::strcat (tmp,logFName);
    }

    flog = idlOS::fopen(tmp,"w+");

    IDE_TEST_RAISE(flog == NULL, error_file);

    IDE_TEST(checkProperty(mDSNMaster,"DB_MASTER") != IDE_SUCCESS);
    IDE_TEST(checkProperty(mDSNSlave ,"DB_SLAVE") != IDE_SUCCESS);

    IDE_TEST_RAISE(mMasterDB.initialize(mDSNMaster) != IDE_SUCCESS, err_url);
    IDE_TEST_RAISE(mSlave_DB.initialize(mDSNSlave) != IDE_SUCCESS, err_url);

    return IDE_SUCCESS;
    IDE_EXCEPTION(error_file);
    {
        flog = stderr;
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Check_Log_File_Error,
                        strerror(errno));
        log(uteGetErrorMSG(&gErrorMgr));
    }
    IDE_EXCEPTION(err_url);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Wrong_URL_Error);
        log(uteGetErrorMSG(&gErrorMgr));
    }
    IDE_EXCEPTION(err_param)
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Wrong_Parameter_Error);
        log(uteGetErrorMSG(&gErrorMgr));
    }

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
#undef IDE_FN
}

utProperties::utProperties()
{
}

IDE_RC utProperties::checkProperty(const SChar * prop, const SChar * key)
{
    IDE_TEST(prop == NULL);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Not_Found_Parameter_Error,
                    key);
    log(uteGetErrorMSG(&gErrorMgr));

    return IDE_FAILURE;
}

utProperties::~utProperties()
{
    // fix BUG-11842
    // finalize();
}

IDE_RC utProperties::setProperty(SInt  * aProperty,
                                 const SChar * aKey1,
                                 const SChar * aKey2,
                                 SChar * aValue)
{
    IDE_TEST(aKey1 == NULL || aKey2 == NULL);

    if(idlOS::strCaselessMatch(aKey1, idlOS::strlen(aKey1),
                               aKey2, idlOS::strlen(aKey2)) == 0)
    {
        *aProperty = idlOS::strtol(aValue, &aValue,10);
        switch(errno){
            case ERANGE:
                IDE_RAISE(err_range); break;
            case EINVAL:
                IDE_RAISE(err_inval); break;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_range);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_ERANGE_Error, aKey2);
        log(uteGetErrorMSG(&gErrorMgr));
    }
    IDE_EXCEPTION(err_inval);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_EINVAL_Error, aKey2);
        log(uteGetErrorMSG(&gErrorMgr));
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC utProperties::setProperty(bool * aProperty,
                                 const SChar * aKey1,
                                 const SChar * aKey2,
                                 SChar * aValue)
{
    IDE_TEST(aKey1 == NULL || aKey2 == NULL);

    if(idlOS::strCaselessMatch(aKey1, idlOS::strlen(aKey1),
                               aKey2, idlOS::strlen(aKey2)) == 0)
    {
        idlOS::strUpper(aValue, idlOS::strlen(aValue));
        *aProperty = (idlOS::strcmp(aValue,"ON") == 0);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utProperties::setProperty (pmode_t * aProperty,
                                  const SChar * aKey1,
                                  const SChar * aKey2,
                                  SChar * aValue)
{
    if(*aProperty != DUMMY)
        return IDE_SUCCESS ; //it is alrady set

    IDE_TEST(aKey1 == NULL || aKey2 == NULL);

    if(idlOS::strCaselessMatch(aKey1, idlOS::strlen(aKey1),
                               aKey2, idlOS::strlen(aKey2)) == 0)
    {
        if(*aProperty == DUMMY)
            idlOS::strUpper(aValue, idlOS::strlen(aValue));
        if(idlOS::strcasecmp(aValue, "DIFF") == 0)
        {
            *aProperty = DIFF;
        }
        else if(idlOS::strcasecmp(aValue, "SYNC") == 0)
        {
            *aProperty = SYNC;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utProperties::setProperty(SChar      ** aProperty,
                                 const SChar * aKey1,
                                 const SChar * aKey2,
                                 SChar       * aValue,
                                 bool          aUpperCase)
{
    IDE_TEST(aKey1 == NULL || aKey2 == NULL);

    if(idlOS::strCaselessMatch(aKey1, idlOS::strlen(aKey1),
                               aKey2, idlOS::strlen(aKey2)) == 0)
    {
        IDE_TEST( *aProperty != NULL );

        if(aUpperCase)
        {
            idlOS::strUpper(aValue, idlOS::strlen(aValue));
        }
        *aProperty = aValue;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Invalid_Parameter_Error, aKey2);
    log(uteGetErrorMSG(&gErrorMgr));

    return IDE_FAILURE;
}

IDE_RC utProperties::tokenize(SChar** aKey, SChar** aValue, SChar * &aString)
{
    SChar bb = '\0', // bracket char begin
          be = '\0'; // bracket char end
    SChar *s = skips(aString);

    if(*s != '\0')
    {
        *aKey = s;
        while(ident(*s, ID_FALSE))
        {
            s++;
        }
        if(*s != '=')
        {
            *s='\0';
            s= skips(++s);
        }
        if(*s == '=')
        {
            *s='\0';
            s= skips(++s);
        }

        switch(*s)
        {
            case '\"':
                if((idlOS::strcasecmp(*aKey, "TABLE")      == 0) ||
                   (idlOS::strcasecmp(*aKey, "SCHEMA")     == 0))
                {
                    // BUG-17517
                    bb =     be = '\0'; break;
                }
                else
                {
                    be = '\"'; break;
                }
            case '(':
                    bb = '('; be = ')'; break;
            case '[':
                bb = '['; be = ']'; break;
            case '{':
                bb = '{'; be = '}'; break;
            default:
                bb =     be = '\0'; break;
        }
        if(be)
        {
            UShort bcount =  1;  // bracket count
            s++;
            *aValue = s;

            for(bcount=1; bcount;)
            {
                ++s;
                IDE_TEST_RAISE(*s == '\0', err_prop);
                if(*s==be)
                    --bcount;
                if(*s==bb)
                    ++bcount;
            }

            *s='\0';
            s=skips(++s);
            IDE_TEST_RAISE(*s != '\0', err_prop);
        }
        else
        {
            *aValue = s;
            while(*s != '\0' && *s != '\n')
            {
                s++;
            }
            *s = '\0';
        }

        idlOS::strUpper(*aKey, idlOS::strlen(*aKey));
        if((idlOS::strcmp(*aKey, "DB_MASTER") != 0) &&
           (idlOS::strcmp(*aKey, "DB_SLAVE")  != 0) &&
           (idlOS::strcmp(*aKey, "TABLE")     != 0) &&
           (idlOS::strcmp(*aKey, "SCHEMA")    != 0) &&
           (idlOS::strcmp(*aKey, "WHERE")     != 0) &&
           (idlOS::strcmp(*aKey, "EXCLUDE")   != 0) &&
           (idlOS::strcmp(*aKey, "LOG_DIR")   != 0) &&
           (idlOS::strcmp(*aKey, "LOG_FILE")  != 0))
        {
            idlOS::strUpper(*aValue, idlOS::strlen(*aValue));
        }
    }
    else
    {
        *aKey   = NULL;
        *aValue = NULL;
    }
    //  aString = s;

    return IDE_SUCCESS;

    IDE_EXCEPTION( err_prop )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Invalid_Parameter_Error,
                        aString);
        log(uteGetErrorMSG(&gErrorMgr));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

utTableProp* utProperties::nextTabProp(SChar* aStr)
{
    utTableProp * sTab;
    bool          sSpecialCharacter = ID_FALSE;

    for(;(*aStr == ' ' || *aStr == '[' || *aStr == '\t'); aStr++);

    sTab = (utTableProp*)idlOS::calloc(1,sizeof(utTableProp));
    IDE_TEST(sTab == NULL);

    sTab->master = aStr;

    //BUG-17517
    while(*aStr != '\0')
    {
        if((*aStr == '\"') && (sSpecialCharacter == ID_FALSE))
        {
            sSpecialCharacter = ID_TRUE;
            aStr ++;
        }
        else if((*aStr == '\"') && (sSpecialCharacter == ID_TRUE))
        {
            sSpecialCharacter = ID_FALSE;
            aStr ++;
            break;
        }
        if(!ident(*aStr, sSpecialCharacter))
        {
            break;
        }
        aStr ++;
    };
    *aStr = '\0';

    sTab->next = mTab;
    mTab = sTab;

    ++mSize;
    sTab->mTabNo = mSize;

    return mTab;

    IDE_EXCEPTION_END;

    return NULL;
}

IDE_RC utProperties::finalize()
{
    utTableProp *i;
    lock();

    for(i = mTab; mTab; i = mTab)
    {
        mTab = i->next;
        idlOS::free(i);
    }
    mTab = NULL;

    // BUG-40205 insure++ warning
    mMasterDB.finalize();
    mSlave_DB.finalize();

    if (memp != NULL)
    {
        idlOS::free(memp);
        memp = NULL;
    }
    unlock();
    idlOS::mutex_destroy(&mLock);

    return IDE_SUCCESS;
}

IDE_RC utProperties::getTabProp(utTableProp** aTab)
{
    (void)  lock();

    // BUG-40205 insure++ warning
    if (*aTab != NULL)
    {
        idlOS::free(*aTab);
    }

    *aTab = mTab;

    if(mTab)
    {
        mTab = mTab->next;
    }
    --mSize;
    (void)unlock();

    return IDE_SUCCESS;
}

IDE_RC utProperties::prepare(const char*,  const char* aConfiguration)
{
    FILE  * sFile = NULL;
    SChar * sKey;
    SChar * sVal;
    SChar *s, *i;
    SInt sLeftSz, sStrLen;
    UInt memsz;
    utTableProp * prop;

    prop = NULL;
    sFile = idlOS::fopen(aConfiguration, "r");

    IDE_TEST_RAISE(sFile == NULL, error_file);

#ifndef VC_WIN32
    memsz = idlOS::filesize(fileno(sFile));
#else
    memsz = idlOS::filesize(aConfiguration);
#endif
    IDE_TEST_RAISE(memsz <= 0, error_file);

    memp = (SChar*)idlOS::malloc(memsz+1);
    IDE_TEST_RAISE(memp == NULL, error_malloc);

    *memp = '\0';
    s = memp;

    for(sLeftSz = memsz; fgets(s, sLeftSz, sFile) != NULL; s += sStrLen+1)
    {
        sStrLen =_tail(s);

        for(i = s; (i != NULL) && (i[sStrLen] == '\\'); sStrLen = _tail(i))
        {
            i[sStrLen] = '\0';
            sLeftSz -= ++sStrLen;
            i += sStrLen+1;
            i = fgets(i, sLeftSz , sFile);
            i = skips(i);
            _strcat(s, i);
        }

        removeComment(s);
        s = skips(s);

        sStrLen = idlOS::strlen( s );
        if(sStrLen == 0) continue;

        sLeftSz -= sStrLen;

        // Replication Name & new structure
        if(*s == '[')
        {
            prop = nextTabProp(s);
            continue;
        }

        tokenize(&sKey, &sVal, s);

        if(sKey == NULL || sVal == NULL)
        {
            s   -=  sStrLen;
            sLeftSz += sStrLen;
            continue;
        }

        if(!mSize)
        {
            setProperty(&logFName, "LOG_FILE", sKey, sVal);
            setProperty(&log_level, "LOG_LEVEL", sKey, sVal);
            setProperty(&logDir, "LOG_DIR", sKey, sVal);
            setProperty(&mDSNMaster, "DB_MASTER", sKey, sVal);
            setProperty(&mDSNSlave, "DB_SLAVE", sKey, sVal);
            setProperty(&mMode, "OPERATION", sKey, sVal);
            setProperty(&mMaxThread, "MAX_THREAD", sKey, sVal);
            setProperty(&mDML[SI], "INSERT_TO_SLAVE" , sKey, sVal);
            setProperty(&mDML[MI], "INSERT_TO_MASTER", sKey, sVal);
            setProperty(&mDML[SD], "DELETE_IN_SLAVE" , sKey, sVal);
            setProperty(&mDML[SU], "UPDATE_TO_SLAVE" , sKey, sVal);
            setProperty(&mTimeInterval, "CHECK_INTERVAL", sKey, sVal);
            setProperty(&mCountToCommit, "COUNT_TO_COMMIT", sKey, sVal);
            /* TASK-4212: audit툴의 대용량 처리시 개선 */
            setProperty(&mMaxArrayFetch, "FILE_MODE_MAX_ARRAY", sKey, sVal);
        }
        else
        {
            setProperty(&prop->slave  ,"TABLE", sKey, sVal);
            setProperty(&prop->where  ,"WHERE", sKey, sVal);
            setProperty(&prop->schema ,"SCHEMA", sKey, sVal);
            setProperty(&prop->exclude,"EXCLUDE", sKey, sVal);
        }//endif
    }//endfor

    idlOS::fclose(sFile);
    sFile = NULL;

    /* check mode of execution */
    switch(mMode)
    {
        case SYNC:  /* disabled states check */
//          IDE_TEST_RAISE( mDML[SD] && mDML[SU], ERR_SD_SU);
            IDE_TEST_RAISE( mDML[SD] && mDML[MI], ERR_SD_MI);   // BUG-18697
            break;
        case DIFF:
            break;
        default: ;
    };

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_SD_MI);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Invalid_Parameter_Error,
                "SD and MI Incompatible");
        log(uteGetErrorMSG(&gErrorMgr));
    }
    IDE_EXCEPTION(error_file);
    {
        log("ERROR[ PROP ] %s\n", strerror(errno));
    }
    IDE_EXCEPTION(error_malloc);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Alloc_Memory_Error);
        log(uteGetErrorMSG(&gErrorMgr));
    }

    IDE_EXCEPTION_END;

    // BUG-25228 [CodeSonar] utProperties::prepare() 함수에서 에러 발생시 파일 닫지 않음.
    if(sFile != NULL)
    {
        idlOS::fclose(sFile);
    }

    return IDE_FAILURE;
}

void utProperties::printUsage()
{
    idlOS::fprintf(stderr, HelpMessage);
}

/* 
 * BUG-32566
 *
 * iloader와 같이 Version 출력되도록 수정
 */
void utProperties::printVersion()
{
    idlOS::fprintf(stdout,
                   "-----------------------------------------------------------------\n"
                   "     Altibase Data Diff/Sync utility.\n"
                   "     Release Version %s\n"
                   "     Copyright 2000, ALTIBASE Corporation or its subsidiaries.\n"
                   "     All Rights Reserved.\n"
                   "-----------------------------------------------------------------\n",
                   iduVersionString);

    return;
}

IDE_RC utProperties::log(const SChar* format, ...)
{
    va_list args;

    (void)lock();
    va_start(args, format);

    IDE_TEST(vfprintf(flog,format, args) < 0);

    va_end(args);
    (void)unlock();

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    (void)unlock();

    return IDE_FAILURE;
}


inline const SChar * is(bool v)
{
    if(v)
    {
        return "ON";
    }
    else
    {
        return "OFF";
    }
};

void utProperties::printConfig(FILE * conf)
{
    idlOS::fprintf( conf,
                    "\n##################################################\n"
                    "###    Audit script file                          \n"
                    "###    (c) Copyright 2000, ALTIBASE Corporation   \n"
                    "##################################################\n\n"
                    "DB_MASTER      = \"%s\"\n"
                    "DB_SLAVE       = \"%s\"\n"
                    "\nOPERATION = "
                    ,mDSNMaster
                    ,mDSNSlave);

    switch(mMode)
    {
        case SYNC:
            idlOS::fprintf(conf, "SYNC\n"); break;
        case DIFF:
            idlOS::fprintf(conf, "DIFF\n"); break;
        case DUMMY:
            idlOS::fprintf(conf,"DUMMY\n"); break;
    }

    idlOS::fprintf(conf,"MAX_THREAD = %d\n",mMaxThread);
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    idlOS::fprintf(conf,"FILE_MODE_MAX_ARRAY = %d\n",mMaxArrayFetch);

    idlOS::fprintf(conf,
                   "\nINSERT_TO_MASTER    = %3s\n"
                   "INSERT_TO_SLAVE     = %3s\n"
                   "DELETE_IN_SLAVE     = %3s\n"
                   "UPDATE_TO_SLAVE     = %3s\n",
                   is(mDML[MI]),
                   is(mDML[SI]),
                   is(mDML[SD]),
                   is(mDML[SU]));
    idlOS::fprintf(conf,
                   "\n##################################################\n");

    for(utTableProp * i = mTab; i; i = i->next)
    {
        printTab(conf, i);
    }
    idlOS::fprintf(conf, "\n\n");
    idlOS::fflush(conf);
};

void utProperties::printTab(FILE *f, utTableProp *p)
{
    if(f == NULL)
    {
        f = stdout;
    }

    if(p != NULL)
    {  idlOS::fprintf(f,        "[%s]\n"
                      " WHERE   = %s\n"
                      " EXCLUDE = %s\n"
                      " TABLE   = %s\n"
                      " SCHEMA  = %s\n",
                      strnull(p->master),
                      strnull(p->where),
                      strnull(p->exclude),
                      strnull(p->slave),
                      strnull(p->schema)
                      );
    }
};
