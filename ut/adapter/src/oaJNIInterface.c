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
 * Copyright 2016, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#include <ace.h>
#include <acl.h>
#include <aciTypes.h>

#include <oaContext.h>
#include <oaMsg.h>
#include <oaJNIInterface.h>
#include <oaLog.h>

static JavaVM  * gJvm = NULL;
static JNIEnv  * gEnv = NULL;

#define MAX_LOCAL_FRAME_SIZE      (10)

oaJNIMethodDesc gJNIMethodDesc[] =
{
    {
        NULL, NULL,
        (acp_char_t *)"java/util/Properties",
        (acp_char_t *)"<init>", 
        (acp_char_t *)"()V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/util/Properties",
        (acp_char_t *)"put", 
        (acp_char_t *)"(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/lang/ClassLoader",
        (acp_char_t *)"getSystemClassLoader", 
        (acp_char_t *)"()Ljava/lang/ClassLoader;",
        ACP_TRUE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/lang/Thread",
        (acp_char_t *)"currentThread", 
        (acp_char_t *)"()Ljava/lang/Thread;",
        ACP_TRUE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/lang/Thread",
        (acp_char_t *)"setContextClassLoader", 
        (acp_char_t *)"(Ljava/lang/ClassLoader;)V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/lang/Class",
        (acp_char_t *)"forName", 
        (acp_char_t *)"(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;",
        ACP_TRUE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/DriverManager",
        (acp_char_t *)"getConnection", 
        (acp_char_t *)"(Ljava/lang/String;Ljava/util/Properties;)Ljava/sql/Connection;",
        ACP_TRUE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/PreparedStatement",
        (acp_char_t *)"setString", 
        (acp_char_t *)"(ILjava/lang/String;)V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/PreparedStatement",
        (acp_char_t *)"setTimestamp", 
        (acp_char_t *)"(ILjava/sql/Timestamp;)V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/PreparedStatement",
        (acp_char_t *)"addBatch", 
        (acp_char_t *)"()V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/PreparedStatement",
        (acp_char_t *)"clearParameters", 
        (acp_char_t *)"()V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/PreparedStatement",
        (acp_char_t *)"execute", 
        (acp_char_t *)"()Z",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/PreparedStatement",
        (acp_char_t *)"executeBatch", 
        (acp_char_t *)"()[I",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/PreparedStatement",
        (acp_char_t *)"clearBatch", 
        (acp_char_t *)"()V",
        ACP_FALSE
    }, 
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/PreparedStatement",
        (acp_char_t *)"close", 
        (acp_char_t *)"()V",
        ACP_FALSE
    }, 
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/Timestamp",
        (acp_char_t *)"valueOf",
        (acp_char_t *)"(Ljava/lang/String;)Ljava/sql/Timestamp;",
        ACP_TRUE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/Connection",
        (acp_char_t *)"setAutoCommit", 
        (acp_char_t *)"(Z)V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/Connection",
        (acp_char_t *)"prepareStatement", 
        (acp_char_t *)"(Ljava/lang/String;)Ljava/sql/PreparedStatement;",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/Connection",
        (acp_char_t *)"commit", 
        (acp_char_t *)"()V",
        ACP_FALSE
    },  
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/Connection",
        (acp_char_t *)"rollback", 
        (acp_char_t *)"()V",
        ACP_FALSE
    }, 
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/Connection",
        (acp_char_t *)"close", 
        (acp_char_t *)"()V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/nio/charset/Charset",
        (acp_char_t *)"forName", 
        (acp_char_t *)"(Ljava/lang/String;)Ljava/nio/charset/Charset;",
        ACP_TRUE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/lang/String",
        (acp_char_t *)"<init>", 
        (acp_char_t *)"([BLjava/lang/String;)V",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/lang/Throwable",
        (acp_char_t *)"toString", 
        (acp_char_t *)"()Ljava/lang/String;",
        ACP_FALSE
    },
    {
        NULL, NULL,
        (acp_char_t *)"java/sql/BatchUpdateException",
        (acp_char_t *)"getUpdateCounts", 
        (acp_char_t *)"()[I",
        ACP_FALSE
    },
};

static jclass oaJNIGetClass( acp_sint32_t aMethodNum )
{
    return gJNIMethodDesc[aMethodNum].mClass;
}

static jmethodID oaJNIGetMethodID( acp_sint32_t aMethodNum )
{
    return gJNIMethodDesc[aMethodNum].mMethodID;
}

