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

  /* Variable for calculate CPU Percentage */
  unsigned long long delta;

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
  jlong* mPrevTimeArr = (*env)->GetLongArrayElements(env, mPrevTime, &isCopy);

  // Calculate
  cpu_t cpu;

  getCpuTime(&cpu);

  delta = cpu.user - mPrevTimeArr[CP_USER] + cpu.sys - mPrevTimeArr[CP_SYS] +
    cpu.idle - mPrevTimeArr[CP_IDLE] + cpu.wait - mPrevTimeArr[CP_WAIT] +
    cpu.nice - mPrevTimeArr[CP_NICE];

  if(delta==0)
  {
    mSysPerc = 0.0;
    mUserPerc = 0.0;
      
    mPrevTimeArr[CP_USER] = cpu.user;
    mPrevTimeArr[CP_SYS] = cpu.sys;
    mPrevTimeArr[CP_WAIT] = cpu.wait;
    mPrevTimeArr[CP_IDLE] = cpu.idle;
    mPrevTimeArr[CP_NICE] = cpu.nice;
  }
  else
  {
    mSysPerc = (double)(cpu.sys - mPrevTimeArr[CP_SYS]) / delta * 100.00;
    mUserPerc = (double)(cpu.user - mPrevTimeArr[CP_USER]) / delta * 100.00;

    mPrevTimeArr[CP_USER] = cpu.user;
    mPrevTimeArr[CP_SYS] = cpu.sys;
    mPrevTimeArr[CP_WAIT] = cpu.wait;
    mPrevTimeArr[CP_IDLE] = cpu.idle;
    mPrevTimeArr[CP_NICE] = cpu.nice;
  }

  (*env)->ReleaseLongArrayElements(env, mPrevTime, mPrevTimeArr, 0);

  /* Read the instance field mSysPerc */
  (*env)->SetObjectField(env, picl_obj, prev_fid, mPrevTime);
  (*env)->SetDoubleField(env, cpu_obj, sys_fid, mSysPerc);
  (*env)->SetDoubleField(env, cpu_obj, user_fid, mUserPerc);
}

