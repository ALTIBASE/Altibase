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
 
// stackwalk.cpp : Defines the entry point for the console application.
//

#pragma warning( disable: 4786 )

#include <windows.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

// imagehlp.h must be compiled with packing to eight-byte-boundaries,
// but does nothing to enforce that. I am grateful to Jeff Shanholtz
// <JShanholtz@premia.com> for finding this problem.
#pragma pack( push, before_imagehlp, 8 )
#include <imagehlp.h>
#pragma pack( pop, before_imagehlp )

#define USE_TRY_CATCH

#define gle (GetLastError())
#define lenof(a) (sizeof(a) / sizeof((a)[0]))
#define MAXNAMELEN 1024 // max name length for found symbols
#define IMGSYMLEN ( sizeof IMAGEHLP_SYMBOL )
#define TTBUFLEN 65536 // for a temp buffer



// SymCleanup()
typedef BOOL (__stdcall *tSC)( IN HANDLE hProcess );

#ifdef _WIN64
// SymFunctionTableAccess()
typedef PVOID (__stdcall *tSFTA)( HANDLE hProcess, DWORD64 AddrBase );

// SymGetLineFromAddr()
typedef BOOL (__stdcall *tSGLFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
	OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line );

// SymGetModuleBase()
typedef DWORD64 (__stdcall *tSGMB)( IN HANDLE hProcess, IN DWORD64 dwAddr );

// SymGetModuleInfo()
typedef BOOL (__stdcall *tSGMI)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PIMAGEHLP_MODULE64 ModuleInfo );
#else
// SymFunctionTableAccess()
typedef PVOID (__stdcall *tSFTA)( HANDLE hProcess, DWORD AddrBase );

// SymGetLineFromAddr()
typedef BOOL (__stdcall *tSGLFA)( IN HANDLE hProcess, IN DWORD dwAddr,
	OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE Line );

// SymGetModuleBase()
typedef DWORD (__stdcall *tSGMB)( IN HANDLE hProcess, IN DWORD dwAddr );

// SymGetModuleInfo()
typedef BOOL (__stdcall *tSGMI)( IN HANDLE hProcess, IN DWORD dwAddr, OUT PIMAGEHLP_MODULE ModuleInfo );
#endif

// SymGetOptions()
typedef DWORD (__stdcall *tSGO)( VOID );

#ifdef _WIN64
// SymGetSymFromAddr()
typedef BOOL (__stdcall *tSGSFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
	OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_SYMBOL64 Symbol );
#else
// SymGetSymFromAddr()
typedef BOOL (__stdcall *tSGSFA)( IN HANDLE hProcess, IN DWORD dwAddr,
	OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_SYMBOL Symbol );
#endif

// SymInitialize()
typedef BOOL (__stdcall *tSI)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );

#ifdef _WIN64
// SymLoadModule()
typedef DWORD64 (__stdcall *tSLM)( IN HANDLE hProcess, IN HANDLE hFile,
	IN PSTR ImageName, IN PSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD SizeOfDll );
#else
// SymLoadModule()
typedef DWORD (__stdcall *tSLM)( IN HANDLE hProcess, IN HANDLE hFile,
	IN PSTR ImageName, IN PSTR ModuleName, IN DWORD BaseOfDll, IN DWORD SizeOfDll );
#endif

// SymSetOptions()
typedef DWORD (__stdcall *tSSO)( IN DWORD SymOptions );

#ifdef _WIN64
// StackWalk()
typedef BOOL (__stdcall *tSW)( DWORD MachineType, HANDLE hProcess,
	HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord,
	PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
	PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
	PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
	PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );
#else
// StackWalk()
typedef BOOL (__stdcall *tSW)( DWORD MachineType, HANDLE hProcess,
	HANDLE hThread, LPSTACKFRAME StackFrame, PVOID ContextRecord,
	PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
	PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
	PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
	PTRANSLATE_ADDRESS_ROUTINE TranslateAddress );
#endif

// UnDecorateSymbolName()
typedef DWORD (__stdcall WINAPI *tUDSN)( PCSTR DecoratedName, PSTR UnDecoratedName,
	DWORD UndecoratedLength, DWORD Flags );