static void oaJNILogMessage( oaJNIInterfaceHandle * aHandle, 
                             jthrowable             aThrowable,
                             acp_char_t           * aFunctionName )
{
    jstring         sThrowableString; 
    acp_char_t     *sErrorMessage;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    if ( aThrowable == NULL )
    {
        aThrowable = ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    }
    else
    {
        /* Nothing to do */
    }

    if ( aThrowable != NULL ) 
    {
        if ( aHandle->mConflictLoggingLevel >= 1 )
        {
            oaLogMessage( OAM_MSG_JNI_EXCEPTION, aFunctionName );
            
            if ( (*(aHandle->mEnv))->IsInstanceOf( aHandle->mEnv, 
                                                   (jobject)aThrowable, 
                                                   oaJNIGetClass( JNI_METHOD_TROWABLE_TOSTRING ) )
                 == JNI_TRUE )                         
            {
                sThrowableString = ( *(aHandle->mEnv) )->CallObjectMethod( aHandle->mEnv, 
                                                                           aThrowable, 
                                                                           oaJNIGetMethodID( JNI_METHOD_TROWABLE_TOSTRING ) );
                sErrorMessage =  (acp_char_t *)( *(aHandle->mEnv) )->GetStringUTFChars( aHandle->mEnv,
                                                                                        sThrowableString,
                                                                                        NULL /* IsCopy */ );
                oaLogMessageInfo( sErrorMessage ); 

                ( *(aHandle->mEnv) )->ReleaseStringUTFChars( aHandle->mEnv,
                                                             sThrowableString,
                                                             sErrorMessage );
            }
            else
            {
                /* Nothing to do */
            }
        }
        
        ( *(aHandle->mEnv) )->ExceptionClear( aHandle->mEnv );
    }
    else
    {
        /* Nothing to do */
    }

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ;
}

static ace_rc_t oaJNIGetCreatedJavaVM( oaContext * aContext, oaJNIInterfaceHandle * aHandle )
{
    JavaVMOption           sOptions[9];
    JavaVMInitArgs         sVmArgs;    
    JNIEnv               * sEnv = NULL;
    JavaVM               * sJvm = NULL;
    acp_uint32_t           sOptionCount = 0;
    
    
    ACP_STR_DECLARE_DYNAMIC( sOptDriverPath );
    ACP_STR_DECLARE_DYNAMIC( sOptXmx );
    ACP_STR_INIT_DYNAMIC( sOptDriverPath, 1024 + 20, 1024 + 20 );
    ACP_STR_INIT_DYNAMIC( sOptXmx, 128, 128 );

    acpMemSet( &sVmArgs, 0x00, ACI_SIZEOF(sVmArgs) );
    (void)acpStrCpyFormat( &sOptDriverPath, "-Djava.class.path=%s", (char *)acpStrGetBuffer(aHandle->mDriverPath) );
    sOptions[sOptionCount].optionString =  (char *)acpStrGetBuffer( &sOptDriverPath );
    sOptionCount++;
    sOptions[sOptionCount].optionString = (char *)"-D64";
    sOptionCount++;
    
    if ( aHandle->mXmxOpt != 0 )
    {
        (void)acpStrCpyFormat( &sOptXmx, "-Xmx%"ACI_UINT32_FMT"M", aHandle->mXmxOpt );
        sOptions[sOptionCount].optionString = (char *)acpStrGetBuffer( &sOptXmx );
        sOptionCount++;
    }
    else
    {
        /* Nothing to do */
    }

    if ( acpStrCmpCString( aHandle->mJVMOpt, "", ACP_STR_CASE_SENSITIVE ) != 0 )
    {
        sOptions[sOptionCount].optionString = (char *)acpStrGetBuffer( aHandle->mJVMOpt );
        sOptionCount++;
    }
    else
    {
        /* Nothing to do */
    }

#ifdef DEBUG
    ACP_STR_DECLARE_DYNAMIC( sOptDebug );
    ACP_STR_INIT_DYNAMIC( sOptDebug, 128, 128 );

    (void)acpStrCpyFormat( &sOptDebug, "-Xrunjdwp:transport=dt_socket,address=8000,server=y,suspend=n" );
    sOptions[sOptionCount].optionString = (char *)acpStrGetBuffer( &sOptDebug );
    sOptionCount++;

    (void)acpStrCpyFormat( &sOptDebug, "-Dcom.sun.management.jmxremote" );
    sOptions[sOptionCount].optionString = (char *)acpStrGetBuffer( &sOptDebug );
    sOptionCount++;

    (void)acpStrCpyFormat( &sOptDebug, "-Dcom.sun.management.jmxremote.port=8001" );
    sOptions[sOptionCount].optionString = (char *)acpStrGetBuffer( &sOptDebug );
    sOptionCount++;

    (void)acpStrCpyFormat( &sOptDebug, "-Dcom.sun.management.jmxremote.ssl=false" );
    sOptions[sOptionCount].optionString = (char *)acpStrGetBuffer( &sOptDebug );
    sOptionCount++;

    (void)acpStrCpyFormat( &sOptDebug, "-Dcom.sun.management.jmxremote.authenticate=false" );
    sOptions[sOptionCount].optionString = (char *)acpStrGetBuffer( &sOptDebug );
    sOptionCount++;
#endif

    sVmArgs.nOptions = sOptionCount;
    sVmArgs.version  = JNI_VERSION_1_6;
    sVmArgs.options  = sOptions;
    
    if ( gJvm == NULL )
    {
        ACE_TEST_RAISE( JNI_CreateJavaVM( &sJvm,
                                          (void *)&sEnv, 
                                          &sVmArgs )
                        == JNI_ERR, ERROR_JVM_CREATE );
        gEnv = sEnv;
        gJvm = sJvm;
    }
    else
    {
        sEnv = gEnv;
        sJvm = gJvm;
    }

    aHandle->mEnv = sEnv;
    aHandle->mJvm = sJvm;
    
    ACP_STR_FINAL( sOptDriverPath );
    ACP_STR_FINAL( sOptXmx );
    
    return ACE_RC_SUCCESS;
    
    ACE_EXCEPTION( ERROR_JVM_CREATE )
    {
        oaLogMessage( OAM_MSG_JVM_NOT_CREATE );
    }
    ACE_EXCEPTION_END;
     
    if ( sJvm != NULL )
    {
        ( *sJvm )->DestroyJavaVM( sJvm );
        sJvm = NULL;
    }
    else
    {
        /* Nothing to do */
    }
    
    ACP_STR_FINAL( sOptDriverPath );
    ACP_STR_FINAL( sOptXmx );
    
    return ACE_RC_FAILURE;
}

