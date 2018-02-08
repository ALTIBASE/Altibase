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
 
#ifndef _O_IDEFAULTMGR_H_
#define _O_IDEFAULTMGR_H_ 1

#include <idl.h>
#include <setjmp.h>

/*
 * __func__ macro is C99 standard and GCC old version support only __FUNCTION__ macro.
 * And C/C++ compiler on solaris doesn't support __FUNCTION__ macro.
 */
#if defined(ALTI_CFG_OS_SOLARIS)
#define ID_FUNCTION __func__
#else /* ALTI_CFG_OS_SOLARIS */
#define ID_FUNCTION __FUNCTION__
#endif /* ALTI_CFG_OS_SOLARIS */

/* To prevent re-odering instructions under HP-UX/IA-64 environment */
#if defined(ALTI_CFG_OS_HPUX) && defined(ALTI_CFG_CPU_IA64)
#define ID_FT_VOLATILE volatile
#else /* ALTI_CFG_OS_HPUX */
#define ID_FT_VOLATILE 
#endif /* ALTI_CFG_OS_HPUX */

#define IDE_FT_MAX_CALLSTACK_SIZE (2048)

#ifndef GEN_ERR_MSG
IDL_EXTERN_C SInt gIdeFTTrace;
#else
static       SInt gIdeFTTrace;
#endif

typedef struct ideFaultCallStackElement
{
    jmp_buf      mJmpBuf;
    const SChar *mEntryFuncName;
} ideFaultCallStackElement;

typedef struct ideFaultMgr
{
    idBool                    mIsThreadEnable;
    ideFaultCallStackElement  mCallStack[IDE_FT_MAX_CALLSTACK_SIZE];
    ID_FT_VOLATILE SInt       mCallStackNext;
    idBool                    mIsCallStackException;  /* ID_TRUE, if overflow or underflow */
    ID_FT_VOLATILE idBool     mIsTransientDisable;    /* [NOFT] */
    const SChar              *mDisabledEntryFuncName; /* [NOFT] */
    SInt                      mDisabledCallCount;     /* [NOFT] more than 2, if the function is called with IDE_NOFT_BEGIN() recursively */
    ID_FT_VOLATILE idBool     mIsExceptionDisable;    /* [EXPT] only for exception section (with from signal handler) */
    sigset_t                  mSavedSigMask;
    SInt                      mDummyFlag;             /* dummy flag for preventing a specific code optimization which can be used for register like %rax instead of stack for local variable */
} ideFaultMgr;

IDE_RC ideEnableFaultMgr(idBool aIsThreadEnable);
idBool ideIsEnabledFaultMgr();
void   ideClearFTCallStack();

idBool ideCanFaultTolerate(SInt        aSigNum,
                           siginfo_t  *aSigInfo,
                           ucontext_t *aContext);

idBool ideCanFaultTolerate();

void   ideNonLocalJumpForFaultTolerance();

#ifndef GEN_ERR_MSG
void   ideLogFT(const SChar *aFunction,
                const SChar *aFile,
                SInt         aLine,
                const SChar *aFormat,
                ...);

void   ideLogFTMacro(const SChar *aFunction,
                     const SChar *aFile,
                     SInt         aLine,
                     SInt         aCallLevel,
                     const SChar *aFTMacro);
#else
static
void   ideLogFT(const SChar *aFunction,
                const SChar *aFile,
                SInt         aLine,
                const SChar *aFormat,
                ...)
{
    PDL_UNUSED_ARG(aFunction);
    PDL_UNUSED_ARG(aFile);
    PDL_UNUSED_ARG(aLine);
    PDL_UNUSED_ARG(aFormat);
}

static
void   ideLogFTMacro(const SChar *aFunction,
                     const SChar *aFile,
                     SInt         aLine,
                     SInt         aCallLevel,
                     const SChar *aFTMacro)
{
    PDL_UNUSED_ARG(aFunction);
    PDL_UNUSED_ARG(aFile);
    PDL_UNUSED_ARG(aLine);
    PDL_UNUSED_ARG(aCallLevel);
    PDL_UNUSED_ARG(aFTMacro);
}
#endif

#endif /* _O_IDEFAULTMGR_H_ */