/*
 * Class:     com_altibase_picl_ProcCpu
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;J)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcCpu_update(
        JNIEnv * env,
        jobject proccpu_obj,
        __attribute__((unused)) jobject picl_obj,
        jlong pid)
{
  jfieldID sys_fid, user_fid, prev_ctime_fid, prev_utime_fid, prev_stime_fid, prev_cutime_fid, prev_cstime_fid;

  double mUserPerc;
  double mSysPerc;

  unsigned long long sdelta;
  unsigned long long udelta;
  unsigned long long cdelta;
  unsigned long long cudelta;
  unsigned long long csdelta;


  unsigned long long prev_utime;
  unsigned long long prev_stime;
  unsigned long long prev_ctime;
  unsigned long long prev_cutime;
  unsigned long long prev_cstime;

  unsigned long long current_utime;
  unsigned long long current_stime;

  unsigned long long CURRENT_CP_USER = 0;
  unsigned long long CURRENT_CP_SYS = 0;
  unsigned long long CURRENT_CP_WAIT = 0;
  unsigned long long CURRENT_CP_IDLE = 0;


  /* Get a reference to obj's class */
  jclass proccpu_cls = (*env)->GetObjectClass(env, proccpu_obj);

  /* Look for the instance field mPrevTime in cls */
  sys_fid = (*env)->GetFieldID(env, proccpu_cls, "mSysPerc", "D");
  user_fid = (*env)->GetFieldID(env, proccpu_cls, "mUserPerc", "D");
  prev_ctime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevCtime", "J");
  prev_utime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevUtime", "J");
  prev_stime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevStime", "J");

  prev_cutime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevCUtime", "J");
  prev_cstime_fid = (*env)->GetFieldID(env, proccpu_cls, "mPrevCStime", "J");

  if(sys_fid == NULL ||
     user_fid == NULL ||
     prev_ctime_fid == NULL ||
     prev_utime_fid == NULL ||
     prev_stime_fid == NULL ||
     prev_cstime_fid == NULL ||
     prev_cutime_fid == NULL)
    {
      return;// failed to find the field
    }

  /* Get the ProcCpu object member variable */
  mSysPerc = (*env)->GetDoubleField(env, proccpu_obj, sys_fid);
  mUserPerc = (*env)->GetDoubleField(env, proccpu_obj, user_fid);
  prev_utime = (*env)->GetLongField(env, proccpu_obj, prev_utime_fid);
  prev_stime = (*env)->GetLongField(env, proccpu_obj, prev_stime_fid);
  prev_ctime = (*env)->GetLongField(env, proccpu_obj, prev_ctime_fid);
  prev_cutime = (*env)->GetLongField(env, proccpu_obj, prev_cutime_fid);
  prev_cstime = (*env)->GetLongField(env, proccpu_obj, prev_cstime_fid);

  //Calculate
  proc_cpu_t proc_cpu;
  cpu_t cpu;

  proc_cpu.utime = -1;
  proc_cpu.stime = -1;

  getCpuTime(&cpu);
  
  if(getProcCpuMtx(&proc_cpu, pid) == 0)
  {
    return;
  }

  if(pid==-1)
  {
    return;
  }

  current_utime = proc_cpu.utime;
  current_stime = proc_cpu.stime;

  CURRENT_CP_USER = cpu.user;
  CURRENT_CP_SYS = cpu.sys;
  CURRENT_CP_IDLE = cpu.idle;
  CURRENT_CP_WAIT = cpu.wait;

  udelta = current_utime - prev_utime;
  sdelta = current_stime - prev_stime;

  cdelta = CURRENT_CP_USER + CURRENT_CP_SYS + CURRENT_CP_WAIT + CURRENT_CP_IDLE - prev_ctime;

  cudelta = CURRENT_CP_USER - prev_cutime;
  csdelta = CURRENT_CP_SYS - prev_cstime;

  if(cudelta < udelta)
  {
    udelta = cudelta;
  }

  if(csdelta < sdelta)
  {
    sdelta = csdelta;
  }

  if(cdelta!=0){
    mSysPerc = 100.0 * ((double)sdelta) / cdelta;
    mUserPerc = 100.0 * ((double)udelta) / cdelta;
  }
  else
  {
    mSysPerc = 0.0;
    mUserPerc = 0.0;
  }

  prev_ctime = CURRENT_CP_USER + CURRENT_CP_SYS + CURRENT_CP_WAIT + CURRENT_CP_IDLE;
  prev_cutime = CURRENT_CP_USER;
  prev_cstime = CURRENT_CP_SYS;
  prev_utime = current_utime;
  prev_stime = current_stime;

  (*env)->SetDoubleField(env, proccpu_obj, sys_fid, mSysPerc);
  (*env)->SetDoubleField(env, proccpu_obj, user_fid, mUserPerc);
  (*env)->SetLongField(env, proccpu_obj, prev_utime_fid, prev_utime);
  (*env)->SetLongField(env, proccpu_obj, prev_stime_fid, prev_stime);
  (*env)->SetLongField(env, proccpu_obj, prev_ctime_fid, prev_ctime);
  (*env)->SetLongField(env, proccpu_obj, prev_cstime_fid, prev_cstime);
  (*env)->SetLongField(env, proccpu_obj, prev_cutime_fid, prev_cutime);
}

