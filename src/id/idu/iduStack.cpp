/** 
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
 
/*****************************************************************************
 * $Id: iduStack_server.cpp 27347 2008-08-01 00:03:25Z orc $
 ****************************************************************************/

#include <idl.h>
#include <idp.h>
#include <ide.h>
#include <iduStack.h>
#include <iduProperty.h>
#include <iduVersion.h>
#include <idtContainer.h>

#if defined(ALTI_CFG_OS_WINDOWS)

IDL_EXTERN_C void WINAPI RtlCaptureContext(CONTEXT*);
iduSymInit                          iduStack::mSymInitFunc      = NULL;
iduStackWalker                      iduStack::mWalkerFunc       = NULL;
PFUNCTION_TABLE_ACCESS_ROUTINE64    iduStack::mAccessFunc       = NULL;
PGET_MODULE_BASE_ROUTINE64          iduStack::mGetModuleFunc    = NULL;
HMODULE                             iduStack::mHModule          = NULL;

#else

IDTHREAD SChar                      iduStack::mSigAltStack[SIGSTKSZ];

# if defined(ALTI_CFG_OS_HPUX)
IDL_EXTERN_C void U_STACK_TRACE(void);
# endif

#endif

PDL_HANDLE      iduStack::mFD;
const SChar     iduStack::mDigitChars[17] = "0123456789ABCDEF";
SInt            iduStack::mCallStackCriticalSection;
IDTHREAD SInt   iduStack::mCallDepth;
IDTHREAD void*  iduStack::mCallStack[IDU_MAX_CALL_DEPTH];

SChar gSigIllMsgs[][64] = 
{
    "Illegal opcode",
    "Illegal operand",
    "Illegal addressing mode",
    "Illegal trap",
    "Privileged opcode",
    "Privileged register",
    "Co-processor error",
    "Internal stack error"
};

SChar gSigFpeMsgs[][64] = 
{
    "Integer divide by zero",
    "Integer overflow",
    "Floating point divide by zero",
    "Floating point overflow",
    "Floating point underflow",
    "Floating point inexact result",
    "Invalid floating point operation",
    "Subscript out of range",
};

SChar gSigSegvMsgs[][64] = 
{
    "Address not mapped to object",
    "Invalid permissions"
};

SChar gSigBusMsgs[][128] =
{
    "Invalid address alignment",
    "Non-existent physical address",
    "Object specific hardware error",
    "Hardware memory error consumed on a machine check; action required.",
    "Hardware  memory  error detected in process but not consumed; action optional"
};


void iduStack::setBucketList(ideLogModule       aModule,
                             const SChar*       aFormat,
                             ...)
{
    idtContainer*   sContainer = idtContainer::getThreadContainer();
    va_list         ap;

    va_start(ap, aFormat);
    sContainer->mBucketMsgLength[aModule] = 
        vsnprintf(sContainer->mBucketMsg[aModule],
                  IDT_MAX_MSG_SIZE,
                  aFormat, ap);
    va_end(ap);
}

void iduStack::clearBucketList(ideLogModule aModule)
{
    idtContainer* sContainer = idtContainer::getThreadContainer();
    sContainer->mBucketMsgLength[aModule] = 0;
}

void iduStack::writeBucket()
{
    SInt            i;
    size_t          sLen;
    idtContainer*   sContainer = idtContainer::getThreadContainer();

    for(i = IDE_ERR; i < IDE_LOG_MAX_MODULE; i++)
    {
        if(sContainer->mBucketMsgLength[i] != 0)
        {
            sLen = idlOS::strlen(ideLog::mMsgModuleName[i]);
            writeString("===== ", 6);
            writeString(ideLog::mMsgModuleName[i], sLen);
            writeString(" =====\n", 7);
            writeString(sContainer->mBucketMsg[i],
                        sContainer->mBucketMsgLength[i]);
        }
        else
        {
            /* continue */
        }
    }
}

void iduStack::writeDec32(UInt aValue)
{
    SInt    i;
    SInt    sDigit;
    idBool  sWritten = ID_FALSE;

    for(i = 1000000000; i >= 10; i /= 10)
    {
        sDigit = aValue / i;
        if(sDigit == 0)
        {
            if(sWritten == ID_TRUE)
            {
                idlOS::write(mFD, mDigitChars + sDigit, 1);
            }
            else
            {
                /* continue */
            }
        }
        else
        {
            idlOS::write(mFD, mDigitChars + sDigit, 1);
            sWritten = ID_TRUE;
        }

        aValue -= sDigit * i;
    }

    idlOS::write(mFD, mDigitChars + aValue, 1);
}