static ace_rc_t oaJNIMethodInitialize( oaContext * aContext, oaJNIInterfaceHandle * aHandle )
{
    acp_sint32_t i = 0;
    
    for ( i = 0 ; i < JNI_METHOD_MAX ; i++ )
    {
        gJNIMethodDesc[i].mClass = ( *(aHandle->mEnv) )->FindClass( aHandle->mEnv, gJNIMethodDesc[i].mClassName );
        ACE_TEST_RAISE( gJNIMethodDesc[i].mClass == NULL, ERR_NOT_FOUND_CLASS );
        
        if ( gJNIMethodDesc[i].mIsStatic == ACP_TRUE )
        {
            gJNIMethodDesc[i].mMethodID = ( *(aHandle->mEnv) )->GetStaticMethodID( aHandle->mEnv, 
                                                                                   gJNIMethodDesc[i].mClass, 
                                                                                   gJNIMethodDesc[i].mMethodName,
                                                                                   gJNIMethodDesc[i].mSig );
        }
        else
        {
            gJNIMethodDesc[i].mMethodID = ( *(aHandle->mEnv) )->GetMethodID( aHandle->mEnv, 
                                                                             gJNIMethodDesc[i].mClass, 
                                                                             gJNIMethodDesc[i].mMethodName,
                                                                             gJNIMethodDesc[i].mSig );
        }
        
        ACE_TEST_RAISE( gJNIMethodDesc[i].mMethodID == NULL, ERR_NOT_FOUND_METHOD );
    }

    return ACE_RC_SUCCESS;
 
    // 개발 시에만 필요, 실제 동작에서는 발생할 일이 없다.
    ACE_EXCEPTION( ERR_NOT_FOUND_CLASS )
    {
        oaLogMessage( OAM_MSG_NOT_FOUND_CLASS, gJNIMethodDesc[i].mClassName );
    }
    ACE_EXCEPTION( ERR_NOT_FOUND_METHOD )
    {
        oaLogMessage( OAM_MSG_NOT_FOUND_METHOD, gJNIMethodDesc[i].mMethodName );
    }
    ACE_EXCEPTION_END;
     
    return ACE_RC_FAILURE;
}

void oaJNIDestroyJAVAVM()
{
    if ( gJvm != NULL ) 
    {
        ( *(gJvm) )->DestroyJavaVM( gJvm );
    }
    else
    {
        /* Nothing to do */
    }
}

ace_rc_t oaJNIInitialize( oaContext * aContext, oaJNIInterfaceHandle * aHandle )
{
    ACE_TEST( oaJNIGetCreatedJavaVM( aContext, aHandle ) != ACE_RC_SUCCESS );

    ACE_TEST( oaJNIMethodInitialize( aContext, aHandle ) != ACE_RC_SUCCESS );

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );
    
    return ACE_RC_SUCCESS;
 
    ACE_EXCEPTION_END;

    /* DestroyJavaVM은 Adapter 완전 종료 시 한번만 한다.
       그래서 여기에서는 처리 하지 않는다. */
    
    return ACE_RC_FAILURE;
}

void oaJNIFinalize( oaJNIInterfaceHandle    * aHandle )
{
    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

/* DestroyJavaVM은 Adapter 완전 종료 시 한번만 한다.
   그래서 여기에서는 처리 하지 않는다. */
//    oaJNIDestroyJAVAVM( aHandle );
}

