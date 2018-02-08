/*****************************************************************************
 * Copyright 1999-2000, RTBase Corporation or its subsidiaries.
 * All rights reserved.
 ****************************************************************************/

/*****************************************************************************
 * $Id: iduStack.h 82088 2018-01-18 09:21:15Z yoonhee.kim $
 ****************************************************************************/

#ifndef _O_IDE_STACK_H_
#define _O_IDE_STACK_H_ 1

#include <idl.h>
#include <ide.h>

#define IDU_MAX_CALL_DEPTH (1024)

#define IDU_SET_BUCKET      iduStack::setBucketList
#define IDU_CLEAR_BUCKET    ideStack::clearBucketList

typedef void iduHandler(int, struct siginfo*, void*);

#if defined(ALTI_CFG_OS_WINDOWS)
typedef BOOL (__stdcall* iduSymInit)(HANDLE, PSTR, BOOL);

typedef BOOL (__stdcall *iduStackWalker)(
        DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID,
        PREAD_PROCESS_MEMORY_ROUTINE64,
        PFUNCTION_TABLE_ACCESS_ROUTINE64,
        PGET_MODULE_BASE_ROUTINE64,
        PTRANSLATE_ADDRESS_ROUTINE64);
#else
typedef struct iduSignalDef
{
    SInt            mNo;        /* Signal number */
    SChar           mName[64];  /* Signal name */
    SChar           mDesc[128]; /* Signal description */
    UInt            mFlags;     /* Signal flags */
    iduHandler*     mFunc;      /* Signal handler function */
} iduSignalDef;
#endif

class iduStack
{
public:
    static IDE_RC   initializeStatic(void);
    static void     setBucketList(ideLogModule, const SChar*, ...);
    static void     clearBucketList(ideLogModule);

    static IDE_RC   setSigStack(void);

#if defined(ALTI_CFG_OS_WINDOWS)
    static void __cdecl dumpStack( const CONTEXT* = NULL,
                                   SChar*         = NULL );
#else
    static void     dumpStack( const iduSignalDef*   = NULL,
                               siginfo_t*            = NULL,
                               ucontext_t*           = NULL,
                               idBool                = ID_FALSE, /* PROJ-2617 */
                               SChar*                = NULL );
#endif

private:
    static SInt             mCallStackCriticalSection;
    static const SChar      mDigitChars[17];

    static IDTHREAD SInt    mCallDepth;
    static IDTHREAD void*   mCallStack[IDU_MAX_CALL_DEPTH];
    static PDL_HANDLE       mFD;

    static void convertHex64(ULong, SChar*, SChar**);
    static void convertDec32(UInt , SChar*, SChar**);
    static void writeBucket();
    static void writeDec32(UInt);
    static void writeHex32(UInt);
    static void writeHex64(ULong);
    static void writeHexPtr(void*);
    static void writeString(const void*, size_t);

#if defined(ALTI_CFG_OS_WINDOWS)
    static iduSymInit                           mSymInitFunc;
    static iduStackWalker                       mWalkerFunc;

    static HMODULE                              mHModule;
    static PFUNCTION_TABLE_ACCESS_ROUTINE64     mAccessFunc;
    static PGET_MODULE_BASE_ROUTINE64           mGetModuleFunc;
#else

    static IDTHREAD SChar                       mSigAltStack[SIGSTKSZ];

# if defined(ALTI_CFG_OS_SOLARIS)
    static int walker(uintptr_t, int, void*);
# elif defined(ALTI_CFG_OS_LINUX) && !defined(ALTI_CFG_CPU_POWERPC)
    static SInt     tracestack(void**, SInt);
# else
# endif
#endif
};

#endif
