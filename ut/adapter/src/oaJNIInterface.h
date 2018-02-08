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
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __OA_JNI_INTERFACE_H__
#define __OA_JNI_INTERFACE_H__


#include <acp.h>
#include <jni.h>

typedef struct oaJNIMethodDesc
{
    jclass          mClass;
    jmethodID       mMethodID;
    acp_char_t     *mClassName;
    acp_char_t     *mMethodName;
    acp_char_t     *mSig;
    acp_bool_t      mIsStatic;
} oaJNIMethodDesc;

typedef struct oaJNIInterfaceHandle
{
    acp_str_t          *mUser;
    acp_str_t          *mPassword;
    acp_str_t          *mDriverPath;
    acp_str_t          *mDriverClass;
    acp_str_t          *mConnectionUrl;
    acp_bool_t          mCommitMode;
    acp_uint32_t        mConflictLoggingLevel;
    acp_uint32_t        mXmxOpt; 
    acp_str_t          *mJVMOpt;
    
    JNIEnv             *mEnv;
    JavaVM             *mJvm;
    
    jintArray           mParamStatusArray;
 
    jobject             mConnectionObject;    
} oaJNIInterfaceHandle;

typedef enum oaJNIMethodType
{
//    JNI_METHOD_UNKNOWN = -1,
    JNI_METHOD_PROPERTIES_INIT = 0,
    JNI_METHOD_PROPERTIES_PUT,
    JNI_METHOD_CLASSLOADER_GETSYSTEMCLASSLOADER,
    JNI_METHOD_THREAD_CURRENTTHREAD,
    JNI_METHOD_THREAD_SETCONTEXTCLASSLOADER,
    JNI_METHOD_CLASS_FORNAME,
    JNI_METHOD_DRIVERMANAGER_GETCONNECTION,
    JNI_METHOD_PREPARESTMT_SETSTRING,
    JNI_METHOD_PREPARESTMT_SETTIMESTAMP,
    JNI_METHOD_PREPARESTMT_ADDBATCH,
    JNI_METHOD_PREPARESTMT_CLEAR_PARAMETERS, // 10
    JNI_METHOD_PREPARESTMT_EXCUTE, 
    JNI_METHOD_PREPARESTMT_EXECUTEBATCH,
    JNI_METHOD_PREPARESTMT_CLEAR_BATCH,
    JNI_METHOD_PREPARESTMT_CLOSE,
    JNI_METHOD_TIMESTAMP_VALUEOF,
    JNI_METHOD_CONNECTION_SETAUTOCOMMIT,
    JNI_METHOD_CONNECTION_PREPARESTATEMENT,
    JNI_METHOD_CONNECTION_COMMIT,
    JNI_METHOD_CONNECTION_ROLLBACK,
    JNI_METHOD_CONNECTION_CLOSE,
    JNI_METHOD_CHARSET_FORNAME,             // 20
    JNI_METHOD_STRING_INIT, 
    JNI_METHOD_TROWABLE_TOSTRING,
    JNI_METHOD_BATCHUPDATEEXCEPTION_GETUPDATECOUNTS,
    JNI_METHOD_MAX
} oaJNIMethodType;

ace_rc_t oaJNIInitialize( oaContext * aContext, oaJNIInterfaceHandle * aHandle );

void oaJNIFinalize( oaJNIInterfaceHandle    * aHandle );

ace_rc_t oaJNIJDBCConnect( oaContext * aContext, oaJNIInterfaceHandle * aHandle );

void oaJNIConnectionClose( oaJNIInterfaceHandle * aHandle );

ace_rc_t oaJNICheckSupportedCharSet( oaContext            * aContext,
                                     oaJNIInterfaceHandle * aHandle, 
                                     acp_char_t           * aCharSet );

ace_rc_t oaJNIPreparedStmtAddBatch( oaContext              * aContext,
                                    oaJNIInterfaceHandle   * aHandle, 
                                    jobject                  aPrepareStmtObject );

ace_rc_t oaJNIPreparedStmtSetString( oaContext              * aContext,
                                     oaJNIInterfaceHandle   * aHandle, 
                                     jobject                  aPrepareStmtObject, 
                                     acp_uint16_t             aPosition, 
                                     acp_sint32_t             aMaxLen,
                                     acp_char_t             * aValue,
                                     acp_char_t             * aNCharSet );

ace_rc_t oaJNIPreparedStmtSetTimeStamp( oaContext              * aContext,
                                        oaJNIInterfaceHandle   * aHandle, 
                                        jobject                  aPrepareStmtObject, 
                                        acp_uint16_t             aPosition, 
                                        acp_char_t             * aValue );

ace_rc_t oaJNIpreparedStatment( oaContext            * aContext,
                                oaJNIInterfaceHandle * aHandle, 
                                acp_char_t           * aSqlQuery, 
                                jobject              * aPrepareStmtObject );

ace_rc_t oaJNIPreparedStmtExecute( oaContext              * aContext,
                                   oaJNIInterfaceHandle   * aHandle, 
                                   jobject                  aPrepareStmtObject );

ace_rc_t oaJNIPreparedStmtClearParameters( oaContext              * aContext,
                                           oaJNIInterfaceHandle   * aHandle, 
                                           jobject                  aPrepareStmtObject );

ace_rc_t oaJNIPreparedStmtExecuteBatch( oaContext              * aContext,
                                        oaJNIInterfaceHandle   * aHandle, 
                                        jobject                  aPrepareStmtObject );

void oaJNIPreparedStmtClose( oaJNIInterfaceHandle  * aHandle, 
                             jobject                 sPrepareStmtObject ); 

ace_rc_t oaJNIPreparedStmtClearBatch( oaContext              * aContext,
                                      oaJNIInterfaceHandle   * aHandle, 
                                      jobject                  aPrepareStmtObject );

ace_rc_t oaJNIConnectionCommit( oaContext * aContext, oaJNIInterfaceHandle * aHandle );

ace_rc_t oaJNIConnectionRollback( oaContext * aContext, oaJNIInterfaceHandle * aHandle );

acp_uint32_t oaJNIGetDMLStatusArrayLength( oaJNIInterfaceHandle * aHandle );

jint *oaJNIGetDMLStatusArray( oaJNIInterfaceHandle * aHandle );

void oaJNIReleaseDMLStatusArray( oaJNIInterfaceHandle * aHandle, jint * aParamStatusArray );

void oaJNIDestroyJAVAVM();

#endif /* OAJNIINTERFACE_H_ */
