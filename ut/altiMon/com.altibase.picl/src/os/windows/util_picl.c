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
 
#include "util_picl.h"

void initQuery()
{
	if(isInitialized==FALSE)
	{
		PdhOpenQuery(NULL, 0, &TotalCpuQuery);
		PdhAddCounter(TotalCpuQuery, "\\Processor(_Total)\\% Privileged Time", 0, &TotalSysCpuCounter);
		PdhAddCounter(TotalCpuQuery, "\\Processor(_Total)\\% User Time", 0, &TotalUserCpuCounter);		
		isInitialized = TRUE;		
	}
}

void closeQuery()
{	
	PdhCloseQuery(TotalCpuQuery);
}

void getCpuTime(cpu_t *cpu)
{	
	PDH_FMT_COUNTERVALUE TotalSysCpuFmtValue;	
	PDH_FMT_COUNTERVALUE TotalUserCpuFmtValue;	

	PdhCollectQueryData(TotalCpuQuery);

	PdhGetFormattedCounterValue(TotalSysCpuCounter, PDH_FMT_DOUBLE, NULL, &TotalSysCpuFmtValue);
	PdhGetFormattedCounterValue(TotalUserCpuCounter, PDH_FMT_DOUBLE, NULL, &TotalUserCpuFmtValue);
	
	cpu->sysPerc = TotalSysCpuFmtValue.doubleValue;
	cpu->userPerc = TotalUserCpuFmtValue.doubleValue;
}

BOOL GetProcTimes(DWORD aPid, FILETIME* sysTime, FILETIME* userTime)
{
	HANDLE procHandle;
	FILETIME startTime, exitTime;

	procHandle = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, aPid);

	if(!GetProcessTimes(procHandle, &startTime, &exitTime, sysTime, userTime))
	{		
		CloseHandle(procHandle);
		return FALSE;
	}
	else
	{
		CloseHandle(procHandle);
		return TRUE;
	}
}

BOOL GetProcMemInfo(DWORD aPid, SIZE_T* aMemUsage)
{
	HANDLE procHandle;
	PROCESS_MEMORY_COUNTERS pmc;

	procHandle = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, aPid);

	if(procHandle == NULL)
		return FALSE;

	if(GetProcessMemoryInfo(procHandle, &pmc, sizeof(pmc)))
	{
		
        *aMemUsage = pmc.WorkingSetSize / DIV;
		CloseHandle(procHandle);
		return TRUE;		
	}
	else
	{
		CloseHandle(procHandle);
		return FALSE;
	}
}

BOOL getDirUsage(const char* name, dir_usage_t *dirusage)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char dirName[4096];
	int pathLength = strlen(name);
	int restLength = sizeof(dirName)-pathLength-1;
	char *index = dirName;
	char delimiter;	

	//printf("Path Name : %s\n", name);

	strncpy(dirName, name, sizeof(dirName));

	index += pathLength;

	if (strchr(dirName, '/')) {
        delimiter = '/';
    }
    else {
        delimiter = '\\';
    }

	if (dirName[pathLength] != delimiter) {
        *index++ = delimiter;
        pathLength++;
		restLength--;
    }

	dirName[pathLength] = '*';
    dirName[pathLength+1] = '\0';

	//printf("DIR Name : %s\n", dirName);

	hFind = FindFirstFile(dirName, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

	//printf("test : %s\n", dirName);

	do{		
		// Skip "." or ".."
		if (!lstrcmp (FindFileData.cFileName, TEXT(".")) || !lstrcmp (FindFileData.cFileName, TEXT("..")))
		{
			continue;
		}

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			strncpy(index, FindFileData.cFileName, restLength);
			
			getDirUsage(dirName, dirusage);
		}
		
		//printf("test : %s\n", FindFileData.cFileName);

		dirusage->disk_usage +=
            (FindFileData.nFileSizeHigh * (MAXDWORD+1)) +
            FindFileData.nFileSizeLow;

	}while(FindNextFile(hFind, &FindFileData));

	FindClose(hFind);

	return TRUE;
}

BOOL getDeviceList(device_list_t_ *device_list)
{
    char device_name[256];
    char *index = device_name;
    
    if(!GetLogicalDriveStringsA(sizeof(device_name), device_name))
	{
		// If you get extended error information, you can use the GetLastError function.
		return FALSE;
	}

	device_list->number = 0;

    device_list->size = DEVICE_MAX;

    device_list->data = malloc(sizeof(*(device_list->data))*device_list->size);

	while(*index)
	{
		device_t_ *device;
		DWORD flags, sn=0;
        char fsname[1024];
        UINT drive_type = GetDriveType(index);
        int type;

		if(drive_type == DRIVE_FIXED)
		{
			fsname[0] = '\0';

			GetVolumeInformation(index, NULL, 0, &sn, NULL,
                             &flags, fsname, sizeof(fsname));

			/* To skip unformatted partitions */
			if (!sn) {
				while(*index++);
				continue; 
			}
		}
		else
		{
			while(*index++);
			continue;
		}
		
		if (device_list->number >= device_list->size) {

            device_list->data = realloc(device_list->data, sizeof(*(device_list->data))*(device_list->size + DEVICE_MAX));

            device_list->size += DEVICE_MAX;

        }

        device = &device_list->data[device_list->number++];		
		/* Fix BUG-30506 */
		strncpy(device->dir_name, index, sizeof(device->dir_name));
        device->dir_name[2] = '\0';

		/*printf("dirname :%s\n", device->dir_name);*/

		strncpy(device->dev_name, index, sizeof(device->dev_name));
		device->dev_name[2] = '\0';
		
		while(*index++);
	}

	return TRUE;
}

