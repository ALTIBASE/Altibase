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
 * picl_linux.c
 *
 *  Created on: 2010. 1. 13
 *      Author: admin
 */

#include "picl_os.h"
#include "util_picl.h"

/*
 * Class:     com_altibase_picl_Cpu
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Cpu_update
(JNIEnv *env, jobject cpu_obj, jobject picl_obj)
{
	jfieldID prev_fid, sys_fid, user_fid;
	jlongArray mPrevTime;
	float mUserPerc;
	float mSysPerc;

	jboolean isCopy = JNI_TRUE;
	int i, core;

	/* Variable for calculate CPU Percentage */
	unsigned long long delta;
	
	jlong* mPrevTimeArr;

	// Init cpu structure
	cpu_t cpu;
	DWORD currentTime;

	/* Get a reference to obj's class */
	jclass cls = (*env)->GetObjectClass(env, cpu_obj);
	jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);
	
	/* Look for the instance field mPrevTime in cls */
	prev_fid = (*env)->GetFieldID(env, picl_cls, "mPrevTime", "[J");
	sys_fid = (*env)->GetFieldID(env, cls, "mSysPerc", "D");
	user_fid = (*env)->GetFieldID(env, cls, "mUserPerc", "D");

	if(prev_fid == NULL || sys_fid == NULL || user_fid == NULL)
	{
	  return;// failed to find the field
	}

	// JNI Object Type
	mPrevTime = (jlongArray)(*env)->GetObjectField(env, picl_obj, prev_fid);
	mSysPerc = (*env)->GetDoubleField(env, cpu_obj, sys_fid);
	mUserPerc = (*env)->GetDoubleField(env, cpu_obj, user_fid);

	// Native Type
	mPrevTimeArr = (*env)->GetLongArrayElements(env, mPrevTime, &isCopy);

	// Calculate	
	initQuery();
	currentTime = timeGetTime();
	
	// If the interval is lower than 500 milli sec, We can't get right information.
	if(currentTime - TotalCpuStandardTime >= 500) {
		getCpuTime(&cpu);
		TotalCpuStandardTime = currentTime;
	}
	else if(currentTime - TotalCpuStandardTime < 0) 
	{
		// Control the time initializing (When the time initializing is occured, timeGetTime() returns 0)
		TotalCpuStandardTime = currentTime;
		return;
	}
	
	mSysPerc = cpu.sysPerc;
	mUserPerc = cpu.userPerc;

	(*env)->ReleaseLongArrayElements(env, mPrevTime, mPrevTimeArr, 0);

	/* Write the instance field */
	(*env)->SetObjectField(env, picl_obj, prev_fid, mPrevTime);
	(*env)->SetDoubleField(env, cpu_obj, sys_fid, mSysPerc);
	(*env)->SetDoubleField(env, cpu_obj, user_fid, mUserPerc);
}