static ace_rc_t oaJNIPutProperties( oaContext             * aContext,
                                    oaJNIInterfaceHandle  * aHandle, 
                                    jobject               * aPropertiesObject )
{
    jobject     sPropertiesObject   = NULL;
    jstring     sKeyString          = NULL;
    jstring     sValueString        = NULL;
    jthrowable  sThrowable          = NULL;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    // new Properies();
    sPropertiesObject = ( *(aHandle->mEnv) )->NewObject( aHandle->mEnv,
                                                         oaJNIGetClass( JNI_METHOD_PROPERTIES_INIT ), 
                                                         oaJNIGetMethodID( JNI_METHOD_PROPERTIES_INIT ), 
                                                        "()V" );
    ACE_TEST_RAISE( sPropertiesObject == NULL, ERR_EXCEPTION );

    // sProps.put("user", "SYS");
    sKeyString = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, "user" ); 
    ACE_TEST_RAISE( sKeyString == NULL, ERR_EXCEPTION );    
    sValueString = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, (char *)acpStrGetBuffer(aHandle->mUser) );
    ACE_TEST_RAISE( sValueString == NULL, ERR_EXCEPTION );  

    (void)( *(aHandle->mEnv) )->CallObjectMethod( aHandle->mEnv, 
                                                  sPropertiesObject, 
                                                  oaJNIGetMethodID( JNI_METHOD_PROPERTIES_PUT ),
                                                  sKeyString,
                                                  sValueString );
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    // sProps.put("password", "manager");
    sKeyString = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, "password" ); 
    ACE_TEST_RAISE( sKeyString == NULL, ERR_EXCEPTION );      
    sValueString = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, (char *)acpStrGetBuffer(aHandle->mPassword) );
    ACE_TEST_RAISE( sValueString == NULL, ERR_EXCEPTION );  

    (void)( *(aHandle->mEnv) )->CallObjectMethod( aHandle->mEnv, 
                                                  sPropertiesObject,
                                                  oaJNIGetMethodID( JNI_METHOD_PROPERTIES_PUT ),
                                                  sKeyString,
                                                  sValueString );
    sThrowable =  ( *aHandle->mEnv )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    sPropertiesObject = ( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, sPropertiesObject );

    *aPropertiesObject = sPropertiesObject;

    return ACE_RC_SUCCESS;
 
    ACE_EXCEPTION( ERR_EXCEPTION)
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIPutProperties" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

static ace_rc_t oaJNILoadDriverClass( oaContext * aContext, oaJNIInterfaceHandle * aHandle )
{
    jobject     sSystemClassLoaderObject    = NULL;
    jobject     sCurrentThreadObject        = NULL;
    jobject     sClassObject                = NULL;
    jstring     sDriverClassString          = NULL;
    jthrowable  sThrowable                  = NULL;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    // call method ClassLoader.getSystemClassLoader
    sSystemClassLoaderObject = ( *(aHandle->mEnv) )->CallStaticObjectMethod( aHandle->mEnv, 
                                                                             oaJNIGetClass( JNI_METHOD_CLASSLOADER_GETSYSTEMCLASSLOADER ), 
                                                                             oaJNIGetMethodID( JNI_METHOD_CLASSLOADER_GETSYSTEMCLASSLOADER ) );
    ACE_TEST_RAISE( sSystemClassLoaderObject == NULL, ERR_EXCEPTION );
     
    // get current thread
    sCurrentThreadObject = ( *(aHandle->mEnv) )->CallStaticObjectMethod( aHandle->mEnv, 
                                                                         oaJNIGetClass( JNI_METHOD_THREAD_CURRENTTHREAD ), 
                                                                         oaJNIGetMethodID( JNI_METHOD_THREAD_CURRENTTHREAD ) );   
    ACE_TEST_RAISE( sCurrentThreadObject == NULL, ERR_EXCEPTION );
    
    // Call setContextClassLoader
    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv, 
                                          sCurrentThreadObject, 
                                          oaJNIGetMethodID( JNI_METHOD_THREAD_SETCONTEXTCLASSLOADER), 
                                          sSystemClassLoaderObject );
    sThrowable = ( *aHandle->mEnv )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );
    
    // Call forName method
    sDriverClassString = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, (char *)acpStrGetBuffer(aHandle->mDriverClass) );
    ACE_TEST_RAISE( sDriverClassString == NULL, ERR_EXCEPTION );
    sClassObject = ( *(aHandle->mEnv) )->CallStaticObjectMethod( aHandle->mEnv, 
                                                                 oaJNIGetClass( JNI_METHOD_CLASS_FORNAME ), 
                                                                 oaJNIGetMethodID( JNI_METHOD_CLASS_FORNAME ),
                                                                 sDriverClassString,
                                                                 JNI_TRUE,
                                                                 sSystemClassLoaderObject );
    ACE_TEST_RAISE( sClassObject == NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
   
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable, 
                         "oaJNILoadDriverClass" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

static ace_rc_t oaJNIGetConnection(  oaContext            * aContext, 
                                     oaJNIInterfaceHandle * aHandle, 
                                     jobject                aPropertiesObject )
{
    jstring     sConnectURLString   = NULL;
    jobject     sConnectionObject      = NULL;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    // call getConnection
    sConnectURLString = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, (char *)acpStrGetBuffer(aHandle->mConnectionUrl) );
    ACE_TEST_RAISE( sConnectURLString == NULL, ERR_EXCEPTION );
    
    sConnectionObject = ( *(aHandle->mEnv) )->CallStaticObjectMethod( aHandle->mEnv, 
                                                                      oaJNIGetClass( JNI_METHOD_DRIVERMANAGER_GETCONNECTION ), 
                                                                      oaJNIGetMethodID( JNI_METHOD_DRIVERMANAGER_GETCONNECTION ),
                                                                      sConnectURLString,
                                                                      aPropertiesObject );
    ACE_TEST_RAISE( sConnectionObject == NULL, ERR_EXCEPTION );

    aHandle->mConnectionObject = ( *(aHandle->mEnv) )->NewGlobalRef( aHandle->mEnv,
                                                                     sConnectionObject );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
 
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         NULL,
                         "oaJNIGetConnection" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );
     
    return ACE_RC_FAILURE;
}