void iduStack::writeHex32(UInt aValue)
{
    idlOS::write(mFD, mDigitChars + ((aValue >> 28) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 24) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 20) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 16) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 12) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >>  8) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >>  4) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue      ) % 16), 1);
}

void iduStack::writeHex64(ULong aValue)
{
    idlOS::write(mFD, mDigitChars + ((aValue >> 60) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 56) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 52) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 48) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 44) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 40) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 36) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 32) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 28) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 24) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 20) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 16) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >> 12) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >>  8) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue >>  4) % 16), 1);
    idlOS::write(mFD, mDigitChars + ((aValue      ) % 16), 1);
}

void iduStack::writeHexPtr(void* aValue)
{
#if defined(COMPILE_64BIT)
    writeHex64((ULong)aValue);
#else
    writeHex32((UInt)aValue);
#endif
}

void iduStack::writeString(const void* aStr, size_t aLen)
{
    idlOS::write(mFD, aStr, aLen);
}

void iduStack::convertHex64(ULong aValue, SChar* aStr, SChar** aEnd)
{
    UInt i;
    SInt j;

    for(i = 0, j = 60; i < sizeof(ULong) * 2; i++, j -= 4)
    {
        *aStr = mDigitChars[(aValue >> j) % 16]; aStr++;
    }

    *aEnd = aStr;
}

#if defined(ALTI_CFG_OS_WINDOWS)

IDE_RC iduStack::setSigStack(void)
{
    return IDE_SUCCESS;
}