BOOL getDiskIo(const char* rootDirName, disk_load_t* diskLoad)
{
	ULARGE_INTEGER avail, total, free;
	BOOL retValue;
	char dirName[256];

	/* To prevent error if floppy disk is empty */
	UINT errStatus = SetErrorMode(SEM_FAILCRITICALERRORS);

	strncpy(dirName, rootDirName, sizeof(dirName));

	/* Fix BUG-30506 */
	dirName[strlen(rootDirName)] = '\\';
	dirName[strlen(rootDirName)+1] = '\0';

	//printf("dirName in getDiskIO : %s\n", dirName);

	retValue = GetDiskFreeSpaceEx(dirName, &avail, &total, &free);

	/* To restore previous error mode */
    SetErrorMode(errStatus);

	if(retValue==0)
	{
		return FALSE;
	}

	diskLoad->total_bytes = total.QuadPart / 1024;
	diskLoad->free_bytes = free.QuadPart / 1024;
	diskLoad->avail_bytes = avail.QuadPart / 1024;
	diskLoad->used_bytes = diskLoad->total_bytes - diskLoad->free_bytes;

	return TRUE;
}

BOOL GetProcessModule (DWORD dwPID, DWORD dwModuleID, 
     LPMODULEENTRY32 lpMe32, DWORD cbMe32) 
{ 
    BOOL          procRet        = FALSE; 
    BOOL          procFound      = FALSE; 
    HANDLE        hProcSnap		 = NULL; 
    MODULEENTRY32 pe32           = {0}; 
 
    // Take a snapshot of all processes in the system.
    hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID); 
    
    if (hProcSnap == INVALID_HANDLE_VALUE) 
    {
        return (FALSE);
    } 
 
    // Fill the size of the structure before using it. 
    pe32.dwSize = sizeof(MODULEENTRY32);

    // Now walk the snapshot of processes, and
	// display information about each process in turn
    if (Module32First(hProcSnap, &pe32)) 
    { 
        do 
        { 
            if (pe32.th32ProcessID == dwPID) 
            { 
                CopyMemory (lpMe32, &pe32, cbMe32); 
                procFound = TRUE; 
            } 
        } 
        while (!procFound && Module32Next(hProcSnap, &pe32)); 
 
        procRet = procFound;    
                          
    } 
    else 
    {
        procRet = FALSE;
    }
     
    CloseHandle (hProcSnap); 
    return (procRet); 
}

DWORD GetProcessID (const char* aPathName) 
{ 
    HANDLE         hProcSnap = NULL; 
    PROCESSENTRY32 pe32      = {0}; 
	INT retVal = -1;

     
    hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 
      
    pe32.dwSize = sizeof(PROCESSENTRY32); 
      
    if (Process32First(hProcSnap, &pe32)) 
    { 
        DWORD         dwPriorityClass; 
        BOOL          bGotModule = FALSE; 
        MODULEENTRY32 me32       = {0}; 
 
        do 
        { 
            bGotModule = GetProcessModule(pe32.th32ProcessID, 
                pe32.th32ModuleID, &me32, sizeof(MODULEENTRY32)); 

            if (bGotModule) 
            { 
                HANDLE hProcess; 
 
                // Get the actual priority class. 
                hProcess = OpenProcess (PROCESS_ALL_ACCESS, 
                    FALSE, pe32.th32ProcessID); 
                dwPriorityClass = GetPriorityClass (hProcess); 
                CloseHandle (hProcess); 

				if(strncmp(me32.szExePath, aPathName, strlen(aPathName))==0){
					// Print the process's information.
					//printf( "PID\t\t\t%d\n", pe32.th32ProcessID);                
					//printf( "Full Path\t\t%s\n\n", me32.szExePath);
					//printf( "Target Path\t\t%s\n\n", aPathName);
					retVal = pe32.th32ProcessID;
					break;
				}
            } 
        } 
        while (Process32Next(hProcSnap, &pe32)); 
    } 

	// Do not forget to clean up the snapshot object. 
    CloseHandle (hProcSnap); 

	return (retVal); 
}