static ace_rc_t oaJNISetAutoCommit( oaContext * aContext, oaJNIInterfaceHandle * aHandle )
{
   jthrowable  sThrowable;

   ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );
    
   // call SetAutoCommit
   ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv, 
                                         aHandle->mConnectionObject, 
                                         oaJNIGetMethodID( JNI_METHOD_CONNECTION_SETAUTOCOMMIT ),
                                         aHandle->mCommitMode );
   sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
   ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

   (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

   return ACE_RC_SUCCESS;

   ACE_EXCEPTION( ERR_EXCEPTION)
   {
       oaJNILogMessage( aHandle,
                        sThrowable,
                        "oaJNISetAutoCommit" );
   }
   ACE_EXCEPTION_END;

   (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );
    
   return ACE_RC_FAILURE;
}

ace_rc_t oaJNIJDBCConnect( oaContext * aContext, oaJNIInterfaceHandle * aHandle )
{
    jobject     sPropertiesObject = NULL;
    acp_bool_t  sIsConnection     = ACP_FALSE;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    ACE_TEST( oaJNIPutProperties( aContext, 
                                  aHandle, 
                                  &sPropertiesObject ) 
              != ACE_RC_SUCCESS );

    ACE_TEST( oaJNILoadDriverClass( aContext, aHandle ) != ACE_RC_SUCCESS );

    ACE_TEST( oaJNIGetConnection( aContext, 
                                  aHandle,
                                  sPropertiesObject )
              != ACE_RC_SUCCESS );
    sIsConnection = ACP_TRUE;

    ACE_TEST( oaJNISetAutoCommit( aContext, aHandle ) != ACE_RC_SUCCESS );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
     
    ACE_EXCEPTION_END;
 
    if ( sIsConnection == ACP_TRUE )
    {
        oaJNIConnectionClose( aHandle );
    }    
    else
    {
        /* Nothing to do */
    }

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );
    
    return ACE_RC_FAILURE;
}

ace_rc_t oaJNIpreparedStatment( oaContext            * aContext,
                                oaJNIInterfaceHandle * aHandle, 
                                acp_char_t           * aSqlQuery, 
                                jobject              * aPrepareStmtObject )
{
    jobject     sPrepareStmtObject  = NULL;
    jobject     sGlobalObject       = NULL;
    jthrowable  sThrowable          = NULL;
    jstring     sSqlQueryString     = NULL;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    sSqlQueryString = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, (char *)aSqlQuery );
    ACE_TEST_RAISE( sSqlQueryString == NULL, ERR_EXCEPTION );
    
    sPrepareStmtObject = ( *(aHandle->mEnv) )->CallObjectMethod( aHandle->mEnv, 
                                                                 aHandle->mConnectionObject, 
                                                                 oaJNIGetMethodID( JNI_METHOD_CONNECTION_PREPARESTATEMENT ), 
                                                                 sSqlQueryString );
    ACE_TEST_RAISE( sPrepareStmtObject == NULL, ERR_EXCEPTION );

    sGlobalObject = ( *(aHandle->mEnv) )->NewGlobalRef( aHandle->mEnv, 
                                                        sPrepareStmtObject );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    *aPrepareStmtObject = sGlobalObject;
         
    return ACE_RC_SUCCESS;
     
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIpreparedStatment" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