tSC    pSymCleanup             = NULL;
tSFTA  pSymFunctionTableAccess = NULL;
tSGLFA pSymGetLineFromAddr     = NULL;
tSGMB  pSymGetModuleBase       = NULL;
tSGMI  pSymGetModuleInfo       = NULL;
tSGO   pSymGetOptions          = NULL;
tSGSFA pSymGetSymFromAddr      = NULL;
tSI    pSymInitialize          = NULL;
tSLM   pSymLoadModule          = NULL;
tSSO   pSymSetOptions          = NULL;
tSW    pStackWalk              = NULL;
tUDSN  pUnDecorateSymbolName   = NULL;


struct ModuleEntry
{
	std::string imageName;
	std::string moduleName;
	DWORD baseAddress;
	DWORD size;
};
typedef std::vector< ModuleEntry > ModuleList;
typedef ModuleList::iterator ModuleListIter;



int threadAbortFlag = 0;
HANDLE hTapTapTap = NULL;



void ShowStack( HANDLE hThread, CONTEXT& c ); // dump a stack
DWORD __stdcall test( void *arg );
LONG __stdcall Filter( EXCEPTION_POINTERS *ep );
void enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid );
bool fillModuleList( ModuleList& modules, DWORD pid, HANDLE hProcess );
bool fillModuleListTH32( ModuleList& modules, DWORD pid );
bool fillModuleListPSAPI( ModuleList& modules, DWORD pid, HANDLE hProcess );



int start_test_end( void ) // init -> test -> fini
{
	HANDLE hThread;
	DWORD idThread;
	HINSTANCE hImagehlpDll = NULL;

	// we load imagehlp.dll dynamically because the NT4-version does not
	// offer all the functions that are in the NT5 lib
	hImagehlpDll = LoadLibrary( "imagehlp.dll" );
	if ( hImagehlpDll == NULL )
	{
		printf( "LoadLibrary( \"imagehlp.dll\" ): gle = %lu\n", gle );
		return 1;
	}

	pSymCleanup             = (tSC)    GetProcAddress( hImagehlpDll, "SymCleanup" );
#ifdef _WIN64
	pSymFunctionTableAccess = (tSFTA)  GetProcAddress( hImagehlpDll, "SymFunctionTableAccess64" );
	pSymGetLineFromAddr     = (tSGLFA) GetProcAddress( hImagehlpDll, "SymGetLineFromAddr64" );
	pSymGetModuleBase       = (tSGMB)  GetProcAddress( hImagehlpDll, "SymGetModuleBase64" );
	pSymGetModuleInfo       = (tSGMI)  GetProcAddress( hImagehlpDll, "SymGetModuleInfo64" );
#else
	pSymFunctionTableAccess = (tSFTA)  GetProcAddress( hImagehlpDll, "SymFunctionTableAccess" );
	pSymGetLineFromAddr     = (tSGLFA) GetProcAddress( hImagehlpDll, "SymGetLineFromAddr" );
	pSymGetModuleBase       = (tSGMB)  GetProcAddress( hImagehlpDll, "SymGetModuleBase" );
	pSymGetModuleInfo       = (tSGMI)  GetProcAddress( hImagehlpDll, "SymGetModuleInfo" );
#endif
	pSymGetOptions          = (tSGO)   GetProcAddress( hImagehlpDll, "SymGetOptions" );
#ifdef _WIN64
	pSymGetSymFromAddr      = (tSGSFA) GetProcAddress( hImagehlpDll, "SymGetSymFromAddr64" );
#else
	pSymGetSymFromAddr      = (tSGSFA) GetProcAddress( hImagehlpDll, "SymGetSymFromAddr" );
#endif
	pSymInitialize          = (tSI)    GetProcAddress( hImagehlpDll, "SymInitialize" );
	pSymSetOptions          = (tSSO)   GetProcAddress( hImagehlpDll, "SymSetOptions" );
#ifdef _WIN64
	pStackWalk              = (tSW)    GetProcAddress( hImagehlpDll, "StackWalk64" );
#else
	pStackWalk              = (tSW)    GetProcAddress( hImagehlpDll, "StackWalk" );
#endif
	pUnDecorateSymbolName   = (tUDSN)  GetProcAddress( hImagehlpDll, "UnDecorateSymbolName" );
#ifdef _WIN64
	pSymLoadModule          = (tSLM)   GetProcAddress( hImagehlpDll, "SymLoadModule64" );
#else
	pSymLoadModule          = (tSLM)   GetProcAddress( hImagehlpDll, "SymLoadModule" );
#endif

	if ( pSymCleanup             == NULL ||
         pSymFunctionTableAccess == NULL || 
         pSymGetModuleBase       == NULL || 
         pSymGetModuleInfo       == NULL ||
		 pSymGetOptions          == NULL || 
         pSymGetSymFromAddr      == NULL || 
         pSymInitialize          == NULL || 
         pSymSetOptions          == NULL ||
		 pStackWalk              == NULL || 
         pUnDecorateSymbolName   == NULL || 
         pSymLoadModule          == NULL )
	{
		puts( "GetProcAddress(): some required function not found." );
		FreeLibrary( hImagehlpDll );
		return 1;
	}

	hTapTapTap = CreateEvent( NULL, false, false, NULL );

	hThread = CreateThread( NULL, 5*524288, test, NULL, 0, &idThread );
	// let the thread complete the exception-handler part
	// and yes, I know I should check the return code
	WaitForSingleObject( hTapTapTap, INFINITE );
	CloseHandle( hTapTapTap );

	// time to stop the victim
	if ( (DWORD) (-1L) == SuspendThread( hThread ) )
	{
		printf( "SuspendThread(): gle = %lu\n", gle );
		goto cleanup;
	}

	CONTEXT c;
	memset( &c, '\0', sizeof c );
	c.ContextFlags = CONTEXT_FULL;

	// init CONTEXT record so we know where to start the stackwalk
	if ( ! GetThreadContext( hThread, &c ) )
	{
		printf( "GetThreadContext(): gle = %lu\n", gle );
		goto cleanup;
	}

	ShowStack( hThread, c );

cleanup:
	++ threadAbortFlag;
	Sleep( 100 ); // to let the thread commit suicide
	FreeLibrary( hImagehlpDll );
	return 0;
}