/*
 * Class:     com_altibase_picl_ProcCpu
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;J)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcCpu_update
(JNIEnv * env, jobject proccpu_obj, jobject picl_obj, jlong pid)
{
	jfieldID sys_fid, user_fid, prev_ctime_fid, prev_utime_fid, prev_stime_fid;	

	double mUserPerc;
	double mSysPerc;

	int i, core, ncpus = 0;

	HANDLE ProcHandle;
	FILETIME sysTime, userTime;
	SYSTEM_INFO info;

	unsigned long long sdelta;
	unsigned long long udelta;
	long long timeDelta;

	unsigned long long prev_utime;
	unsigned long long prev_stime;
	unsigned long long prev_ctime;

	unsigned long long current_utime;
	unsigned long long current_stime;
	unsigned long long current_ctime;

	/* Get a reference to obj's class */
	jclass proccpu_cls = (*env)->GetObjectClass(env, proccpu_obj);
	jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);

	/* Look for the instance field mPrevTime in cls */
	sys_fid = (*env)->GetFieldID(env, proccpu_cls, "mSysPerc", "D");
	user_fid = (*env)->GetFieldID(env, proccpu_cls, "mUserPerc", "D");
	prev_ctime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevCtime", "J");
	prev_utime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevUtime", "J");
	prev_stime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevStime", "J");	
	
	if(sys_fid == NULL ||
	 user_fid == NULL ||
	 prev_ctime_fid == NULL ||
	 prev_utime_fid == NULL ||
	 prev_stime_fid == NULL)
	{
	  return;// failed to find the field
	}

	/* Get the ProcCpu object member variable */
	mSysPerc = (*env)->GetDoubleField(env, proccpu_obj, sys_fid);
	mUserPerc = (*env)->GetDoubleField(env, proccpu_obj, user_fid);
	prev_utime = (*env)->GetLongField(env, proccpu_obj, prev_utime_fid);
	prev_stime = (*env)->GetLongField(env, proccpu_obj, prev_stime_fid);
	prev_ctime = (*env)->GetLongField(env, proccpu_obj, prev_ctime_fid);

	//Calculate
	if(!GetProcTimes((DWORD)pid, &sysTime, &userTime))
	{
		return;
	}
	
	GetSystemInfo(&info);
	ncpus = GETNUMOFCPU(info);

	current_utime = FILETIME2MSEC(userTime);	// current user time
	current_stime = FILETIME2MSEC(sysTime);		// current sys time
	current_ctime = timeGetTime();				// current time in milli sec

	udelta = current_utime - prev_utime;		// Time diff between previous user time and current user time
	sdelta = current_stime - prev_stime;		// Time diff between previous sys time and current sys time
	timeDelta = current_ctime - prev_ctime;		// Time diff between previous time and current time

	if(timeDelta >= 500) {
		mSysPerc = (((double)sdelta / timeDelta) * 100) / ncpus;
		mUserPerc = (((double)udelta / timeDelta) * 100) / ncpus;
		prev_stime = current_stime;
		prev_utime = current_utime;
		prev_ctime = current_ctime;
	}
	else if(timeDelta < 0)
	{
		// Control the time initializing (When the time initializing is occured, timeGetTime() returns 0)
		prev_stime = current_stime;
		prev_utime = current_utime;
		prev_ctime = current_ctime;		
	}
	
	/* Write the instance field */
	(*env)->SetDoubleField(env, proccpu_obj, sys_fid, mSysPerc);
	(*env)->SetDoubleField(env, proccpu_obj, user_fid, mUserPerc);
	(*env)->SetLongField(env, proccpu_obj, prev_utime_fid, prev_utime);
	(*env)->SetLongField(env, proccpu_obj, prev_stime_fid, prev_stime);
	(*env)->SetLongField(env, proccpu_obj, prev_ctime_fid, prev_ctime);	
}

/*
 * Class:     com_altibase_picl_Memory
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Memory_update
(JNIEnv * env, jobject mem_obj, jobject picl_obj)
{
	jfieldID memUsage_fid;
	jlongArray memUsage;
	jlong* memUsageArr;

	jboolean isCopy = JNI_TRUE;

	/* Variable for calculate Memory Usage */
	MEMORYSTATUSEX memInfo;

	/* Get a reference to obj's class */
	jclass mem_cls = (*env)->GetObjectClass(env, mem_obj);
	jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);

	/* Look for the instance field mMemoryUsage in cls */
	memUsage_fid = (*env)->GetFieldID(env, mem_cls, "mMemoryUsage", "[J");

	if(memUsage_fid == NULL)
	{
	  return;// failed to find the field
	}

	memUsage = (jlongArray)(*env)->GetObjectField(env, mem_obj, memUsage_fid);

	// Native Type
	memUsageArr = (*env)->GetLongArrayElements(env, memUsage, &isCopy);

	// Calculate
	memInfo.dwLength = sizeof(memInfo);
	GlobalMemoryStatusEx (&memInfo);
	
	// In kilo bytes
	memUsageArr[MemFree] = memInfo.ullAvailPhys / DIV;
	memUsageArr[MemTotal] = memInfo.ullTotalPhys / DIV;
	memUsageArr[MemUsed] = memUsageArr[MemTotal] - memUsageArr[MemFree];

	// Save into Java Object
	(*env)->ReleaseLongArrayElements(env, memUsage, memUsageArr, 0);
	(*env)->SetObjectField(env, mem_obj, memUsage_fid, memUsage);
}