/*
 * Class:     com_altibase_picl_Memory
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Memory_update(
        JNIEnv * env,
        jobject mem_obj,
        __attribute__((unused)) jobject picl_obj)
{
  jfieldID memUsage_fid;
  jlongArray memUsage;
  jlong* memUsageArr;

  jboolean isCopy = JNI_TRUE;

  /* Variable for calculate Memory Usage */

  /* Get a reference to obj's class */
  jclass mem_cls = (*env)->GetObjectClass(env, mem_obj);

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
  char memInfo[BUFSIZ];

  memInfo[0] = '\0'; /* BUG-43351 Uninitialized Variable */
  if(convertName2Content(TOTAL_MEMINFO, memInfo, sizeof(memInfo)) != 1)
  {
    return;
  }

  memUsageArr[MemTotal] = getMem(memInfo, GET_PARAM_MEM(PARAM_MEMTOTAL)) / 1024;
  memUsageArr[MemFree] = getMem(memInfo, GET_PARAM_MEM(PARAM_MEMFREE)) / 1024;
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
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcMemory_update(
        JNIEnv * env,
        jobject procmem_obj,
        __attribute__((unused)) jobject picl_obj,
        jlong pid)
{
  jfieldID memUsed_fid;
  long memUsed;

  /* Get a reference to obj's class */
  jclass procmem_cls = (*env)->GetObjectClass(env, procmem_obj);

  /* Look for the instance field mMemoryUsage in cls */
  memUsed_fid = (*env)->GetFieldID(env, procmem_cls, "mMemUsed", "J");

  if(memUsed_fid == NULL)
  {
    return;// failed to find the field
  }

  /* Get the ProcCpu object member variable */
  memUsed = (*env)->GetLongField(env, procmem_obj, memUsed_fid);

  // Calculate
  char procMemInfo[BUFSIZ], *index=procMemInfo;

  if(getProcString(procMemInfo, sizeof(procMemInfo), pid, MEM_STATM, sizeof(MEM_STATM)-1) != 1)
  {
    return;
  }
  // matric is byte
  EXTRACTDECIMALULL(index);  //skip other mem info
  memUsed = EXTRACTDECIMALULL(index) * getpagesize() / 1024;

  // Save into Java Object
  (*env)->SetLongField(env, procmem_obj, memUsed_fid, memUsed);
}

/*
 * Class:     com_altibase_picl_Swap
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Swap_update(
        JNIEnv * env,
        jobject swap_obj,
        __attribute__((unused)) jobject picl_obj)
{
  jfieldID swapTotal_fid, swapFree_fid;
  unsigned long long swapTotal, swapFree;

  /* Get a reference to obj's class */
  jclass swap_cls = (*env)->GetObjectClass(env, swap_obj);

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
  char buff[BUFSIZ];

  buff[0] = '\0'; /* BUG-43351 Uninitialized Variable */
  if(convertName2Content(TOTAL_SWAP, buff, sizeof(buff)) != 1)
  {
    return;
  }

  swapTotal = getMem(buff, GET_PARAM_MEM(PARAM_SWAP_TOTAL)) / 1024;
  swapFree = getMem(buff, GET_PARAM_MEM(PARAM_SWAP_FREE)) / 1024;

  // Save into Java Object
  (*env)->SetLongField(env, swap_obj, swapTotal_fid, swapTotal);
  (*env)->SetLongField(env, swap_obj, swapFree_fid, swapFree);
}

/*
 * Class:     com_altibase_picl_Disk
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Disk_update(
        JNIEnv *env,
        jobject disk_obj,
        __attribute__((unused)) jobject picl_obj,
        jstring aPath)
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

  getDirUsage(dirname, &dirusage);

  // Save into Java Object
  (*env)->SetLongField(env, disk_obj, dirusage_fid, dirusage.disk_usage);
}

/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    getDeviceListNative
 * Signature: ()[Lcom/altibase/picl/Device;
 * Description: This function only returns the device list, because we can choose specific device about what we want to know.
 */