// if you use C++ exception handling: install a translator function
// with set_se_translator(). In the context of that function (but *not*
// afterwards), you can either do your stack dump, or save the CONTEXT
// record as a local copy. Note that you must do the stack sump at the
// earliest opportunity, to avoid the interesting stackframes being gone
// by the time you do the dump.
LONG __stdcall Filter( EXCEPTION_POINTERS *ep )
{
	HANDLE hThread;

	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &hThread, 0, false, DUPLICATE_SAME_ACCESS );
	ShowStack( hThread, *(ep->ContextRecord) );
	CloseHandle( hThread );

	return EXCEPTION_EXECUTE_HANDLER;
}

DWORD __stdcall test( void *arg )
{
    (void) arg;
	// first, we cause an exception and dump our own stack
	__try
	{
		char *p = NULL;
		*p = 'A'; // BANG!
	}
	__except ( Filter( GetExceptionInformation() ) )
	{
		printf( "Handled exception.\n" );
	}
    //SetUnhandledExceptionFilter(Filter);

	// phase 1 done, tell main thread we can be autopSymInitializeed from the outside
	SetEvent( hTapTapTap );

	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );
	for ( ; ! threadAbortFlag; )
	{
		Sleep( 10 );
	}
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );

    return 0;
}


void ShowStack( HANDLE hThread, CONTEXT& c )
{
	// normally, call ImageNtHeader() and use machine info from PE header
#ifdef _WIN64
	DWORD    imageType = IMAGE_FILE_MACHINE_AMD64;
#else
	DWORD    imageType = IMAGE_FILE_MACHINE_I386;
#endif
	HANDLE   hProcess  = GetCurrentProcess(); // hProcess normally comes from outside
	int      frameNum; // counts walked frames
// tells us how far from the symbol we were
#ifdef _WIN64
	DWORD64  offsetFromSymbol;
#else
	DWORD    offsetFromSymbol;
#endif
	DWORD    symOptions; // symbol handler settings
	char     undName[MAXNAMELEN]; // undecorated name
	char     undFullName[MAXNAMELEN]; // undecorated name with all shenanigans
	IMAGEHLP_SYMBOL *pSym = (IMAGEHLP_SYMBOL *) malloc( IMGSYMLEN + MAXNAMELEN );
#ifdef _WIN64
	IMAGEHLP_MODULE64 Module;
	IMAGEHLP_LINE64   Line;
	STACKFRAME64      s; // in/out stackframe
#else
	IMAGEHLP_MODULE Module;
	IMAGEHLP_LINE   Line;
	STACKFRAME      s; // in/out stackframe
#endif
	std::string     symSearchPath;
	char *tt = 0, *p;

	memset( &s, '\0', sizeof s );

	// NOTE: normally, the exe directory and the current directory should be taken
	// from the target process. The current dir would be gotten through injection
	// of a remote thread; the exe fir through either ToolHelp32 or PSAPI.

	tt = new char[TTBUFLEN]; // this is a _sample_. you can do the error checking yourself.

	// build symbol search path from:
	symSearchPath = "";
	// current directory
	if ( GetCurrentDirectory( TTBUFLEN, tt ) )
		symSearchPath += tt + std::string( ";" );
	// dir with executable
	if ( GetModuleFileName( 0, tt, TTBUFLEN ) )
	{
		for ( p = tt + strlen( tt ) - 1; p >= tt; -- p )
		{
			// locate the rightmost path separator
			if ( *p == '\\' || *p == '/' || *p == ':' )
				break;
		}
		// if we found one, p is pointing at it; if not, tt only contains
		// an exe name (no path), and p points before its first byte
		if ( p != tt ) // path sep found?
		{
			if ( *p == ':' ) // we leave colons in place
				++ p;
			*p = '\0'; // eliminate the exe name and last path sep
			symSearchPath += tt + std::string( ";" );
		}
	}
	// environment variable _NT_SYMBOL_PATH
	if ( GetEnvironmentVariable( "_NT_SYMBOL_PATH", tt, TTBUFLEN ) )
		symSearchPath += tt + std::string( ";" );
	// environment variable _NT_ALTERNATE_SYMBOL_PATH
	if ( GetEnvironmentVariable( "_NT_ALTERNATE_SYMBOL_PATH", tt, TTBUFLEN ) )
		symSearchPath += tt + std::string( ";" );
	// environment variable SYSTEMROOT
	if ( GetEnvironmentVariable( "SYSTEMROOT", tt, TTBUFLEN ) )
		symSearchPath += tt + std::string( ";" );

	if ( symSearchPath.size() > 0 ) // if we added anything, we have a trailing semicolon
		symSearchPath = symSearchPath.substr( 0, symSearchPath.size() - 1 );

	printf( "symbols path: %s\n", symSearchPath.c_str() );

	// why oh why does SymInitialize() want a writeable string?
	strncpy( tt, symSearchPath.c_str(), TTBUFLEN );
	tt[TTBUFLEN - 1] = '\0'; // if strncpy() overruns, it doesn't add the null terminator

	// init symbol handler stuff (SymInitialize())
	if ( ! pSymInitialize( hProcess, tt, false ) )
	{
		printf( "SymInitialize(): gle = %lu\n", gle );
		goto cleanup;
	}

	// SymGetOptions()
	symOptions = pSymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES;
	symOptions &= ~SYMOPT_UNDNAME;
	pSymSetOptions( symOptions ); // SymSetOptions()

	// Enumerate modules and tell imagehlp.dll about them.
	// On NT, this is not necessary, but it won't hurt.
	//enumAndLoadModuleSymbols( hProcess, GetCurrentProcessId() );

	// init STACKFRAME for first call
	// Notes: AddrModeFlat is just an assumption. I hate VDM debugging.
	// Notes: will have to be #ifdef-ed for Alphas; MIPSes are dead anyway,
	// and good riddance.
#ifdef _WIN64
// 	s.AddrPC.Offset     = c.StIIP;
// 	s.AddrPC.Mode       = AddrModeFlat;
//     s.AddrStack.Offset  = c.IntSp;
//     s.AddrStack.Mode    = AddrModeFlat;
//     s.AddrFrame.Offset  = 0;
//     s.AddrFrame.Mode    = AddrModeFlat;
//     s.AddrBStore.Offset = c.RsBSP;
//     s.AddrBStore.Mode   = AddrModeFlat;

// #elif _M_X64
  imageType = IMAGE_FILE_MACHINE_AMD64;
  s.AddrPC.Offset = c.Rip;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.Rsp;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Offset = c.Rsp;
  s.AddrStack.Mode = AddrModeFlat;
// #elif _M_IA64
//   imageType = IMAGE_FILE_MACHINE_IA64;
//   s.AddrPC.Offset = c.StIIP;
//   s.AddrPC.Mode = AddrModeFlat;
//   s.AddrFrame.Offset = c.IntSp;
//   s.AddrFrame.Mode = AddrModeFlat;
//   s.AddrBStore.Offset = c.RsBSP;
//   s.AddrBStore.Mode = AddrModeFlat;
//   s.AddrStack.Offset = c.IntSp;
//   s.AddrStack.Mode = AddrModeFlat;

#else
	s.AddrPC.Offset    = c.Eip;
	s.AddrPC.Mode      = AddrModeFlat;
	s.AddrFrame.Offset = c.Ebp;
	s.AddrFrame.Mode   = AddrModeFlat;
#endif

	memset( pSym, '\0', IMGSYMLEN + MAXNAMELEN );
	pSym->SizeOfStruct = IMGSYMLEN;
	pSym->MaxNameLength = MAXNAMELEN;

	memset( &Line, '\0', sizeof Line );
	Line.SizeOfStruct = sizeof Line;

	memset( &Module, '\0', sizeof Module );
	Module.SizeOfStruct = sizeof Module;

	offsetFromSymbol = 0;

	printf( "\n--# FV EIP----- RetAddr- FramePtr StackPtr Symbol\n" );
	for ( frameNum = 0; ; ++ frameNum )
	{
		// get next stack frame (StackWalk(), SymFunctionTableAccess(), SymGetModuleBase())
		// if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
		// assume that either you are done, or that the stack is so hosed that the next
		// deeper frame could not be found.
		if ( ! pStackWalk( imageType, hProcess, hThread, &s, &c, NULL,
			pSymFunctionTableAccess, pSymGetModuleBase, NULL ) )
		{
			break;
		}

		// display its contents
		printf( "\n%3d %c%c %08lx %08lx %08lx %08lx ",
			frameNum, s.Far? 'F': '.', s.Virtual? 'V': '.',
			s.AddrPC.Offset, s.AddrReturn.Offset,
			s.AddrFrame.Offset, s.AddrStack.Offset );

		if ( s.AddrPC.Offset == 0 )
		{
			printf( "(-nosymbols- PC == 0)\n" );
		}
		else
		{ // we seem to have a valid PC
			// show procedure info (SymGetSymFromAddr())
			if ( ! pSymGetSymFromAddr( hProcess, s.AddrPC.Offset, &offsetFromSymbol, pSym ) )
			{
				if ( gle != 487 )
					printf( "SymGetSymFromAddr(): gle = %lu\n", gle );
			}
			else
			{
				// UnDecorateSymbolName()
				pUnDecorateSymbolName( pSym->Name, undName, MAXNAMELEN, UNDNAME_NAME_ONLY );
				pUnDecorateSymbolName( pSym->Name, undFullName, MAXNAMELEN, UNDNAME_COMPLETE );
				printf( "%s", undName );
				if ( offsetFromSymbol != 0 )
					printf( " %+ld bytes", (long) offsetFromSymbol );
				putchar( '\n' );
				printf( "    Sig:  %s\n", pSym->Name );
				printf( "    Decl: %s\n", undFullName );
			}

			// show line number info, NT5.0-method (SymGetLineFromAddr())
			if ( pSymGetLineFromAddr != NULL )
			{ // yes, we have SymGetLineFromAddr()
#ifdef _WIN64
				if ( ! pSymGetLineFromAddr( hProcess, s.AddrPC.Offset, reinterpret_cast<PDWORD>(&offsetFromSymbol), &Line ) )
#else
				if ( ! pSymGetLineFromAddr( hProcess, s.AddrPC.Offset, &offsetFromSymbol, &Line ) )
#endif
				{
					if ( gle != 487 )
						printf( "SymGetLineFromAddr(): gle = %lu\n", gle );
				}
				else
				{
					printf( "    Line: %s(%lu) %+ld bytes\n",
						Line.FileName, Line.LineNumber, offsetFromSymbol );
				}
			} // yes, we have SymGetLineFromAddr()

			// show module info (SymGetModuleInfo())
			if ( ! pSymGetModuleInfo( hProcess, s.AddrPC.Offset, &Module ) )
			{
				printf( "SymGetModuleInfo): gle = %lu\n", gle );
			}
			else
			{ // got module info OK
				char ty[80];
				switch ( Module.SymType )
				{
				case SymNone:
					strcpy( ty, "-nosymbols-" );
					break;
				case SymCoff:
					strcpy( ty, "COFF" );
					break;
				case SymCv:
					strcpy( ty, "CV" );
					break;
				case SymPdb:
					strcpy( ty, "PDB" );
					break;
				case SymExport:
					strcpy( ty, "-exported-" );
					break;
				case SymDeferred:
					strcpy( ty, "-deferred-" );
					break;
				case SymSym:
					strcpy( ty, "SYM" );
					break;
				default:
					_snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.SymType );
					break;
				}

				printf( "    Mod:  %s[%s], base: %08lxh\n",
					Module.ModuleName, Module.ImageName, Module.BaseOfImage );
				printf( "    Sym:  type: %s, file: %s\n",
					ty, Module.LoadedImageName );
			} // got module info OK
		} // we seem to have a valid PC

		// no return address means no deeper stackframe
		if ( s.AddrReturn.Offset == 0 )
		{
			// avoid misunderstandings in the printf() following the loop
			SetLastError( 0 );
			break;
		}

	} // for ( frameNum )

	if ( gle != 0 )
		printf( "\nStackWalk(): gle = %lu\n", gle );