/*
 * Class:     com_altibase_picl_ProcMemory
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;J)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcMemory_update
(JNIEnv * env, jobject procmem_obj, jobject picl_obj, jlong pid)
{
	jfieldID memUsed_fid;
	SIZE_T memUsed;	

	/* Get a reference to obj's class */
	jclass procmem_cls = (*env)->GetObjectClass(env, procmem_obj);
	jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);

	/* Look for the instance field mMemoryUsage in cls */
	memUsed_fid = (*env)->GetFieldID(env, procmem_cls, "mMemUsed", "J");

	if(memUsed_fid == NULL)
	{
		return;// failed to find the field
	}

	/* Get the ProcCpu object member variable */
	memUsed = (*env)->GetLongField(env, procmem_obj, memUsed_fid);

	// Calculate
	if(!GetProcMemInfo((DWORD)pid, &memUsed))
	{
		return;
	}
	
	// Save into Java Object
	(*env)->SetLongField(env, procmem_obj, memUsed_fid, memUsed);
}

/*
 * Class:     com_altibase_picl_Swap
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Swap_update
(JNIEnv * env, jobject swap_obj, jobject picl_obj)
{
	jfieldID swapTotal_fid, swapFree_fid;
	unsigned long long swapTotal, swapFree;
	int i=0;

	/* Variable for calculate Memory Usage */
	MEMORYSTATUSEX memInfo;

	/* Get a reference to obj's class */
	jclass swap_cls = (*env)->GetObjectClass(env, swap_obj);
	jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);

	/* Look for the instance field in swap_cls */
	swapTotal_fid = (*env)->GetFieldID(env, swap_cls, "mSwapTotal", "J");
	swapFree_fid = (*env)->GetFieldID(env, swap_cls, "mSwapFree", "J");

	if(swapTotal_fid == NULL || swapFree_fid == NULL)
	{
	  return;// failed to find the field
	}

	/* Get the ProcCpu object member variable */
	swapTotal = (*env)->GetLongField(env, swap_obj, swapTotal_fid);
	swapFree = (*env)->GetLongField(env, swap_obj, swapFree_fid);

	// Calculate
	memInfo.dwLength = sizeof(memInfo);
	GlobalMemoryStatusEx (&memInfo);

	swapTotal = memInfo.ullTotalPageFile / DIV;
	swapFree = memInfo.ullAvailPageFile / DIV;

	// Save into Java Object
	(*env)->SetLongField(env, swap_obj, swapTotal_fid, swapTotal);
	(*env)->SetLongField(env, swap_obj, swapFree_fid, swapFree);
}


/*
 * Class:     com_altibase_picl_Disk
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Disk_update
(JNIEnv *env, jobject disk_obj, jobject picl_obj , jstring aPath)
{
	jfieldID dirusage_fid;

	const char *dirname;
	dir_usage_t dirusage;

	/* Get a reference to obj's class */
	jclass disk_cls = (*env)->GetObjectClass(env, disk_obj);

	/* Look for the instance field mMemoryUsage in cls */
	dirusage_fid = (*env)->GetFieldID(env, disk_cls, "mDirUsage", "J");

	dirusage.disk_usage = -1;

	dirname = (*env)->GetStringUTFChars(env, aPath, NULL);

	//Calculate
	getDirUsage(dirname, &dirusage);

	//printf("size : %ld\n", dirusage.disk_usage);

	// Save into Java Object
	(*env)->SetLongField(env, disk_obj, dirusage_fid, dirusage.disk_usage);
	(*env)->ReleaseStringUTFChars(env, aPath, dirname);
}