ace_rc_t oaJNICheckSupportedCharSet( oaContext            * aContext,
                                     oaJNIInterfaceHandle * aHandle, 
                                     acp_char_t           * aCharSet )
{
    jstring     sCharSetStr    = NULL;
    jobject     sCharSetObject  = NULL;  

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    sCharSetStr = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, (char *)aCharSet ); 
    ACE_TEST_RAISE( sCharSetStr == NULL, ERR_EXCEPTION );
    
    sCharSetObject = ( *(aHandle->mEnv) )->CallStaticObjectMethod( aHandle->mEnv, 
                                                                   oaJNIGetClass( JNI_METHOD_CHARSET_FORNAME ), 
                                                                   oaJNIGetMethodID( JNI_METHOD_CHARSET_FORNAME ), 
                                                                   sCharSetStr );    
    ACE_TEST_RAISE( sCharSetObject == NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
    
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle,
                         NULL, 
                         "oaJNICheckSupportedCharSet" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

static jbyteArray oaJNICstr2JbyteArray( JNIEnv        * aEnv, 
                                        acp_char_t    * aStr, 
                                        acp_sint32_t    aLen )
{
    jbyteArray      sJavaBytes  = NULL;
    
    sJavaBytes = ( *aEnv )->NewByteArray( aEnv, aLen );
    ( *aEnv )->SetByteArrayRegion( aEnv, 
                                   sJavaBytes, 
                                   0, 
                                   aLen, 
                                   (jbyte *)aStr );

    return sJavaBytes;
}

static jstring oaJNINewStringEncoding( JNIEnv     * aEnv, 
                                       jbyteArray   aJavaBytes, 
                                       acp_char_t * aEncoding )
{
    jstring sString = NULL;

    sString = ( *aEnv )->NewObject( aEnv, 
                                    oaJNIGetClass( JNI_METHOD_STRING_INIT ),
                                    oaJNIGetMethodID( JNI_METHOD_STRING_INIT ), 
                                    aJavaBytes, 
                                    ( *aEnv )->NewStringUTF( aEnv, aEncoding ) );
    
    return sString;
}

ace_rc_t oaJNIPreparedStmtSetString( oaContext              * aContext,
                                     oaJNIInterfaceHandle   * aHandle, 
                                     jobject                  aPrepareStmtObject, 
                                     acp_uint16_t             aPosition, 
                                     acp_sint32_t             aLen,
                                     acp_char_t             * aValue,
                                     acp_char_t             * aCharSet )  
{
    jthrowable      sThrowable      = NULL;
    jstring         sStringValue    = NULL;
    jbyteArray      sJbyteArray     = NULL;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    if ( aValue != NULL )
    {
        if ( aCharSet == NULL )
        {
            sStringValue = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, aValue ); 
            ACE_TEST_RAISE( sStringValue == NULL, ERR_EXCEPTION );  
        }
        else
        {
            sJbyteArray = oaJNICstr2JbyteArray( aHandle->mEnv,
                                                aValue,
                                                aLen );
            ACE_TEST_RAISE( sJbyteArray == NULL, ERR_EXCEPTION );

            sStringValue = oaJNINewStringEncoding( aHandle->mEnv, 
                                                   sJbyteArray,
                                                   aCharSet );
            ACE_TEST_RAISE( sStringValue == NULL, ERR_EXCEPTION );
        }
    }
    else
    {
        /* do nothing */
    }
    
    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv, 
                                          aPrepareStmtObject,
                                          oaJNIGetMethodID( JNI_METHOD_PREPARESTMT_SETSTRING ),
                                          aPosition,
                                          sStringValue );
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
     
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIPreparedStmtSetString" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );
     
    return ACE_RC_FAILURE;
}

ace_rc_t oaJNIPreparedStmtSetTimeStamp( oaContext              * aContext,
                                        oaJNIInterfaceHandle   * aHandle, 
                                        jobject                  aPrepareStmtObject, 
                                        acp_uint16_t             aPosition, 
                                        acp_char_t             * aValue )  
{
    jthrowable      sThrowable        = NULL;
    jstring         sStringValue      = NULL;
    jobject         sTimestampObject  = NULL;  

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    sStringValue = ( *(aHandle->mEnv) )->NewStringUTF( aHandle->mEnv, aValue ); 
    ACE_TEST_RAISE( sStringValue == NULL, ERR_EXCEPTION );  

    sTimestampObject = ( *(aHandle->mEnv) )->CallObjectMethod( aHandle->mEnv, 
                                                               aPrepareStmtObject,
                                                               oaJNIGetMethodID( JNI_METHOD_TIMESTAMP_VALUEOF ),
                                                               sStringValue );
    ACE_TEST_RAISE( sTimestampObject == NULL, ERR_EXCEPTION );
    
    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv, 
                                          aPrepareStmtObject,
                                          oaJNIGetMethodID( JNI_METHOD_PREPARESTMT_SETTIMESTAMP ),
                                          aPosition,
                                          sTimestampObject );
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
     
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIPreparedStmtSetTimeStamp" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

