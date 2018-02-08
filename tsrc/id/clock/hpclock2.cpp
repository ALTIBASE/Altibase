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
 
/*
 * *****************************************************************
 * *                                                               *
 * *   Copyright 2003 HP Partner Technology Access Center - MA      *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of            *
 * *   Hewlett-Packard Corporation.  Possession, use,  duplication *
 * *   or dissemination of the software and media is authorized    *
 * *   only pursuant to a valid written license from               * 
 * *   Hewlett-Packard Corporation.                                *
 * *                                                               *
 * *   Hewlett-Packard Corporation makes no representations about  *
 * *   the suitability of the software described herein for any    *
 * *   purpose.  It is provided "as is" without express or implied *
 * *   warranty.                                                   *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 *
 * The TICKS macro can be used to access the high-resolution counters
 * on the CPU for timing. These counters increment at a rate based on
 * the CPU cycle clock. This does not imply a 1:1 ratio. 
 *
 * Use caution, particularly when using these macros, as
 * there is no consideration to overflow or changing CPUs. The counters
 * in multi-CPU systems are not required to be in sync and may be 
 * significantly different.
 *
 * Use these timers only for VERY small windows, 1-2 milliseconds
 * to reduce the risk of these problems. These counters return 64-bit
 * integer values and should not be stored in int type variables, 
 * whenever possible the C99 uint64_t type should be used.
 *
 */
#ifndef __TICKS_H_
#define __TICKS_H
#include <inttypes.h>
/* Check for HP Tru64 UNIX compilers */
#if defined(__DECC_VER) || defined(__DECCXX_VER)
/*
 * Handle the Alpha Processor Family
 */
#if defined(__alpha)
#include <alpha/builtins.h>
#define __TICKS __RPCC()
#endif /* __alpha */
/* Check for the HP-UX Compilers */
#elif defined(__HP_aCC) || defined(__HP_cc)
#if defined (__STDC_32_MODE__) 
#error "Must use +e or -Ae for 64-bit integer support"
#endif /* __STDC_32_MODE__ */
/*
 * Handle the Intel Itanium Processor Family
 */
#if defined(__ia64)
#include <machine/sys/inline.h>
#define __TICKS _Asm_mov_from_ar(_AREG_ITC)
/*
 * Handle the PA-RISC Processor Family
 */
#elif defined(__hppa)
#if defined (__HP_aCC)
#error "Inline assembler not supported in C++, compile using ANSI C"
#endif /* __HP_aCC */
#include <machine/inline.h>
#include <machine/reg.h>
#ifdef __cplusplus
inline
#else /* __cplusplus */
#pragma INLINE __TICKS_f
#endif /* __cplusplus */
static uint64_t __TICKS_f(void) {
 register uint64_t _ticks;
 _MFCTL(CR16,_ticks);
 return _ticks;
}
#define __TICKS __TICKS_f()
#else 
#error "Unknown chip for compiler."
#endif /* arch && ( __HP_aCC || __HP_cc ) */
/*
 * Check for GCC 
 */
#elif defined(__GNUC__)
#ifdef __cplusplus
inline
#endif /* __cplusplus */
static uint64_t __TICKS_f(void) {
 register uint64_t _ticks;
#if defined(__alpha) || defined(__alpha__)
/*
 * Handle the Alpha Processor Family
 */
 asm volatile ("rpcc %0" : "=r"(_ticks));
#elif defined(__ia64) || defined(__ia64__)
/*
 * Handle the Intel Itanium Processor Family
 */
 asm volatile ("mov %0 = ar.itc ": "=r" (_ticks) );
#elif defined(__hppa) || defined(__hppa__)
/*
 * Handle the PA-RISC Processor Family
 */
 asm volatile ("mfctl %%cr16, %0" : "=r" (_ticks) : "0" (_ticks));
#else
#error "Unknown Architecture"
#endif /* GCC arch test */
 return _ticks;
}
#define __TICKS __TICKS_f()
#else
#error "Unknown Compiler"
#endif /* Complier Tests */
#endif /* __TICKS_H */


int main()
{
  return 0;
}
