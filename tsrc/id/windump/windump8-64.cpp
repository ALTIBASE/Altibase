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
#include <stdio.h>
#include <setjmp.h>
#include <windump.h>

//#pragma inline_depth(0)

void printCallstack2(
                    vULong     *aFrameAddr,
                    idBool      aIsCalledFromSignalHandler,
                    vULong      aCoreAddress)
{
  int i;
  vULong *sFrame;
  vULong *sCaller;
  jmp_buf sJump;
  setjmp(sJump);
  //idlOS::sleep(10);
  fprintf(stderr, "BEGIN-STACK-%s=====================================\n",
          (aIsCalledFromSignalHandler == ID_TRUE) ? "[CRASH]" : "[NORMAL]");
  fflush(stderr);

  if (aIsCalledFromSignalHandler == ID_TRUE)
    {
      // signal handler stack back trace
      sFrame = aFrameAddr;
      sCaller = (vULong *) *(sFrame + 1);
    }
  else
    {   // normal stack back trace
      sFrame  = (vULong *)(((_JUMP_BUFFER *) &sJump)->Frame);
      sCaller = 0;//(vULong *) *(sFrame + 1);

      fprintf(stderr, 
              "!!!!Rbp(%016" ID_vxULONG_FMT") "
              "caller=%016"ID_vxULONG_FMT"\n",
              sFrame, sCaller);
      fflush(stderr);

    }

  while(sCaller != NULL)
    {
      fprintf(stderr, 
              "Caller(%016" ID_vxULONG_FMT") Size=%016"
              ID_vxULONG_FMT" Thr=%"ID_UINT64_FMT"\n",
              (vULong)sCaller,
              0,
              0/*idlOS::getThreadID()*/);
      sFrame =  (vULong *)(*(sFrame));
      sCaller = (vULong *) *(sFrame + 1);

    }
  idlOS::fflush(stderr);
}
LONG __stdcall problem_signal_handler(LPEXCEPTION_POINTERS e)
{
    vULong  sCoreAddr;
    vULong *sFrame;

    sCoreAddr = (vULong  )(e->ExceptionRecord)->ExceptionAddress;
    sFrame    = (vULong *)((CONTEXT *)(e->ContextRecord))->Rbp;

 
//     fprintf(stderr, "[Context BLOCK]\n"
//         " [EDI:0x%08X ESI:0x%08X EBX:0x%08X EDX:0x%08X ECX:0x%08X EAX:0x%08X]\n"
//         " [EBP:0x%08X EIP:0x%08X SegCs:0x%08X EFlags:0x%08X ESP:0x%08X SegSs:0x%08X]\n"
//         " [Dr0:0x%08X Dr1:0x%08X Dr2:0x%08X Dr3:0x%08X Dr6:0x%08X Dr7:0x%08X] \n", 
//     ((CONTEXT *)(e->ContextRecord))->Edi,
//     ((CONTEXT *)(e->ContextRecord))->Esi,
//     ((CONTEXT *)(e->ContextRecord))->Ebx,
//     ((CONTEXT *)(e->ContextRecord))->Edx,
//     ((CONTEXT *)(e->ContextRecord))->Ecx,
//     ((CONTEXT *)(e->ContextRecord))->Eax,

//     ((CONTEXT *)(e->ContextRecord))->Ebp,
//     ((CONTEXT *)(e->ContextRecord))->Eip,
//     ((CONTEXT *)(e->ContextRecord))->SegCs,
//     ((CONTEXT *)(e->ContextRecord))->EFlags,
//     ((CONTEXT *)(e->ContextRecord))->Esp,
//     ((CONTEXT *)(e->ContextRecord))->SegSs,
    
//     ((CONTEXT *)(e->ContextRecord))->Dr0,
//     ((CONTEXT *)(e->ContextRecord))->Dr1,
//     ((CONTEXT *)(e->ContextRecord))->Dr2,
//     ((CONTEXT *)(e->ContextRecord))->Dr3,
//     ((CONTEXT *)(e->ContextRecord))->Dr6,
//     ((CONTEXT *)(e->ContextRecord))->Dr7
//     );
    printCallstack2(NULL, ID_FALSE, NULL);
    fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}


int func8(char arg[128])
{
  int i;
  char *core = NULL;

  jmp_buf sJump;

//   idlOS::sleep(15);
  setjmp(sJump);

//   fprintf(stderr, 
//           "Caller(%016" ID_vxULONG_FMT") ",
//           0xABCDABCDFFFFFFFF);


//   SetUnhandledExceptionFilter(problem_signal_handler);
  printCallstack2(NULL, ID_FALSE, NULL);


  //  *(core) = 0;
  return 0;
}