ace_rc_t oaJNIPreparedStmtAddBatch( oaContext              * aContext,
                                    oaJNIInterfaceHandle   * aHandle, 
                                    jobject                  aPrepareStmtObject )
{
    jthrowable    sThrowable;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );
 
    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv,
                                          aPrepareStmtObject,
                                          oaJNIGetMethodID( JNI_METHOD_PREPARESTMT_ADDBATCH ) );
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
     
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIPreparedStmtAddBatch" );
    }
    ACE_EXCEPTION_END;

    ( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

ace_rc_t oaJNIPreparedStmtClearParameters( oaContext              * aContext,
                                           oaJNIInterfaceHandle   * aHandle, 
                                           jobject                  aPrepareStmtObject )
{
    jthrowable    sThrowable;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );
 
    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv,
                                          aPrepareStmtObject,
                                          oaJNIGetMethodID( JNI_METHOD_PREPARESTMT_CLEAR_PARAMETERS ) );
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
     
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIPreparedStmtClearParameters" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

ace_rc_t oaJNIPreparedStmtExecute( oaContext             * aContext,
                                   oaJNIInterfaceHandle  * aHandle, 
                                   jobject                 aPrepareStmtObject )
{
    jthrowable    sThrowable;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );
 
    ( *(aHandle->mEnv) )->CallBooleanMethod( aHandle->mEnv,
                                             aPrepareStmtObject,
                                             oaJNIGetMethodID( JNI_METHOD_PREPARESTMT_EXCUTE ) );
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
     
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIPreparedStmtExecute" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE; 
}

static jintArray oaJNIBatchExceptionGetUpdateCounts( oaJNIInterfaceHandle * aHandle )  
{
    jthrowable      sThrowable;
    jstring         sThrowableString; 
    acp_char_t     *sErrorMessage = NULL;
    jintArray       sParamStatusArray = NULL;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );

    if ( sThrowable != NULL ) 
    {
        if ( (*(aHandle->mEnv))->IsInstanceOf( aHandle->mEnv, 
                                                (jobject)sThrowable, 
                                                oaJNIGetClass( JNI_METHOD_TROWABLE_TOSTRING ) )
             == JNI_TRUE )                         
        {
            if ( aHandle->mConflictLoggingLevel >= 1 )
            {    
                sThrowableString = ( *(aHandle->mEnv) )->CallObjectMethod( aHandle->mEnv, 
                                                                           sThrowable, 
                                                                           oaJNIGetMethodID( JNI_METHOD_TROWABLE_TOSTRING ) );
                sErrorMessage =  (acp_char_t *)( *(aHandle->mEnv) )->GetStringUTFChars( aHandle->mEnv,
                                                                                        sThrowableString,
                                                                                        NULL /* IsCopy */ );
                oaLogMessageInfo( sErrorMessage ); 

                ( *(aHandle->mEnv) )->ReleaseStringUTFChars( aHandle->mEnv,
                                                             sThrowableString,
                                                             sErrorMessage );
            }
            else
            {
                /* Nothing to do */
            }
            
            if ( (*(aHandle->mEnv))->IsInstanceOf( aHandle->mEnv, 
                                                   (jobject)sThrowable, 
                                                   oaJNIGetClass( JNI_METHOD_BATCHUPDATEEXCEPTION_GETUPDATECOUNTS ) )
                 == JNI_TRUE )
            {
                
                sParamStatusArray  = (jintArray)( *(aHandle->mEnv) )->CallObjectMethod( aHandle->mEnv, 
                                                                                        (jobject)sThrowable, 
                                                                                        oaJNIGetMethodID( JNI_METHOD_BATCHUPDATEEXCEPTION_GETUPDATECOUNTS ) );
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }
        
        ( *(aHandle->mEnv) )->ExceptionClear( aHandle->mEnv );
    }
    else
    {
        /* Nothing to do */
    }

    sParamStatusArray = ( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, sParamStatusArray );

    return sParamStatusArray;
}

ace_rc_t oaJNIPreparedStmtExecuteBatch( oaContext              * aContext,
                                        oaJNIInterfaceHandle   * aHandle, 
                                        jobject                  aPrepareStmtObject )  
{
    jintArray     sParamStatusArray = NULL;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );
 
    sParamStatusArray = (jintArray)( *(aHandle->mEnv) )->CallObjectMethod( aHandle->mEnv,
                                                                           aPrepareStmtObject,
                                                                           oaJNIGetMethodID( JNI_METHOD_PREPARESTMT_EXECUTEBATCH ) );
    ACE_TEST_RAISE( sParamStatusArray == NULL, ERR_EXCEPTION );
 
    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
    
    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        sParamStatusArray = oaJNIBatchExceptionGetUpdateCounts( aHandle );  

        if ( aHandle->mParamStatusArray != NULL )
        {
            ( *(aHandle->mEnv) )->DeleteGlobalRef( aHandle->mEnv, aHandle->mParamStatusArray );
            aHandle->mParamStatusArray = NULL;
        }
        else
        {
            /* do nothing */
        }

        if ( sParamStatusArray != NULL )
        {
            aHandle->mParamStatusArray = ( *(aHandle->mEnv) )->NewGlobalRef( aHandle->mEnv, 
                                                                             sParamStatusArray );
        }
        else
        {
            /* do nothing */
        }
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE; 
}