cleanup:
	ResumeThread( hThread );
	// de-init symbol handler etc. (SymCleanup())
	pSymCleanup( hProcess );
	free( pSym );
	delete [] tt;
}



void enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid )
{
	ModuleList modules;
	ModuleListIter it;
	char *img, *mod;

	// fill in module list
	fillModuleList( modules, pid, hProcess );

	for ( it = modules.begin(); it != modules.end(); ++ it )
	{
		// unfortunately, SymLoadModule() wants writeable strings
		img = new char[(*it).imageName.size() + 1];
		strcpy( img, (*it).imageName.c_str() );
		mod = new char[(*it).moduleName.size() + 1];
		strcpy( mod, (*it).moduleName.c_str() );

		if ( pSymLoadModule( hProcess, 0, img, mod, (*it).baseAddress, (*it).size ) == 0 )
			printf( "Error %lu loading symbols for \"%s\"\n",
				gle, (*it).moduleName.c_str() );
		else
			printf( "Symbols loaded: \"%s\"\n", (*it).moduleName.c_str() );

		delete [] img;
		delete [] mod;
	}
}


bool fillModuleList( ModuleList& modules, DWORD pid, HANDLE hProcess )
{
	// try toolhelp32 first
	if ( fillModuleListTH32( modules, pid ) )
		return true;
	// nope? try psapi, then
	return fillModuleListPSAPI( modules, pid, hProcess );
}