IDE_RC iduStack::initializeStatic(void)
{
    // Dynamically load the Entry-Points for dbghelp.dll:
    // First try to load the newsest one from
    TCHAR szTemp[4096];
    // But before wqe do this, we first check if the ".local" file exists
    if (GetModuleFileName(NULL, szTemp, 4096) > 0)
    {
        if (GetEnvironmentVariable(("ProgramFiles"), szTemp, 4096) > 0)
        {
            idlOS::strcat(szTemp, "\\Debugging Tools for Windows\\dbghelp.dll");
            // now check if the file exists:
            if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
            {
                mHModule = LoadLibrary(szTemp);
            }
        }
        // Still not found? Then try to load the 64-Bit version:
        if ( (mHModule == NULL) && (GetEnvironmentVariable("ProgramFiles", szTemp, 4096) > 0) )
        {
            idlOS::strcat(szTemp, "\\Debugging Tools for Windows 64-Bit\\dbghelp.dll");
            if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
            {
                mHModule = LoadLibrary(szTemp);
            }
        }
    }
    if (mHModule == NULL)  // if not already loaded, try to load a default-one
    {
        mHModule = LoadLibrary( "dbghelp.dll");
    }

    IDE_TEST(mHModule == NULL);
    mSymInitFunc    = (iduSymInit)::GetProcAddress(mHModule, "SymInitialize");
    mWalkerFunc     = (iduStackWalker)::GetProcAddress(mHModule, "StackWalk64");
    mAccessFunc     = 
        (PFUNCTION_TABLE_ACCESS_ROUTINE64)
        ::GetProcAddress(mHModule, "SymFunctionTableAccess64");
    mGetModuleFunc  = 
        (PGET_MODULE_BASE_ROUTINE64)
        ::GetProcAddress(mHModule, "SymGetModuleBase64");

    IDE_TEST_RAISE(mSymInitFunc     == NULL, EFUNCNOTFOUND);
    IDE_TEST_RAISE(mWalkerFunc      == NULL, EFUNCNOTFOUND);
    IDE_TEST_RAISE(mAccessFunc      == NULL, EFUNCNOTFOUND);
    IDE_TEST_RAISE(mGetModuleFunc   == NULL, EFUNCNOTFOUND);

    IDE_TEST_RAISE(mSymInitFunc(GetCurrentProcess(), NULL, TRUE) == FALSE, EFUNCNOTFOUND);

    return IDE_SUCCESS;

    IDE_EXCEPTION(EFUNCNOTFOUND)
    {
        FreeLibrary(mHModule);
    }

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

void iduStack::dumpStack( const CONTEXT    *aContext,
                          SChar            *aSqlString )
{
    DWORD           sImageType;
    CONTEXT         sContext;
    STACKFRAME64    sFrame;
    time_t          sTime;
    SChar*          sTemp;
    SChar           sLine[512] = "[0x";

    IDE_TEST(mHModule       == NULL);
    IDE_TEST(mWalkerFunc    == NULL);
    IDE_TEST(mAccessFunc    == NULL);
    IDE_TEST(mGetModuleFunc == NULL);

    mFD = ideLog::getErrFD();

    IDE_TEST( mFD == PDL_INVALID_HANDLE );

    sTime = idlOS::time(NULL);

    convertHex64(sTime, sLine + 3, &sTemp);
    *sTemp = ' '; sTemp++;
    convertHex64(ideLogEntry::getLogSerial(), sTemp, &sTemp);
    idlOS::strcpy(sTemp, "] Dump of Stack\n");
    sTemp += 16;
        
    writeString(sLine, sTemp - sLine);

    if(aContext == NULL)
    {
        idlOS::memset(&sContext, 0, sizeof(sContext));
        sContext.ContextFlags = CONTEXT_FULL;
        RtlCaptureContext(&sContext);
        
        writeString("BEGIN-STACK [NORMAL] ============================\n", 50);
    }
    else
    {
        idlOS::memcpy(&sContext, aContext, sizeof(CONTEXT));

        writeString("BEGIN-DUMP ======================================\n", 50);
        writeBucket();
        if( aSqlString != NULL ) 
        {
            writeString( "SQL : ", 6 );
            writeString( aSqlString, idlOS::strlen(aSqlString) );
            writeString( "\n", 1 );
        }
        else
        {
            /* Do nothing */
        }
        writeString("END-DUMP ========================================\n", 50);

        writeString("BEGIN-STACK [CRASH] =============================\n", 50);
    }

# if defined(_M_IX86)
    sImageType = IMAGE_FILE_MACHINE_I386;
    sFrame.AddrPC.Offset = sContext.Eip;
    sFrame.AddrPC.Mode = AddrModeFlat;
    sFrame.AddrFrame.Offset = sContext.Ebp;
    sFrame.AddrFrame.Mode = AddrModeFlat;
    sFrame.AddrStack.Offset = sContext.Esp;
    sFrame.AddrStack.Mode = AddrModeFlat;
# elif defined(_M_AMD64)
    sImageType = IMAGE_FILE_MACHINE_AMD64;
    sFrame.AddrPC.Offset = sContext.Rip;
    sFrame.AddrPC.Mode = AddrModeFlat;
    sFrame.AddrFrame.Offset = sContext.Rbp;
    sFrame.AddrFrame.Mode = AddrModeFlat;
    sFrame.AddrStack.Offset = sContext.Rsp;
    sFrame.AddrStack.Mode = AddrModeFlat;
# else
#  error Unsupported platform!
# endif

    mCallDepth = 0;

    mCallStack[mCallDepth] = (void*)sFrame.AddrPC.Offset;

    writeString("Caller[", 7);
    writeDec32(mCallDepth);
    writeString("] ", 2);
    writeHexPtr(mCallStack[mCallDepth]);
    writeString("\n", 1);

    mCallDepth++;

    while(sFrame.AddrPC.Offset != NULL)
    {
        if(mWalkerFunc(sImageType, GetCurrentProcess(), GetCurrentThread(),
                    &sFrame, &sContext, NULL,
                    mAccessFunc, mGetModuleFunc,
                    NULL) == FALSE)
        {
            break;
        }

        mCallStack[mCallDepth] = (void*)sFrame.AddrPC.Offset;

        writeString("Caller[", 7);
        writeDec32(mCallDepth);
        writeString("] ", 2);
        writeHexPtr(mCallStack[mCallDepth]);
        writeString("\n", 1);

        mCallDepth++;
    }

    writeString( "END-STACK =======================================\n", 50 );

    return;

    IDE_EXCEPTION_END;

    if ( mFD != PDL_INVALID_HANDLE )
    {
        writeString( "ERROR : Could not get context ===================\n", 50 );
    }
    else
    {
        /* do nothing */
    }

    return;
}

#else

IDE_RC iduStack::setSigStack(void)
{
#if !defined(ALTIBASE_MEMORY_CHECK)
    stack_t     sNewStack;

    sNewStack.ss_sp     = mSigAltStack;
    sNewStack.ss_flags  = 0;
    sNewStack.ss_size   = SIGSTKSZ;

    IDE_TEST(idlOS::sigaltstack(&sNewStack, NULL) != 0);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
#else
    return IDE_SUCCESS;
#endif
}

# if defined(ALTI_CFG_OS_LINUX) && !defined(ALTI_CFG_CPU_POWERPC)

/*
 * Definitions and datatypes from glibc
 */

#define DWARF_FRAME_REGISTERS 17

struct dwarf_eh_bases
{
  void *tbase;
  void *dbase;
  void *func;
};

typedef struct iduUnwindContext
{
  void*                 reg[DWARF_FRAME_REGISTERS+1];
  void*                 cfa;
  void*                 ra;
  void*                 lsda;
  struct dwarf_eh_bases bases;
  unsigned int          args_size;
} iduUnwindContext;

typedef struct iduTraceArg
{
  void**    mAddr;
  int       mCnt;
  int       mSize;
  void*     mLastEBP;
  void*     mLastESP;
} iduTraceArg;

typedef enum
{
  _URC_NO_REASON = 0,
  _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
  _URC_FATAL_PHASE2_ERROR = 2,
  _URC_FATAL_PHASE1_ERROR = 3,
  _URC_NORMAL_STOP = 4,
  _URC_END_OF_STACK = 5,
  _URC_HANDLER_FOUND = 6,
  _URC_INSTALL_CONTEXT = 7,
  _URC_CONTINUE_UNWIND = 8
} iduUnwindResult;

typedef iduUnwindResult     iduBacktraceFunc(void*, iduTraceArg*);
typedef void*               iduGetIPFunc(iduUnwindContext*);

static iduBacktraceFunc*    gUnwindBacktraceFunc;
static iduGetIPFunc*        gUnwindGetIPFunc;
static void*                gLibGccHandle;

static iduUnwindResult iduTraceStack(iduUnwindContext*  aContext,
                                     iduTraceArg*       aArg)
{
    /* Skip first call from tracestack */
    if ( aArg->mCnt != -1 )
    {
        aArg->mAddr[aArg->mCnt] = (void*)gUnwindGetIPFunc(aContext);
    }
    else
    {
        /* fall through */
    }

    aArg->mCnt++;
    if ( aArg->mCnt == aArg->mSize )
    {
        return _URC_END_OF_STACK;
    }
    else
    {
        aArg->mLastEBP = (void*)aContext->reg[5];
        aArg->mLastESP = (void*)aContext->cfa;
        return _URC_NO_REASON;
    }
}

SInt iduStack::tracestack(void** aAddr, SInt aSize)
{
    iduTraceArg sArg;
    sArg.mAddr = aAddr;
    sArg.mSize = aSize;
    sArg.mCnt = -1;

    IDE_TEST( aSize <= 0 );

    gUnwindBacktraceFunc((void*)iduTraceStack, &sArg);

    if (
         (sArg.mCnt > 1) &&
         (sArg.mAddr[sArg.mCnt - 1] == NULL)
       )
    {
        sArg.mCnt--;
    }
    IDE_TEST( sArg.mCnt == -1 );

    return sArg.mCnt;

    IDE_EXCEPTION_END;
    return 0;
}

# endif

IDE_RC iduStack::initializeStatic(void)
{
    /* BUG-44297 */
    mCallStackCriticalSection = 0;

#if defined(ALTI_CFG_OS_LINUX) && !defined(ALTI_CFG_CPU_POWERPC)
    gLibGccHandle = idlOS::dlopen ("libgcc_s.so.1", RTLD_NOW);

    IDE_TEST(gLibGccHandle == NULL);

    gUnwindBacktraceFunc    = (iduBacktraceFunc*)idlOS::dlsym(
            gLibGccHandle,
            "_Unwind_Backtrace");
    gUnwindGetIPFunc        = (iduGetIPFunc*)idlOS::dlsym(
            gLibGccHandle,
            "_Unwind_GetIP");

    IDE_TEST_RAISE(gUnwindBacktraceFunc == NULL,    ENOSYMBOL);
    IDE_TEST_RAISE(gUnwindGetIPFunc     == NULL,    ENOSYMBOL);

    return setSigStack();

    IDE_EXCEPTION(ENOSYMBOL)
    {
        gUnwindGetIPFunc        = NULL;
        gUnwindBacktraceFunc    = NULL;
        idlOS::dlclose(gLibGccHandle);
    }

    IDE_EXCEPTION_END;
    gLibGccHandle = NULL;
    ideLog::logMessage(1, IDE_SERVER, 0,
            "Warning!!! Failed to get unwind functions.\n"
            "Callstack will not be logged on CRASH!\n");
    return setSigStack();
#else
    return setSigStack();
#endif
}

# if defined(ALTI_CFG_OS_SOLARIS)
int iduStack::walker(uintptr_t ptr, int, void*)
{
    mCallStack[mCallDepth] = (void*)ptr;
    mCallDepth++;
    return 0;
}
# endif

void iduStack::dumpStack( const iduSignalDef    *aSignal,
                          siginfo_t             *aSigInfo,
                          ucontext_t            *aContext,
                          idBool                 aIsFaultTolerate, /* PROJ-2617 */
                          SChar                 *aSqlString )
{
#if defined(COMPILE_64BIT)
    SInt        i;
    SInt        sLen;
    SChar*      sTemp;
    SChar       sLine[512] = "[0x";
        
    void*       sIP;
    void**      sSP;

    ucontext_t* sContext;
    ucontext_t  sRealContext;

    time_t      sTime;

    mFD = ideLog::getErrFD();
    sTime = idlOS::time(NULL);

    convertHex64(sTime, sLine + 3, &sTemp);
    *sTemp = ' '; sTemp++;
    convertHex64(ideLogEntry::getLogSerial(), sTemp, &sTemp);
    idlOS::strcpy(sTemp, "] Dump of Stack\n");
    sTemp += 16;
        
    /*
     * BUG-44297
     * Critical section for logging call stack
     */
    while(acpAtomicCas32(&mCallStackCriticalSection, 1, 0) == 1)
    {
        idlOS::thr_yield();
    }

    writeString(sLine, sTemp - sLine);

    if(aSignal == NULL)
    {
        IDE_TEST(getcontext(&sRealContext) != 0);
        sContext = &sRealContext;
        writeString("BEGIN-STACK [NORMAL] ============================\n", 50);
    }
    else
    {
        sContext = aContext;

        writeString("SIGNAL INFORMATION ==============================\n", 50);

        writeString("Signal ", 7);
        writeDec32(aSignal->mNo);
        writeString("(", 1);
        sLen = idlOS::strlen(aSignal->mName);
        writeString(aSignal->mName, sLen);
        writeString(") caught.\n", 10);

        if(aSigInfo->si_code <= 0)
        {
            writeString("\tSent by process : ", 19);
            writeDec32(aSigInfo->si_pid);
            writeString("\n\tSent by user    : ", 20);
            writeDec32(aSigInfo->si_uid);
            writeString("\n", 1);
        }
        else
        {
            sTemp = NULL;
            switch(aSignal->mNo)
            {
            case SIGILL:
                if(aSigInfo->si_code <= 8)
                {
                    sTemp = gSigIllMsgs[aSigInfo->si_code - 1];
                }
                break;
            case SIGFPE:
                if(aSigInfo->si_code <= 8)
                {
                    sTemp = gSigFpeMsgs[aSigInfo->si_code - 1];
                }
                break;
            case SIGSEGV:
                if(aSigInfo->si_code <= 2)
                {
                    sTemp = gSigSegvMsgs[aSigInfo->si_code - 1];
                }
                break;
            case SIGBUS:
                if(aSigInfo->si_code <= 5)
                {
                    sTemp = gSigBusMsgs[aSigInfo->si_code - 1];
                }
                break;
            default:
                sTemp = NULL;
                break;
            }
                
            if(sTemp != NULL)
            {
                writeString(sTemp, idlOS::strlen(sTemp));
                writeString("\n", 1);
            }
            else
            {
                /* PROJ-2617 */
                writeString("Signal code : ", 14);
                writeDec32(aSigInfo->si_code);
                writeString("\n", 1);
            }

            switch(aSignal->mNo)
            {
            case SIGSEGV:   /* fall through */
            case SIGILL:    /* fall through */
            case SIGBUS:    /* fall through */
            case SIGFPE:    /* fall through */
            case SIGTRAP:   /* fall through */
                writeString("Fault address was 0x", 20);
                writeHexPtr(aSigInfo->si_addr);
                writeString("\n", 1);
                break;
            default:
                break;
                /* fall through */
            }
        }

        /* PROJ-2617 */
        if (aIsFaultTolerate == ID_TRUE)
        {
            writeString("Fault tolerated!\n", 17);
        }
        else
        {
            /* nothing to do */
        }
    
        writeString("BEGIN-DUMP ======================================\n", 50);
       
        writeBucket();
        if( aSqlString != NULL ) 
        {
            writeString( "SQL : ", 6 );
            writeString( aSqlString, idlOS::strlen(aSqlString) );
            writeString( "\n", 1 );
        }
        else
        {
            /* Do nothing */
        }
        
        writeString("END-DUMP ========================================\n", 50);

        writeString("BEGIN-STACK [CRASH] =============================\n", 50);
    }

# if defined(ALTI_CFG_OS_HPUX)
    U_STACK_TRACE();
# else
#  if defined(ALTI_CFG_OS_LINUX)
#   if defined(ALTI_CFG_CPU_POWERPC)
    mCallDepth = 0;

    sIP = (void*)sContext->uc_mcontext.gp_regs[36];
    sSP = (void**)sContext->uc_mcontext.gp_regs[1];

    do
    {
        mCallStack[mCallDepth] = sIP;
        mCallDepth++;

        writeString("Caller[", 7);
        writeDec32(mCallDepth);
        writeString("] ", 2);
        writeHexPtr(sIP);
        writeString("\n", 1);

        sSP = (void**)sSP[0];
        if( sSP == NULL)
        {
            sIP = NULL;
        }
        else
        {
            sIP = sSP[2];
        }
    } while((sSP != NULL) && (sIP != NULL));
#   else
    if(gLibGccHandle == NULL)
    {
#define NOUNWINDFUNC    \
        "Failed to get unwind functions.\n" \
        "Callstack cannot be logged.\n"
        writeString(NOUNWINDFUNC, idlOS::strlen(NOUNWINDFUNC));
#undef NOUNWINDFUNC
    }
    else
    {
        mCallDepth = tracestack(mCallStack, IDU_MAX_CALL_DEPTH);
        for(i = 0; i < mCallDepth; i++)
        {
            writeString("Caller[", 7);
            writeDec32(i);
            writeString("] ", 2);
            writeHexPtr(mCallStack[i]);
            writeString("\n", 1);
        }
    }
#   endif
#  elif defined(ALTI_CFG_OS_SOLARIS)
    mCallDepth = 0;
    walkcontext(sContext, iduStack::walker, NULL);
    for(i = 0; i < mCallDepth; i++)
    {
        writeString("Caller[", 7);
        writeDec32(i);
        writeString("] ", 2);
        writeHexPtr(mCallStack[i]);
        writeString("\n", 1);
    }
#  elif defined(ALTI_CFG_OS_AIX)
    mCallDepth = 0;

    sIP = (void*)sContext->uc_mcontext.jmp_context.iar;
    sSP = (void**)sContext->uc_mcontext.jmp_context.gpr[1];

    do
    {
        mCallStack[mCallDepth] = sIP;
        mCallDepth++;

        writeString("Caller[", 7);
        writeDec32(mCallDepth);
        writeString("] ", 2);
        writeHexPtr(sIP);
        writeString("\n", 1);

        sSP = (void**)sSP[0];
        if( sSP == NULL)
        {
            sIP = NULL;
        }
        else
        {
            sIP = sSP[2];
        }
    } while((sSP != NULL) && (sIP != NULL));
#  endif

# endif
    writeString("END-STACK =======================================\n", 50);
    /*
     * BUG-44297
     * Critical section end
     */
    mCallStackCriticalSection = 0;

    return;

    IDE_EXCEPTION_END;
    writeString("ERROR : Could not get context ===================\n", 50);
    /*
     * BUG-44297
     * Critical section end
     */
    mCallStackCriticalSection = 0;

    return;
#else
    PDL_UNUSED_ARG(aSignal);
    PDL_UNUSED_ARG(aSigInfo);
    PDL_UNUSED_ARG(aContext);
#endif
}

#endif