JNIEXPORT jobjectArray JNICALL Java_com_altibase_picl_DiskLoad_getDeviceListNative(
        JNIEnv *env,
        __attribute__((unused)) jobject device_obj)
{
  int status;
  int i;
  device_list_t_ device_list;
  jobjectArray device_arr;
  jfieldID fids[DEVICE_FIELD_MAX];
  jclass device_cls = (*env)->FindClass(env, "com/altibase/picl/Device");

  /* BUG-43351 Uninitialized Variable */
  device_list.number = 0;
  device_list.size = 0;
  device_list.data = NULL;

  if ((status = getDeviceList(&device_list)) != 1) {
    return NULL;
  }

  fids[DEVICE_DIRNAME] = (*env)->GetFieldID(env, device_cls, "mDirName", "Ljava/lang/String;");
  fids[DEVICE_DEVNAME] = (*env)->GetFieldID(env, device_cls, "mDevName", "Ljava/lang/String;");

  device_arr = (*env)->NewObjectArray(env, device_list.number, device_cls, 0);

  for (i=0; i<(long)device_list.number; i++) {
    device_t_ *device = &(device_list.data)[i];
    jobject temp_obj;

    temp_obj = (*env)->AllocObject(env, device_cls);

    (*env)->SetObjectField(env, temp_obj, fids[DEVICE_DIRNAME], (*env)->NewStringUTF(env, device->dir_name));
    (*env)->SetObjectField(env, temp_obj, fids[DEVICE_DEVNAME], (*env)->NewStringUTF(env, device->dev_name));

    (*env)->SetObjectArrayElement(env, device_arr, i, temp_obj);
  }

  if (device_list.size > 0 ) {
    if (device_list.data != NULL) { /* BUG-43351 Free Null Pointer */
      free(device_list.data);
    }
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
JNIEXPORT jobject JNICALL Java_com_altibase_picl_DiskLoad_update(
        JNIEnv *env,
        __attribute__((unused)) jobject diskload_obj,
        __attribute__((unused)) jobject picl_obj,
        jstring dirname)
{
  disk_load_t disk_io;
  jclass iops_cls = (*env)->FindClass(env, "com/altibase/picl/Iops");
  jfieldID fids[IO_FIELD_MAX];
  jobject iops_obj;
  const char* name;
  struct timeval currentTime;

  name = (*env)->GetStringUTFChars(env, dirname, 0);

  /* BUG-43351 Uninitialized Variable */
  disk_io.numOfReads = 0;
  disk_io.numOfWrites = 0;
  disk_io.write_bytes = 0;
  disk_io.read_bytes = 0;
  disk_io.total_bytes = 0;
  disk_io.avail_bytes = 0;
  disk_io.used_bytes = 0;
  disk_io.free_bytes = 0;

  getDiskIo(name, &disk_io);//Get current disk io value
  gettimeofday(&currentTime, NULL);
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
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_READ], disk_io.numOfReads);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_WRITE], disk_io.numOfWrites);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_TIME], currentTime.tv_sec);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_TOTAL], disk_io.total_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_AVAIL], disk_io.avail_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_USED], disk_io.used_bytes);
  (*env)->SetLongField(env, iops_obj, fids[IO_FIELD_FREE], disk_io.free_bytes);

  return iops_obj;
}


/*
 * Class:     com_altibase_picl_ProcessFinder
 * Method:    getPid
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_altibase_picl_ProcessFinder_getPid(
        JNIEnv *env,
        __attribute__((unused)) jclass cls,
        jstring aProcPath)
{
  DIR *procDir;                 // A variable to point directory /proc/pid
  struct dirent *entry;         // A structure to select specific file to use I-node
  struct stat fileStat;         // A structure to store file information

  int pid;
  int status = 0;
  char cmdLine[256];
  char tempPath[256];
  const jbyte *str;

  int pathLength;

  str = (jbyte *) ((*env)->GetStringUTFChars(env, aProcPath, NULL));

  if(str == NULL) {
    return -1;
  }

  /* BUG-43351 Null Pointer Dereference */
  if ( (procDir = opendir("/proc")) == NULL )
  {
      return -1;
  }

  while((entry = readdir(procDir)) != NULL ) 
  {
    sprintf(tempPath, "/proc/%s", entry->d_name);
    if (lstat(tempPath, &fileStat) < 0)
    {
      continue;
    }

    if(!S_ISDIR(fileStat.st_mode))
    {
      continue;
    }

    pid = atoi(entry->d_name);

    if(pid <= 0)
    {
      continue;
    }

    sprintf(tempPath, "/proc/%d/cmdline", pid);
    
    memset(cmdLine, 0, sizeof(cmdLine));
    if(getCmdLine(tempPath, cmdLine)==0)
    {
      continue;
    }

    pathLength = strlen((char *)str);

    if(strncmp((char*)str, cmdLine, pathLength)==0)
      {
	status = 1;
	break;
      }
  }

  closedir(procDir);

  (*env)->ReleaseStringUTFChars(env, aProcPath, (char*)str);

  if(status==1)
  {
    return (long)pid;
  }
  else
  {
    return -1;
  }
}