// miscellaneous toolhelp32 declarations; we cannot #include the header
// because not all systems may have it
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE   0x00000008
#pragma pack( push, 8 )
typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;
#pragma pack( pop )



bool fillModuleListTH32( ModuleList& modules, DWORD pid )
{
	// CreateToolhelp32Snapshot()
	typedef HANDLE (__stdcall *tCT32S)( DWORD dwFlags, DWORD th32ProcessID );
	// Module32First()
	typedef BOOL (__stdcall *tM32F)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
	// Module32Next()
	typedef BOOL (__stdcall *tM32N)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );

	// I think the DLL is called tlhelp32.dll on Win9X, so we try both
	const char *dllname[] = { "kernel32.dll", "tlhelp32.dll" };
	HINSTANCE hToolhelp;
	tCT32S pCT32S;
	tM32F pM32F;
	tM32N pM32N;

	HANDLE hSnap;
	MODULEENTRY32 me = { sizeof me };
	bool keepGoing;
	ModuleEntry e;
	int i;

	for ( i = 0; i < lenof( dllname ); ++ i )
	{
		hToolhelp = LoadLibrary( dllname[i] );
		if ( hToolhelp == 0 )
			continue;
		pCT32S = (tCT32S) GetProcAddress( hToolhelp, "CreateToolhelp32Snapshot" );
		pM32F = (tM32F) GetProcAddress( hToolhelp, "Module32First" );
		pM32N = (tM32N) GetProcAddress( hToolhelp, "Module32Next" );
		if ( pCT32S != 0 && pM32F != 0 && pM32N != 0 )
			break; // found the functions!
		FreeLibrary( hToolhelp );
		hToolhelp = 0;
	}

	if ( hToolhelp == 0 ) // nothing found?
		return false;

	hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
	if ( hSnap == (HANDLE) -1 )
		return false;

	keepGoing = !!pM32F( hSnap, &me );
	while ( keepGoing )
	{
		// here, we have a filled-in MODULEENTRY32
		printf( "%08lXh %6lu %-15.15s %s\n", me.modBaseAddr, me.modBaseSize, me.szModule, me.szExePath );
		e.imageName = me.szExePath;
		e.moduleName = me.szModule;
		e.baseAddress = (DWORD) me.modBaseAddr;
		e.size = me.modBaseSize;
		modules.push_back( e );
		keepGoing = !!pM32N( hSnap, &me );
	}

	CloseHandle( hSnap );

	FreeLibrary( hToolhelp );

	return modules.size() != 0;
}



