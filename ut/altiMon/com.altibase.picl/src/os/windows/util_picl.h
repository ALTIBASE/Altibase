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
 
#include <windows.h>
#include <tlhelp32.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include <pdh.h>
#include <psapi.h>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Psapi.lib")

static HQUERY TotalCpuQuery;
static HCOUNTER TotalSysCpuCounter;
static HCOUNTER TotalUserCpuCounter;

static BOOL isInitialized = FALSE;

static DWORD TotalCpuStandardTime = 0;

#define MemFree         0
#define MemUsed         1
#define MemTotal        2

#define CP_USER 0
#define CP_SYS  1
#define CP_NICE 2
#define CP_WAIT 3
#define CP_IDLE 4

#define DIR_NAME_MAX 4096

#define DEVICE_INFO_MAX 256
#define DEVICE_MAX 10

// Use to convert bytes to KB
#define DIV 1024

// Specify the width of the field in which to print the numbers. 
// The asterisk in the format specifier "%*I64d" takes an integer 
// argument and uses it to pad and right justify the number.
#define WIDTH 7

#define NS100_2MSEC(t) ((t) / 10000)

#define FILETIME2MSEC(ft) \
	NS100_2MSEC(((ft.dwHighDateTime << 32) | ft.dwLowDateTime))

#define GETNUMOFCPU(info) info.dwNumberOfProcessors

enum {
  DEVICE_DIRNAME,
  DEVICE_DEVNAME,
    DEVICE_FIELD_MAX
};

enum {
  IO_FIELD_DIRNAME,
  IO_FIELD_READ,
  IO_FIELD_WRITE,
  IO_FIELD_TIME,
  IO_FIELD_TOTAL,
  IO_FIELD_AVAIL,
  IO_FIELD_USED,
  IO_FIELD_FREE,
    IO_FIELD_MAX
};

// Structure of CPU
typedef struct {
	double sysPerc, userPerc;
} cpu_t;

// Structure of Process
typedef struct {
  unsigned long long utime;
  unsigned long long stime;
} proc_cpu_t;

typedef struct {
  long disk_usage;
} dir_usage_t;

typedef struct {
  unsigned long numOfReads;
  unsigned long numOfWrites;
  unsigned long write_bytes;
  unsigned long read_bytes;
  unsigned long total_bytes;
  unsigned long avail_bytes;
  unsigned long used_bytes;
  unsigned long free_bytes;
} disk_load_t;


typedef struct {
  char dir_name[DIR_NAME_MAX];
  char dev_name[DIR_NAME_MAX];
} device_t_;

typedef struct {
  unsigned long number;
  unsigned long size;
  device_t_ *data;
} device_list_t_;