/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    getDeviceListNative
 * Signature: ()[Lcom/altibase/picl/Device;
 * Description: This function only returns the device list, because we can choose specific device about what we want to know.
 */
JNIEXPORT jobjectArray JNICALL Java_com_altibase_picl_DiskLoad_getDeviceListNative
(JNIEnv *env, jobject device_obj)
{
  int status;
  int i;
  device_list_t_ device_list;
  jobjectArray device_arr = NULL;
  jfieldID fids[DEVICE_FIELD_MAX];
  jclass nfs_cls=NULL;
  jclass device_cls = (*env)->FindClass(env, "com/altibase/picl/Device");


  if (!getDeviceList(&device_list)) {
    return NULL;
  }

  fids[DEVICE_DIRNAME] = (*env)->GetFieldID(env, device_cls, "mDirName", "Ljava/lang/String;");
  fids[DEVICE_DEVNAME] = (*env)->GetFieldID(env, device_cls, "mDevName", "Ljava/lang/String;");

  device_arr = (*env)->NewObjectArray(env, device_list.number, device_cls, 0);

  for (i=0; i<device_list.number; i++) {
    device_t_ *device = &(device_list.data)[i];
    jobject temp_obj;

    temp_obj = (*env)->AllocObject(env, device_cls);

    (*env)->SetObjectField(env, temp_obj, fids[DEVICE_DIRNAME], (*env)->NewStringUTF(env, device->dir_name));
    (*env)->SetObjectField(env, temp_obj, fids[DEVICE_DEVNAME], (*env)->NewStringUTF(env, device->dev_name));

	//printf("dirname : %s\n", device->dir_name);

    (*env)->SetObjectArrayElement(env, device_arr, i, temp_obj);
  }

  if (device_list.size > 0 ) {
    free(device_list.data);
    device_list.number = 0;
    device_list.size = 0;
  }

  return device_arr;
}

/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT jobject JNICALL Java_com_altibase_picl_DiskLoad_update
(JNIEnv *env, jobject diskload_obj, jobject picl_obj, jstring dirname)
{
  disk_load_t disk_io;
  int status;
  jclass cls = (*env)->GetObjectClass(env, diskload_obj);
  jclass iops_cls = (*env)->FindClass(env, "com/altibase/picl/Iops");
  jfieldID fids[IO_FIELD_MAX];
  jobject iops_obj;
  const char* name;
  DWORD currentTime;

  name = (*env)->GetStringUTFChars(env, dirname, 0);

  if(!getDiskIo(name, &disk_io))//Get current disk io value
  {	  
	  return NULL;
  }

  currentTime = timeGetTime();  
  fids[IO_FIELD_DIRNAME] = (*env)->GetFieldID(env, iops_cls, "mDirName", "Ljava/lang/String;");
  fids[IO_FIELD_READ] = (*env)->GetFieldID(env, iops_cls, "mRead", "J");
  fids[IO_FIELD_WRITE] = (*env)->GetFieldID(env, iops_cls, "mWrite", "J");
  fids[IO_FIELD_TIME] = (*env)->GetFieldID(env, iops_cls, "mTime4Sec", "J");
  fids[IO_FIELD_TOTAL] = (*env)->GetFieldID(env, iops_cls, "mTotalSize", "J");
  fids[IO_FIELD_AVAIL] = (*env)->GetFieldID(env, iops_cls, "mAvailSize", "J");
  fids[IO_FIELD_USED] = (*env)->GetFieldID(env, iops_cls, "mUsedSize", "J");
  fids[IO_FIELD_FREE] = (*env)->GetFieldID(env, iops_cls, "mFreeSize", "J");

  iops_obj = (*env)->AllocObject(env, iops_cls);

  (*env)->SetObjectField(env, iops_obj, fids[IO_FIELD_DIRNAME], (*env)->NewStringUTF(env, name));
  //(*env)->SetLongField(env, iops_obj, fids[IO_FIELD_READ], disk_io.numOfReads);
  //(*env)->SetLongField(env, iops_obj, fids[IO_FIELD_WRITE], disk_io.numOfWrites);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_TIME], currentTime);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_TOTAL], disk_io.total_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_AVAIL], disk_io.avail_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_USED], disk_io.used_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_FREE], disk_io.free_bytes);
	
  (*env)->ReleaseStringUTFChars(env, dirname, name);
  return iops_obj;
}

/*
 * Class:     com_altibase_picl_ProcessFinder
 * Method:    getPid
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_altibase_picl_ProcessFinder_getPid
(JNIEnv *env, jclass cls, jstring aProcPath)
{
	LONG pid = -1;
	const jbyte *str;
	
	str = (*env)->GetStringUTFChars(env, aProcPath, NULL);

	if(str == NULL) {
		return -1;
	}

	pid = (LONG)GetProcessID(str);	

	(*env)->ReleaseStringUTFChars(env, aProcPath, str);

	return pid;
}
