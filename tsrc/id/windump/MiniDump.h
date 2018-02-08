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
 
#ifndef __MINIDUMP_H__
#define __MINIDUMP_H__

#pragma once

#include <windows.h>
#include <dbghelp.h>
#include <tchar.h>

class CMiniDump
{
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(	HANDLE hProcess,
											DWORD dwPid,
											HANDLE hFile,
											MINIDUMP_TYPE DumpType,
											CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
											CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
											CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

// A couple of private smart types
struct CSmartModule
{
	HMODULE m_hModule;

	CSmartModule(HMODULE h)
	{
		m_hModule = h;
	}

	~CSmartModule()
	{
		if(m_hModule != NULL)
		{
			FreeLibrary(m_hModule);
		}
	}

	operator HMODULE()
	{
		return m_hModule;
	}
};

struct CSmartHandle
{
	HANDLE m_h;

	CSmartHandle(HANDLE h)
	{
		m_h = h;
	}

	~CSmartHandle()
	{
		if(m_h != NULL && m_h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_h);
		}
	}

	operator HANDLE()
	{
		return m_h;
	}
};


public:
	// Only public method
	static BOOL Create(HMODULE hModule, PEXCEPTION_POINTERS pExceptionInfo, LPCRITICAL_SECTION pCS)
	{
		BOOL bRet = FALSE;
		DWORD dwLastError = 0;

		CSmartHandle hImpersonationToken = NULL;
		if(!GetImpersonationToken(&hImpersonationToken.m_h))
		{
			return FALSE;
		}

		// Load DBGHELP.dll
		CSmartModule hDbgDll = LocalLoadLibrary(hModule, _T("DBGHELP.dll"));
		if(hDbgDll == NULL)
		{
			return FALSE;
		}

		// Get a pointer to MiniDumpWriteDump
		MINIDUMPWRITEDUMP pDumpFunction = (MINIDUMPWRITEDUMP)::GetProcAddress(hDbgDll, "MiniDumpWriteDump");
		if(NULL == pDumpFunction)
		{
			return FALSE;
		}

		// Create filename
		TCHAR szFile[MAX_PATH + 2];
		if(!GenerateDumpFilename(szFile, sizeof(szFile) / sizeof(szFile[0])))
		{
			return FALSE;
		}

		// Create the dump file
		CSmartHandle hDumpFile = ::CreateFile(	szFile, 
												GENERIC_READ | GENERIC_WRITE, 
												FILE_SHARE_WRITE | FILE_SHARE_READ, 
												0, CREATE_ALWAYS, 0, 0);
		if(hDumpFile == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}

		// Write the dump
		MINIDUMP_EXCEPTION_INFORMATION stInfo = {0};

		stInfo.ThreadId = GetCurrentThreadId();
		stInfo.ExceptionPointers = pExceptionInfo;
		stInfo.ClientPointers = TRUE;

		// We need the SeDebugPrivilege to be able to run MiniDumpWriteDump
		TOKEN_PRIVILEGES tp;
		BOOL bPrivilegeEnabled = EnablePriv(SE_DEBUG_NAME, hImpersonationToken, &tp);

		// DBGHELP.DLL is not thread safe
		EnterCriticalSection(pCS);
		bRet = pDumpFunction(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithDataSegs, &stInfo, NULL, NULL);
		LeaveCriticalSection(pCS);

		if(bPrivilegeEnabled)
		{
			// Restore the privilege
			RestorePriv(hImpersonationToken, &tp);
		}
	
		return bRet;
	}

private:
	static BOOL GetImpersonationToken(HANDLE* phToken)
	{
		*phToken = NULL;

		if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, TRUE, phToken))
		{
			if(GetLastError() == ERROR_NO_TOKEN)
			{
				// No impersonation token for the curren thread available - go for the process token
				if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, phToken))
				{
					return FALSE;
				}
			}
			else
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	static BOOL EnablePriv(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld)
	{
		BOOL bOk = FALSE;

		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		bOk = LookupPrivilegeValue( 0, pszPriv, &tp.Privileges[0].Luid);
		if(bOk)
		{
			DWORD cbOld = sizeof(*ptpOld);
			bOk = AdjustTokenPrivileges(hToken, FALSE, &tp, cbOld, ptpOld, &cbOld);
		}
	
		return (bOk && (ERROR_NOT_ALL_ASSIGNED != GetLastError()));
	}

	static BOOL RestorePriv(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld)
	{
		BOOL bOk = AdjustTokenPrivileges(hToken, FALSE, ptpOld, 0, 0, 0);	
		return (bOk && (ERROR_NOT_ALL_ASSIGNED != GetLastError()));
	}

	static HMODULE LocalLoadLibrary(HMODULE hModule, LPCTSTR pszModule)
	{
		HMODULE hDll = NULL;

		// Attempt to load local module first
		TCHAR pszModulePath[MAX_PATH];
		if(GetModuleFileName(hModule, pszModulePath, sizeof(pszModulePath) / sizeof(pszModulePath[0])))
		{
			TCHAR* pSlash = _tcsrchr(pszModulePath, _T('\\'));
			if(0 != pSlash)
			{
				_tcscpy(pSlash + 1, pszModule);
				hDll = ::LoadLibrary(pszModulePath);
			}
		}

		if(NULL == hDll)
		{
			// If not found, load any available copy
			hDll = ::LoadLibrary(pszModule);
		}

		return hDll;
	}

	static BOOL GenerateDumpFilename(LPTSTR pszBuffer, DWORD cbBuffer)
	{
		// Default path is executable root
		TCHAR pszDumpPath[MAX_PATH];
		SYSTEMTIME stTime;
		GetLocalTime(&stTime); 
        if(GetModuleFileName(NULL, pszDumpPath, sizeof(pszDumpPath) / sizeof(pszDumpPath[0])))
          {
            // keep it.
          }
        else
          {
            memset(pszDumpPath, 0, sizeof(pszDumpPath));
          }
		

		// Filename is composed like this, to avoid collisions;
		// <DumpPath>\DUMP-<PID>-<TID>-YYYYMMDD-HHMMSS.dmp
		size_t cbFilename = sizeof(pszDumpPath) / sizeof(pszDumpPath[0]) - 1;
		unsigned iWritten = _sntprintf(pszDumpPath, cbFilename, _T("%s-DUMP-%ld-%ld-%04d%02d%02d-%02d%02d%02d.dmp"), pszDumpPath, GetCurrentProcessId(), GetCurrentThreadId(), stTime.wYear,stTime.wMonth,stTime.wDay,stTime.wHour, stTime.wMinute, stTime.wSecond);
		if(iWritten < 0 || (iWritten + 1) > cbBuffer)
		{
			// Either the local buffer or the output buffer was too small
			return FALSE;
		}

		_tcscpy(pszBuffer, pszDumpPath);

		return TRUE;
	}
};


#endif //__MINIDUMP_H__