// miscellaneous psapi declarations; we cannot #include the header
// because not all systems may have it
typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;



bool fillModuleListPSAPI( ModuleList& modules, DWORD pid, HANDLE hProcess )
{
	// EnumProcessModules()
	typedef BOOL (__stdcall *tEPM)( HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded );
	// GetModuleFileNameEx()
	typedef DWORD (__stdcall *tGMFNE)( HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
	// GetModuleBaseName() -- redundant, as GMFNE() has the same prototype, but who cares?
	typedef DWORD (__stdcall *tGMBN)( HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
	// GetModuleInformation()
	typedef BOOL (__stdcall *tGMI)( HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize );

	HINSTANCE hPsapi;
	tEPM pEPM;
	tGMFNE pGMFNE;
	tGMBN pGMBN;
	tGMI pGMI;

	int i;
	ModuleEntry e;
	DWORD cbNeeded;
	MODULEINFO mi;
	HMODULE *hMods = 0;
	char *tt = 0;

	hPsapi = LoadLibrary( "psapi.dll" );
	if ( hPsapi == 0 )
		return false;

	modules.clear();

	pEPM = (tEPM) GetProcAddress( hPsapi, "EnumProcessModules" );
	pGMFNE = (tGMFNE) GetProcAddress( hPsapi, "GetModuleFileNameExA" );
	pGMBN = (tGMFNE) GetProcAddress( hPsapi, "GetModuleBaseNameA" );
	pGMI = (tGMI) GetProcAddress( hPsapi, "GetModuleInformation" );
	if ( pEPM == 0 || pGMFNE == 0 || pGMBN == 0 || pGMI == 0 )
	{
		// yuck. Some API is missing.
		FreeLibrary( hPsapi );
		return false;
	}

	hMods = new HMODULE[TTBUFLEN / sizeof HMODULE];
	tt = new char[TTBUFLEN];
	// not that this is a sample. Which means I can get away with
	// not checking for errors, but you cannot. :)

	if ( ! pEPM( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
	{
		printf( "EPM failed, gle = %lu\n", gle );
		goto cleanup;
	}

	if ( cbNeeded > TTBUFLEN )
	{
		printf( "More than %lu module handles. Huh?\n", lenof( hMods ) );
		goto cleanup;
	}

	for ( i = 0; i < cbNeeded / sizeof hMods[0]; ++ i )
	{
		// for each module, get:
		// base address, size
		pGMI( hProcess, hMods[i], &mi, sizeof mi );
		e.baseAddress = (DWORD) mi.lpBaseOfDll;
		e.size = mi.SizeOfImage;
		// image file name
		tt[0] = '\0';
		pGMFNE( hProcess, hMods[i], tt, TTBUFLEN );
		e.imageName = tt;
		// module name
		tt[0] = '\0';
		pGMBN( hProcess, hMods[i], tt, TTBUFLEN );
		e.moduleName = tt;
		printf( "%08lXh %6lu %-15.15s %s\n", e.baseAddress,
			e.size, e.moduleName.c_str(), e.imageName.c_str() );

		modules.push_back( e );
	}

cleanup:
	if ( hPsapi )
		FreeLibrary( hPsapi );
	delete [] tt;
	delete [] hMods;

	return modules.size() != 0;
}

int main(int argc, char *argv[])
{
  int i;
  start_test_end();
  return 0;
}

