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
 * Picl_hpux.c
 *
 *  Created on: 2010. 1. 13
 *      Author: canpc815
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
  struct pst_dynamic ps_dyn;
  float delta; 

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

  if(pstat_getdynamic(&ps_dyn, sizeof(ps_dyn), 1, 0)==-1)
  {
    perror("pstat function getdynamic Error");
  }

  // Native Type
  jlong* mPrevTimeArr = (*env)->GetLongArrayElements(env, mPrevTime, &isCopy);
  
  // Cpu time delta
  delta = ps_dyn.psd_cpu_time[CP_USER] - mPrevTimeArr[CP_USER] +
  	  ps_dyn.psd_cpu_time[CP_SYS] - mPrevTimeArr[CP_SYS] +
	  ps_dyn.psd_cpu_time[CP_NICE] - mPrevTimeArr[CP_NICE] +
	  ps_dyn.psd_cpu_time[CP_WAIT] - mPrevTimeArr[CP_WAIT] + 
	  ps_dyn.psd_cpu_time[CP_IDLE] - mPrevTimeArr[CP_IDLE];

  if(delta==0)
  {
    mSysPerc = 0.0;
    mUserPerc = 0.0;
  }
  else
  {
    mSysPerc = (ps_dyn.psd_cpu_time[CP_SYS] - mPrevTimeArr[CP_SYS]) / delta * 100.00;
    mUserPerc = (ps_dyn.psd_cpu_time[CP_USER] - mPrevTimeArr[CP_USER]) / delta * 100.00;

    mPrevTimeArr[CP_USER] = ps_dyn.psd_cpu_time[CP_USER];
    mPrevTimeArr[CP_SYS] = ps_dyn.psd_cpu_time[CP_SYS];
    mPrevTimeArr[CP_NICE] = ps_dyn.psd_cpu_time[CP_NICE];
    mPrevTimeArr[CP_WAIT] = ps_dyn.psd_cpu_time[CP_WAIT];
    mPrevTimeArr[CP_IDLE] = ps_dyn.psd_cpu_time[CP_IDLE];
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
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcCpu_update
(JNIEnv * env, jobject proccpu_obj, jobject picl_obj, jlong pid)
{
  jfieldID procTable_fid, sys_fid, user_fid;
  
  jmethodID procTable_containsKey_mid;
  jmethodID procTable_get_mid;
  jmethodID procTable_put_mid;
  jmethodID string_valueof_mid;

  jobject mProcTable_obj;

  jlongArray mPrevTime;
  jlong mPrevTimeArr[5];
  jstring mPid;
  jboolean hasPid;
  double mUserPerc;
  double mSysPerc;
  long mUserTime;
  long mSysTime;

  /* pstat information */
  struct pst_status pst;
  struct pst_dynamic psd;
  /* Get a reference to obj's class */
  jclass mProcTable_cls;
  jclass proccpu_cls = (*env)->GetObjectClass(env, proccpu_obj);
  jclass picl_cls = (*env)->GetObjectClass(env, picl_obj);  
  jclass string_cls = (*env)->FindClass(env, "Ljava/lang/String;");

  /* Look for the instance field mPrevTime in cls */
  procTable_fid = (*env)->GetFieldID(env, picl_cls, "mProcTable", "Ljava/util/Properties;");
  sys_fid = (*env)->GetFieldID(env, proccpu_cls, "mSysPerc", "D");
  user_fid = (*env)->GetFieldID(env, proccpu_cls, "mUserPerc", "D");

  if(procTable_fid == NULL || sys_fid == NULL || user_fid == NULL)
  {
    return;// failed to find the field
  }

  /* Get the ProcCpu object member variable */
  mSysPerc = (*env)->GetDoubleField(env, proccpu_obj, sys_fid);
  mUserPerc = (*env)->GetDoubleField(env, proccpu_obj, user_fid);

  if (pstat_getdynamic (&psd, sizeof(psd), (size_t)1, 0) == -1) 
  {
    return;
  }

  if(pstat_getproc(&pst, sizeof(pst), (size_t)0, (int)pid) != -1)
  { 
    mUserTime = pst.pst_utime;
    mSysTime = pst.pst_stime;
    if(mUserTime==0 && mSysTime==0)
    {
      mUserPerc = 0;
      mSysPerc = 0;
    }
    else
    {
      mUserPerc = (double)(100 * pst.pst_pctcpu * ((double)mUserTime / (double)(mUserTime + mSysTime))) / (double)psd.psd_max_proc_cnt;
      mSysPerc = (double)(100 * pst.pst_pctcpu * ((double)mSysTime / (double)(mSysTime + mUserTime))) / (double)psd.psd_max_proc_cnt;
    }
  }
  (*env)->SetDoubleField(env, proccpu_obj, sys_fid, mSysPerc);
  (*env)->SetDoubleField(env, proccpu_obj, user_fid, mUserPerc);
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
  struct pst_dynamic ps_dyn;
  struct pst_static pstatic;
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

  if(pstat_getdynamic(&ps_dyn, sizeof(ps_dyn), 1, 0)==-1)
  {
    perror("pstat function getdynamic Error");
  }

  /* does not change while system is running */
  pstat_getstatic(&pstatic, sizeof(pstatic), 1, 0);

  // Native Type
  memUsageArr = (*env)->GetLongArrayElements(env, memUsage, &isCopy);

  // Calculate Memory Usage
  // Virtual Memory = Real Memory + Paging Space
  //printf("Total Real Memory Pages : %ld\n", ps_dyn.psd_rm);
  //printf("Active Real Memory Pages : %ld\n", ps_dyn.psd_arm);
  //printf("Total Virtual Memory Pages : %ld\n", ps_dyn.psd_vm);
  //printf("Active Virtual Memory Pages : %ld\n", ps_dyn.psd_avm);
  //printf("Free Memory Pages : %ld\n", ps_dyn.psd_free);
  memUsageArr[MemTotal] = pstatic.physical_memory * (pstatic.page_size / 1024);
  memUsageArr[MemFree] = ps_dyn.psd_free * (pstatic.page_size / 1024);
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
  long memUsed;

  /* pstat information */
  struct pst_status pst;
  struct pst_static pstatic;

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

  if(pstat_getproc(&pst, sizeof(pst), (size_t)0, (int)pid) != -1)
  {
    // Calculate ProcMemory Usage
    //printf("Real pages used for data : %ld KByte\n", pst.pst_dsize * 4);
    //printf("Real pages used for text : %ld KByte\n", pst.pst_tsize * 4);
    //printf("Real pages used for stack : %ld KByte\n", pst.pst_ssize * 4);
    //printf("Real pages used for shared memory : %ld KByte\n", pst.pst_shmsize * 4);
    //printf("Real pages used for memory mapped files : %ld KByte\n", pst.pst_mmsize * 4);
    //printf("Real pages used for U-Area & K-Stack : %ld KByte\n", pst.pst_usize * 4);
    //printf("Real pages used for I/O Device Mapping : %ld KByte\n", pst.pst_iosize * 4);    

    if(pstat_getstatic(&pstatic, sizeof(pstatic), (size_t)1, 0) != -1) {
      memUsed = pst.pst_dsize + pst.pst_tsize + pst.pst_ssize + pst.pst_shmsize + pst.pst_mmsize +
	pst.pst_usize + pst.pst_iosize;
      memUsed*=(pstatic.page_size/1024);
    }
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
  long swapTotal, swapFree;
  int i=0;

  /* pstat information */
  struct pst_swapinfo pst;
  struct pst_static pstatic;

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

  pstat_getstatic(&pstatic, sizeof(pstatic), (size_t)1, 0);

  while (pstat_getswap(&pst, sizeof(pst), 1, i++) > 0) {
    pst.pss_nfpgs *= (pstatic.page_size/1024);  /* nfpgs is in 512 byte blocks */

    if (pst.pss_nblksenabled == 0) {
      pst.pss_nblksenabled = pst.pss_nfpgs;
    }

    swapTotal += pst.pss_nblksenabled;
    swapFree  += pst.pss_nfpgs;
  }

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

  dirusage.disk_usage = 0;

  dirname = (*env)->GetStringUTFChars(env, aPath, NULL);

  getDirUsage(dirname, &dirusage);

  // Save into Java Object
  (*env)->SetLongField(env, disk_obj, dirusage_fid, dirusage.disk_usage);

  (*env)->ReleaseStringUTFChars(env, aPath, dirname);
}

/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    getDeviceListNative
 * Signature: ()[Lcom/altibase/picl/Device;
 */
JNIEXPORT jobjectArray JNICALL Java_com_altibase_picl_DiskLoad_getDeviceListNative
  (JNIEnv *env, jobject device_obj)
{
  int status;
  int i;
  device_list_t_ device_list;
  jobjectArray device_arr;
  jfieldID fids[DEVICE_FIELD_MAX];
  jclass nfs_cls=NULL;
  jclass device_cls = (*env)->FindClass(env, "com/altibase/picl/Device");

  if ((status = getDeviceList(&device_list)) != 1) {
    return NULL;
  }

  fids[DEVICE_DIRNAME] = (*env)->GetFieldID(env, device_cls, "mDirName", "Ljava/lang/String;");
  fids[DEVICE_DEVNAME] = (*env)->GetFieldID(env, device_cls, "mDevName", "Ljava/lang/String;");

  device_arr = (*env)->NewObjectArray(env, device_list.number, device_cls, 0);

  for (i=0; i<device_list.number; i++) {
    device_t_ *device =  &(device_list.data)[i];
    jobject temp_obj;

    temp_obj = (*env)->AllocObject(env, device_cls);
    
    (*env)->SetObjectField(env, temp_obj, fids[DEVICE_DIRNAME], (*env)->NewStringUTF(env, device->dir_name));
    (*env)->SetObjectField(env, temp_obj, fids[DEVICE_DEVNAME], (*env)->NewStringUTF(env, device->dev_name));

    (*env)->SetObjectArrayElement(env, device_arr, i, temp_obj);
  }

  if (device_list.size > 0 ) {
    free(device_list.data);
    device_list.number = 0;
    device_list.size = 0;
  }
  
  return device_arr;
}

void getDevName(const char *mountName, char **devName)
{
  device_list_t_ device_list;
  int i;

  if(getDeviceList(&device_list) == 1)
  {
    for (i=0; i<device_list.number; i++) {
      device_t_ *device = &(device_list.data)[i];
      if(strncmp(device->dir_name, mountName, strlen(mountName))==0)
      {
	*devName = device->dev_name;
	break;
      }
    }
  }

  if (device_list.size > 0 ) {
    free(device_list.data);
    device_list.number = 0;
    device_list.size = 0;
  }
}

int getDiskIo(const char *mountName, disk_load_t *diskLoad)
{
  struct stat buf;
  struct statvfs vbuf;
  char *devName;
  unsigned long numOfBlocks, blockSize;

  getDevName(mountName, &devName);

  if(statvfs(mountName, &vbuf)==0)
  {
    blockSize = vbuf.f_frsize / 512;

    numOfBlocks = vbuf.f_blocks;
    diskLoad->total_bytes = CONVERT_TO_BYTES(numOfBlocks, blockSize);

    numOfBlocks = vbuf.f_bavail;
    diskLoad->avail_bytes = CONVERT_TO_BYTES(numOfBlocks, blockSize);

    numOfBlocks = vbuf.f_bfree;
    diskLoad->free_bytes = CONVERT_TO_BYTES(numOfBlocks, blockSize);

    diskLoad->used_bytes = diskLoad->total_bytes - diskLoad->free_bytes;
  }
  else
  {
    return 0;
  }

  if (stat(devName, &buf) == 0) {

    struct pst_lvinfo lv;

    int retval = pstat_getlv(&lv, sizeof(lv), 0, (int)buf.st_rdev);

    if (retval == 1) {
      diskLoad->numOfReads  = lv.psl_rxfer;
      diskLoad->numOfWrites = lv.psl_wxfer;
      diskLoad->read_bytes  = lv.psl_rcount;
      diskLoad->write_bytes = lv.psl_wcount;
    }
    
  }
  else {
    return 0;
  }
  
  return 1;
}

/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT jobject JNICALL Java_com_altibase_picl_DiskLoad_update
(JNIEnv *env, jobject diskload_obj, jobject picl_obj, jstring mountName)
{
  disk_load_t disk_io;
  int status;
  jclass cls = (*env)->GetObjectClass(env, diskload_obj);
  jclass iops_cls = (*env)->FindClass(env, "com/altibase/picl/Iops");
  jfieldID fids[IO_FIELD_MAX];
  jobject iops_obj;
  const char* name;
  struct timeval currentTime;

  clock_t currentTick;

  name = (*env)->GetStringUTFChars(env, mountName, 0);

  status = getDiskIo(name, &disk_io);//Get current disk io value
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

  (*env)->ReleaseStringUTFChars(env, mountName, name);

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
  struct pst_status pst[BURST];
  int i, count, pid = -1;
  int idx = 0; // index within the context
  const jbyte *str;
  int pathLength;  
  char long_command [MAX_LENGTH];
  union pstun pu;

  pu.pst_command = long_command;

  str = (*env)->GetStringUTFChars(env, aProcPath, NULL);
  
  if(str == NULL) {
    return -1;
  }
  
  /* loop until count == 0 */
  while ((count = pstat_getproc(pst, sizeof(pst[0]), BURST, idx))> 0) {

    /* got count (max of BURST) this time. Process them */
    for (i = 0; i < count; i++) {
      
      if (pstat(PSTAT_GETCOMMANDLINE, pu, MAX_LENGTH, 1, pst[i].pst_pid) == -1) {
	return -1;
      }

      //printf("pid is %d, command is %s\n", pst[i].pst_pid, pst[i].pst_cmd);
      if(strncmp(pu.pst_command, str, strlen(str))==0)
      {     
	  pid = pst[i].pst_pid;
	  break;
      }
    }

    idx = pst[count-1].pst_idx + 1;
    //printf("idx : %d\n", idx);
  }

  (*env)->ReleaseStringUTFChars(env, aProcPath, str);
 
  return (long)pid;
}
