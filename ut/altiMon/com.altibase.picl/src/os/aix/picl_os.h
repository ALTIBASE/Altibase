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
 * picl_os.h
 *
 *  Created on: 2010. 1. 13
 *      Author: admin
 */

#include <jni.h>

#ifndef _PICL_OS_H_
#define _PICL_OS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_altibase_picl_Cpu
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Cpu_update
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_altibase_picl_Disk
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Disk_update
  (JNIEnv *, jobject, jobject, jstring);

/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT jobject JNICALL Java_com_altibase_picl_DiskLoad_update
(JNIEnv *, jobject, jobject, jstring);

/*
 * Class:     com_altibase_picl_DiskLoad
 * Method:    getDeviceListNative
 * Signature: ()[Lcom/altibase/picl/Device;
 */
JNIEXPORT jobjectArray JNICALL Java_com_altibase_picl_DiskLoad_getDeviceListNative
  (JNIEnv *, jobject);

/*
 * Class:     com_altibase_picl_Memory
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Memory_update
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_altibase_picl_ProcCpu
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;J)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcCpu_update
  (JNIEnv *, jobject, jobject, jlong);

/*
 * Class:     com_altibase_picl_ProcMemory
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;J)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_ProcMemory_update
  (JNIEnv *, jobject, jobject, jlong);

/*
 * Class:     com_altibase_picl_ProcessFinder
 * Method:    getPid
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_altibase_picl_ProcessFinder_getPid
(JNIEnv *, jclass, jstring);


/*
 * Class:     com_altibase_picl_Swap
 * Method:    update
 * Signature: (Lcom/altibase/picl/Picl;)V
 */
JNIEXPORT void JNICALL Java_com_altibase_picl_Swap_update
(JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif

#endif /* PICL_OS_H_ */