void oaJNIPreparedStmtClose( oaJNIInterfaceHandle  * aHandle, 
                             jobject                 sPrepareStmtObject )  
{
    jthrowable  sThrowable = NULL;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );
   
    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv, 
                                          sPrepareStmtObject, 
                                          oaJNIGetMethodID( JNI_METHOD_PREPARESTMT_CLOSE ) );    
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    if ( sThrowable != NULL )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable, 
                         "oaJNIPreparedStmtClose" );
    }
    else
    {
        /* Nothing to do */
    }

    ( *(aHandle->mEnv) )->DeleteGlobalRef( aHandle->mEnv, sPrepareStmtObject );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ;
}

ace_rc_t oaJNIPreparedStmtClearBatch( oaContext              * aContext,
                                      oaJNIInterfaceHandle   * aHandle, 
                                      jobject                  aPrepareStmtObject )
{
    jthrowable    sThrowable;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );
 
    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv,
                                          aPrepareStmtObject,
                                          oaJNIGetMethodID( JNI_METHOD_PREPARESTMT_CLEAR_BATCH ) );
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERR_EXCEPTION )
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIPreparedStmtClearBatch" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

ace_rc_t oaJNIConnectionCommit( oaContext * aContext, oaJNIInterfaceHandle * aHandle )
{
    jthrowable  sThrowable;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv,
                                          aHandle->mConnectionObject, 
                                          oaJNIGetMethodID( JNI_METHOD_CONNECTION_COMMIT ) );
    sThrowable =  ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_SUCCESS;
 
    ACE_EXCEPTION( ERR_EXCEPTION)
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIConnectionCommit" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );

    return ACE_RC_FAILURE;
}

ace_rc_t oaJNIConnectionRollback( oaContext * aContext, oaJNIInterfaceHandle * aHandle )
{
    jthrowable  sThrowable;

    ACE_ASSERT( ( *(aHandle->mEnv) )->PushLocalFrame( aHandle->mEnv, MAX_LOCAL_FRAME_SIZE ) >= 0 );

    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv,
                                          aHandle->mConnectionObject, 
                                          oaJNIGetMethodID( JNI_METHOD_CONNECTION_ROLLBACK ) );
    sThrowable = ( *(aHandle->mEnv) )->ExceptionOccurred( aHandle->mEnv );
    ACE_TEST_RAISE( sThrowable != NULL, ERR_EXCEPTION );

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );
 
    return ACE_RC_SUCCESS;
 
    ACE_EXCEPTION( ERR_EXCEPTION)
    {
        oaJNILogMessage( aHandle, 
                         sThrowable,
                         "oaJNIConnectionRollback" );
    }
    ACE_EXCEPTION_END;

    (void)( *(aHandle->mEnv) )->PopLocalFrame( aHandle->mEnv, NULL );
    
    return ACE_RC_FAILURE;
}

void oaJNIConnectionClose( oaJNIInterfaceHandle * aHandle )
{
    ( *(aHandle->mEnv) )->CallVoidMethod( aHandle->mEnv,
                                          aHandle->mConnectionObject,
                                          oaJNIGetMethodID( JNI_METHOD_CONNECTION_CLOSE ) );

    ( *(aHandle->mEnv) )->DeleteGlobalRef( aHandle->mEnv, aHandle->mConnectionObject );
    aHandle->mConnectionObject = NULL;
}

acp_uint32_t oaJNIGetDMLStatusArrayLength( oaJNIInterfaceHandle * aHandle )  
{
    acp_uint32_t sArrayLength = 0;
    
    if ( aHandle->mParamStatusArray != NULL ) 
    {
        sArrayLength = ( *(aHandle->mEnv) )->GetArrayLength( aHandle->mEnv, aHandle->mParamStatusArray );
    }
    else
    {
        /* Nothing to do */
    }
 
    return sArrayLength ;
}

jint *oaJNIGetDMLStatusArray( oaJNIInterfaceHandle * aHandle )  
{
    jint * sParamStatusArray = NULL;
     
    if ( aHandle->mParamStatusArray != NULL )
    {
        sParamStatusArray = ( *(aHandle->mEnv) )->GetIntArrayElements( aHandle->mEnv, 
                                                                       aHandle->mParamStatusArray , 
                                                                       NULL /* isCopy */ );
    }
    else
    {
        /* Nothing to do */
    }
    return sParamStatusArray;
}

void oaJNIReleaseDMLStatusArray( oaJNIInterfaceHandle * aHandle, jint * aParamStatusArray )  
{
    if ( aHandle->mParamStatusArray != NULL )
    {
        ( *(aHandle->mEnv) )->ReleaseIntArrayElements( aHandle->mEnv,
                                                       aHandle->mParamStatusArray, 
                                                       aParamStatusArray, 
                                                       0 /* copy back and free elems */ );
    }
    else
    {
        /* Nothing to do */
    }
        
}
