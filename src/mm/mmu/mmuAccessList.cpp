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

#include <qci.h>
#include <mmErrorCode.h>
#include <mmuAccessList.h>
#include <mmuProperty.h>

iduLatch  mmuAccessList::mLatch;
idBool    mmuAccessList::mInitialized = ID_FALSE;

/* PROJ-2624 [기능성] MM - 유연한 access_list 관리방법 제공 : 1024로 늘림 */
idBool    mmuAccessList::mIPACLPermit[MM_IP_ACL_MAX_COUNT]; /* ID_TRUE: Permit, ID_FALSE: Deny */
struct in6_addr mmuAccessList::mIPACLAddr[MM_IP_ACL_MAX_COUNT];
SChar     mmuAccessList::mIPACLAddrStr[MM_IP_ACL_MAX_COUNT][MM_IP_ACL_MAX_ADDR_STR];
UInt      mmuAccessList::mIPACLAddrFamily[MM_IP_ACL_MAX_COUNT];
UInt      mmuAccessList::mIPACLMask[MM_IP_ACL_MAX_COUNT];
UInt      mmuAccessList::mIPACLCount;

IDE_RC mmuAccessList::initialize()
{
    IDE_TEST( mLatch.initialize(
                    (SChar *)"MMU_ACCESS_LIST_LATCH",
                    IDU_LATCH_TYPE_NATIVE )
              != IDE_SUCCESS );

    mInitialized = ID_TRUE;

    clear();

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmuAccessList::finalize()
{
    mInitialized = ID_FALSE;

    IDE_TEST( mLatch.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void mmuAccessList::clear()
{
    lock();

    if ( mIPACLCount > 0 )
    {
        /* Nothing To Do */
    }
    else
    {
        mIPACLCount = 0;
    }

    unlock();
}

IDE_RC mmuAccessList::add( idBool            aIPACLPermit,
                           struct in6_addr * aIPACLAddr,
                           SChar           * aIPACLAddrStr,
                           UInt              aIPACLAddrFamily,
                           UInt              aIPACLMask )
{
    idBool  sLocked = ID_FALSE;

    lock();
    sLocked = ID_TRUE;

    IDE_TEST_RAISE( mIPACLCount >= MM_IP_ACL_MAX_COUNT,
                    ERR_ABORT_EXCEEDED_ACCESS_LIST_COUNT_LIMIT );

    mIPACLPermit[mIPACLCount]     = aIPACLPermit;
    mIPACLAddr[mIPACLCount]       = *aIPACLAddr;
    mIPACLAddrFamily[mIPACLCount] = aIPACLAddrFamily;
    mIPACLMask[mIPACLCount]       = aIPACLMask;
    idlOS::snprintf( mIPACLAddrStr[mIPACLCount],
                     MM_IP_ACL_MAX_ADDR_STR,
                     "%s",
                     aIPACLAddrStr);

    mIPACLCount++;

    sLocked = ID_FALSE;
    unlock();

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ABORT_EXCEEDED_ACCESS_LIST_COUNT_LIMIT );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_EXCEEDED_ACCESS_LIST_COUNT_LIMIT ) );
    }
    IDE_EXCEPTION_END;

    if (sLocked == ID_TRUE)
    {
        mmuAccessList::unlock();
    }
    
    return IDE_FAILURE;
}

/* proj-1538 ipv6 */
static idBool bitsIsSame( void* aSrc1, void* aSrc2, UInt aBitLen )
{
    /* ex) aBitLen: 20 => (8 * 2 sByteLen) + 4 sBitsLeft */
    SInt  sByteLen = aBitLen / 8;
    SInt  sBitsLeft = aBitLen % 8;
    UChar sLastByte1 = 0;
    UChar sLastByte2 = 0;
    UChar sBitMask = 0;
    SInt  sBit = 0;

    /* compare in bytes */
    if (sByteLen > 0)
    {
        IDE_TEST_RAISE(idlOS::memcmp( aSrc1, aSrc2, sByteLen ) != 0,
                       LABEL_BIT_DIFF);
    }
    else
    {
        /* Nothing To Do */
    }

    /* compare the last byte in bits */
    sLastByte1 = *( ((SChar*)aSrc1) + sByteLen);
    sLastByte2 = *( ((SChar*)aSrc2) + sByteLen);
    for (sBit = 7; (sBitsLeft > 0) && (sBit >= 0); sBit--)
    {
        sBitMask = 0x01 << sBit;
        IDE_TEST_RAISE(
            (sLastByte1 & sBitMask) != (sLastByte2 & sBitMask),
            LABEL_BIT_DIFF);
        sBitsLeft--;
    }

    return ID_TRUE;

LABEL_BIT_DIFF:
    return ID_FALSE;
}

/* proj-1538 ipv6
 * there are 3 cases which we have to handle.
 *       client addr     acl addr   server mode
 * 1.    ipv4            ipv4       ipv4
 * 2.    v4mapped ipv6   ipv4       ipv6-dual
 * 3.    ipv6            ipv6       ipv6-dual
 */
/**
 * ACL을 검사해 접속을 허용할지 여부를 확인한다.
 *
 * @param aLink    ACL을 검사할 cmiLink
 * @param aAllowed 접속을 허용할지 여부
 * @param aIPACL   ACL 정보.
 *                 aAllowed가 ID_FALSE 일때 해당 규칙 정보를 담는다.
 *                 aAllowed가 ID_TRUE 일 때는 의미 없음.
 *
 * @return 에러가 발생했으면 IDE_FAILURE, 아니면 IDE_SUCCESS
 */
/* latch는 밖에서 잡았다. */
IDE_RC mmuAccessList::checkIPACL( struct sockaddr_storage  * aAddr,
                                  idBool                   * aAllowed,
                                  SChar                   ** aIPACL )
{
    struct sockaddr*         sAddrCommon = NULL ;
    struct sockaddr_in*      sAddr4 = NULL;
    struct sockaddr_in6*     sAddr6 = NULL;
    UInt                     sAddr4Dst   = 0;
    UInt                     sAddr4Entry = 0;
    UInt*                    sUIntPtr    = NULL;
    UInt                     i = 0;
    idBool                   sIsIPv6Client = ID_FALSE;

    *aAllowed = ID_TRUE; /* default: all clients are allowed */
    *aIPACL = NULL;

    /* if no real-entry exist, then the 1st entry is dummy */
    IDE_TEST_CONT(mIPACLCount == 0, LABEL_EMPTY_LIST);

    /* bug-30541: ipv6 code review bug.
     * use sockaddr.sa_family instead of sockaddr_storage.ss_family.
     * because the name is different on AIX 5.3 tl1 as __ss_family
     */
    sAddrCommon = (struct sockaddr*)aAddr;
    /* client is ipv4. it means that server mode is ipv4 only */
    if (sAddrCommon->sa_family == AF_INET)
    {
        sAddr4 = (struct sockaddr_in*)aAddr;
        sAddr4Dst = *((UInt*)&sAddr4->sin_addr);
        sIsIPv6Client = ID_FALSE;
    }
    /* client addr is ipv6 or v4mapped-ipv6 */
    else
    {
        sAddr6 = (struct sockaddr_in6*)aAddr;
        /* if v4mapped-ipv6, we compare it with ipv4s in acl list */
        if (idlOS::in6_is_addr_v4mapped(&(sAddr6->sin6_addr)))
        {
            /* sin6_addr: 16bytes => 4 UInts.
             * ex) ::ffff:127.0.0.1 => extract 127.0.0.1 */
            sUIntPtr = (UInt*)&(sAddr6->sin6_addr);
            sAddr4Dst = *(sUIntPtr + 3);
            sIsIPv6Client = ID_FALSE;
        }
        else
        {
            sIsIPv6Client = ID_TRUE;
        }
    }

    /* fix BUG-28834 IP Access Control List 잘못되었습니다 */
    /* IF BITXOR (BITAND(IP_Packet, mask) , BITAND(address,mask)) */
    /* if ipv4 or v4mapped-ipv6, then sIsIPv6Client is false */
    if (sIsIPv6Client == ID_FALSE)
    {
        for ( i = 0; i < mIPACLCount; i++ )
        {
            if (mIPACLAddrFamily[i] == AF_INET)
            {
                sAddr4Entry = *((UInt*)&mIPACLAddr[i]);

                if ( ((sAddr4Dst & mIPACLMask[i])
                     ^ (sAddr4Entry & mIPACLMask[i])) == 0 )
                {
                    if ( mIPACLPermit[i] == ID_TRUE)
                    {
                        *aAllowed = ID_TRUE;
                        *aIPACL = NULL;
                        break;
                    }
                    else
                    {
                        *aAllowed = ID_FALSE;
                        /* 별도로 기록된 address string을 반환 */
                        *aIPACL = mIPACLAddrStr[i];
                    }
                }
                else
                {
                    /* Nothing To Do */
                }
            }
        }
    }
    /* client: ipv6 addr. it is compared to only ipv6 addrs of list */
    else
    {
        for (i = 0; i < mIPACLCount; i++ )
        {
            if (mIPACLAddrFamily[i] == AF_INET6)
            {
                if (bitsIsSame(&sAddr6->sin6_addr,
                               &(mIPACLAddr[i]),
                               mIPACLMask[i]) == ID_TRUE)
                {
                    if ( mIPACLPermit[i] == ID_TRUE)
                    {
                        *aAllowed = ID_TRUE;
                        *aIPACL = NULL;
                        break;
                    }
                    else
                    {
                        *aAllowed = ID_FALSE;
                        /* 별도로 기록된 address string을 반환 */
                        *aIPACL = mIPACLAddrStr[i];
                    }
                }
                else
                {
                    /* Nothing To Do */
                }
            }
            else
            {
                /* Nothing To Do */
            }
        }
    }

    IDE_EXCEPTION_CONT(LABEL_EMPTY_LIST);

    return IDE_SUCCESS;
}

/* idp::readSPFile()을 참조하여 구현 */

/* ACCESS_LIST_METHOD가 1인 경우는 0일때에 비해
 * access control value를 엄격하게 검사한다.
 */
IDE_RC mmuAccessList::loadAccessList()
{
    SChar            sLineBuf[IDP_MAX_PROP_LINE_SIZE];
    SChar            sMsg[IDP_MAX_PROP_LINE_SIZE];
    FILE           * sFP = NULL;
    idBool           sOpened = ID_FALSE;
    struct in6_addr  sDummyAddr;
    SChar          * sTk = NULL;
    UInt             sLen = 0;
    UInt             sLine = 0;
    UInt             i = 0;
    UInt             j = 0;

    idBool           sPermit[MM_IP_ACL_MAX_COUNT];
    struct in6_addr  sAddr[MM_IP_ACL_MAX_COUNT];
    UInt             sAddrFamily[MM_IP_ACL_MAX_COUNT];
    SChar            sAddrStr[MM_IP_ACL_MAX_COUNT][MM_IP_ACL_MAX_ADDR_STR];
    UInt             sMask[MM_IP_ACL_MAX_COUNT];
    UInt             sCount = 0;

    /* ACCESS_LIST_FILE 프로퍼티에 지정된 이름이 NULL - error */
    IDE_TEST_RAISE( mmuProperty::mIPACLFile[0] == '\0',
                    ERR_ABORT_RELOAD_ACL_NOT_PERMITTED );

    /* open ACCESS LIST FILE */
    sFP = idlOS::fopen( mmuProperty::mIPACLFile, "r" );

    /* open error */
    IDE_TEST_RAISE( sFP == NULL,
                    ERR_ABORT_ACCESS_LIST_FILE_OPEN_ERROR );
    sOpened = ID_TRUE;

    /* 임시변수에 기록한다. */
    i = 0;
    while ( idlOS::idlOS_feof( sFP ) == 0 )
    {
        idlOS::memset( sLineBuf, 0, IDP_MAX_PROP_LINE_SIZE );

        if ( idlOS::fgets( sLineBuf, IDP_MAX_PROP_LINE_SIZE, sFP ) == NULL )
        {
            /* EOF */
            break;
        }
        else
        {
            sLine++;
        }

        /* remove white space */
        idp::eraseWhiteSpace( sLineBuf );

        if ( (sLineBuf[0] == '#') || (sLineBuf[0] == 0) )
        {
            /* comment */
            continue;
        }
        else
        {
            /* Nothing to do */
        }

        IDE_TEST_RAISE( i >= MM_IP_ACL_MAX_COUNT,
                        ERR_ABORT_EXCEEDED_ACCESS_LIST_COUNT_LIMIT );

        /* default setting; initialize */
        sPermit[i] = ID_TRUE;
        idlOS::memset(&sAddr[i], 0x00, ID_SIZEOF(struct in6_addr));
        sAddrFamily[i] = AF_UNSPEC;
        sAddrStr[i][0] = '\0';
        sMask[i] = 0;

        idlOS::memset( &sDummyAddr, 0x00, ID_SIZEOF(struct in6_addr) );

        idlOS::snprintf( sMsg,
                         IDP_MAX_PROP_LINE_SIZE,
                         "%s",
                         sLineBuf );

        /* ACCESS_LIST string에 대한 validation 필요 */
        /* ascii 확인 */
        sLen = idlOS::strlen( sLineBuf );
        for ( j = 0; j < sLen; j++ )
        {
            IDE_TEST_RAISE( ((sLineBuf[j]) & (~0x7F)) != 0,
                            ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
        }

        /* 길이 확인 */
        IDE_TEST_RAISE( sLen > IDP_MAX_PROP_STRING_LEN,
                        ERR_ABORT_INVALID_ACCESS_LIST_VALUE );

        /* First Value, "Permit" or "Deny" */
        sTk = idlOS::strtok( sLineBuf, " \t," );
        /* if no entry in the file, sTk: NULL,  mIPACLCount: 1. */
        IDE_TEST_RAISE( sTk == NULL,
                        ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
        if (idlOS::strcasecmp(sTk, "DENY") == 0)
        {
            sPermit[i] = ID_FALSE;
        }
        else if (idlOS::strcasecmp(sTk, "PERMIT") == 0)
        {
            sPermit[i] = ID_TRUE;
        }
        else
        {
            IDE_RAISE( ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
        }

        /* Second Value, IP Address */
        sTk = idlOS::strtok(NULL, " \t,");
        IDE_TEST_RAISE( sTk == NULL,
                        ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
        /* ipv6 addr includes colons */
        if (idlOS::strchr(sTk, ':'))
        {
            /* inet_pton returns a negative value or 0 if error */
            IDE_TEST_RAISE( idlOS::inet_pton(AF_INET6, sTk, &sAddr[i]) <= 0,
                            ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
            sAddrFamily[i] = AF_INET6;
            idlOS::snprintf( sAddrStr[i],
                             MM_IP_ACL_MAX_ADDR_STR,
                             "%s",
                             sTk );
        }
        /* ipv4 addr */
        else
        {
            /* inet_pton returns a negative value or 0 if error */
            IDE_TEST_RAISE( idlOS::inet_pton(AF_INET, sTk, &sAddr[i]) <= 0,
                            ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
            sAddrFamily[i] = AF_INET;
            idlOS::snprintf( sAddrStr[i],
                             MM_IP_ACL_MAX_ADDR_STR,
                             "%s",
                             sTk );
        }

        /* Third Value, IP mask
         * ipv4: d.d.d.d mask form
         * ipv6: prefix bit mask length by which to compare */
        sTk = idlOS::strtok(NULL, " \t,");
        IDE_TEST_RAISE( sTk == NULL,
                        ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
        if (sAddrFamily[i] == AF_INET)
        {
            /* inet_pton returns a negative value or 0 if error */
            IDE_TEST_RAISE( idlOS::inet_pton( AF_INET, sTk, &sDummyAddr ) <= 0,
                            ERR_ABORT_INVALID_ACCESS_LIST_VALUE );

            sMask[i] = idlOS::inet_addr(sTk);
        }
        else
        {
            sMask[i] = idlOS::atoi(sTk);
            /* max ipv6 addr bits: 128 */
            IDE_TEST_RAISE( sMask[i] > 128, 
                            ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
        }

        ideLog::log( IDE_SERVER_0, "[ACCESS LIST] LOADED : %s", sMsg );
        i++;
    }
    sCount = i;

    sOpened = ID_FALSE;
    IDE_TEST_RAISE( idlOS::fclose( sFP ) != 0,
                    ERR_ABORT_ACCESS_LIST_FILE_CLOSE_ERROR );

    /* 복제 */
    lock();

    for ( i = 0; i < sCount; i++ )
    {
        mIPACLPermit[i]     = sPermit[i];
        mIPACLAddr[i]       = sAddr[i];
        mIPACLAddrFamily[i] = sAddrFamily[i];
        mIPACLMask[i]       = sMask[i];
        idlOS::snprintf( mIPACLAddrStr[i],
                         MM_IP_ACL_MAX_ADDR_STR,
                         "%s",
                         sAddrStr[i]);
    }
    mIPACLCount = sCount;

    unlock();

    /* log */
    ideLog::log( IDE_SERVER_0, "[ACCESS LIST] %"ID_INT32_FMT" LOADED\n", sCount );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ABORT_RELOAD_ACL_NOT_PERMITTED );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_RELOAD_ACL_NOT_PERMITTED ) );
    }
    IDE_EXCEPTION( ERR_ABORT_ACCESS_LIST_FILE_OPEN_ERROR );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_INVALID_ACCESS_LIST_FILE,
                                  "open file error",
                                  mmuProperty::mIPACLFile ) );
    }
    IDE_EXCEPTION( ERR_ABORT_ACCESS_LIST_FILE_CLOSE_ERROR );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_INVALID_ACCESS_LIST_FILE,
                                  "close file error",
                                  mmuProperty::mIPACLFile ) );
    }
    IDE_EXCEPTION( ERR_ABORT_EXCEEDED_ACCESS_LIST_COUNT_LIMIT );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_EXCEEDED_ACCESS_LIST_COUNT_LIMIT ) );
        ideLog::log( IDE_SERVER_0, "[ACCESS LIST] EXCEEDED MAXIMUM LIMIT" );
    }
    IDE_EXCEPTION( ERR_ABORT_INVALID_ACCESS_LIST_VALUE );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_INVALID_ACCESS_LIST_VALUE,
                                  sLine,
                                  sMsg ) );

        /* startup시 error mgr가 로딩되지 않은 상태일 수 있어 추가 기록한다. */
        ideLog::log( IDE_SERVER_0,
                     "[ACCESS LIST] value is not acceptable : "
                     "(Line %"ID_INT32_FMT") %s",
                     sLine,
                     sMsg );
    }
    IDE_EXCEPTION_END;

    if ( sOpened == ID_TRUE )
    {
        idlOS::fclose( sFP );
    }
    else
    {
        /* Nothing To Do */
    }

    return IDE_FAILURE;
}

iduFixedTableColDesc gAccessListColDesc[] =
{
    {
        (SChar *)"ID",
        offsetof(mmuAccessListInfo, mNumber),
        IDU_FT_SIZEOF(mmuAccessListInfo, mNumber),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"OPERATION",
        offsetof(mmuAccessListInfo, mOp),
        IDU_FT_SIZEOF(mmuAccessListInfo, mOp),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"ADDRESS",
        offsetof(mmuAccessListInfo, mAddress),
        MM_IP_ACL_MAX_ADDR_STR,
        IDU_FT_TYPE_VARCHAR,
        mmuAccessList::convertToChar,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"MASK",
        offsetof(mmuAccessListInfo, mMask),
        MM_IP_ACL_MAX_MASK_STR,
        IDU_FT_TYPE_VARCHAR,
        mmuAccessList::convertToChar,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    },
};

iduFixedTableDesc gAccessListDesc =
{
    (SChar *)"X$ACCESS_LIST",
    mmuAccessList::buildAccessListRecord,
    gAccessListColDesc,
    IDU_STARTUP_INIT,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

IDE_RC mmuAccessList::buildAccessListRecord( idvSQL              * /* aStatistics */,
                                             void                * aHeader,
                                             void                * /* aDumpObj */,
                                             iduFixedTableMemory * aMemory )
{
    mmuAccessListInfo      sACLInfo;
    struct in_addr         sAddr;
    idBool                 sLocked = ID_FALSE;
    UInt                   i = 0;

    lockRead();
    sLocked = ID_TRUE;

    for ( i = 0; i < mIPACLCount; i++ )
    {
        sACLInfo.mNumber = i + 1;

        /* Permit/Deny */
        if ( mIPACLPermit[i] == ID_TRUE )
        {
            sACLInfo.mOp = 1;
        }
        else
        {
            sACLInfo.mOp = 0;
        }

        /* Address & Mask */
        if ( mIPACLAddrFamily[i] == AF_INET6 )
        {
            /* IPv6 */
            /* address */
            idlOS::inet_ntop( AF_INET6,
                              (void*)(&mIPACLAddr[i]),
                              sACLInfo.mAddress,
                              MM_IP_ACL_MAX_ADDR_STR );

            /* mask */
            idlOS::snprintf( sACLInfo.mMask,
                             MM_IP_ACL_MAX_MASK_STR,
                             "%"ID_UINT32_FMT"",
                             mIPACLMask[i] );
        }
        else
        {
            IDE_DASSERT( mIPACLAddrFamily[i] == AF_INET );

            /* AF_INET; IPv4 */
            /* address */
            idlOS::inet_ntop( AF_INET,
                              (void*)(&mIPACLAddr[i]),
                              sACLInfo.mAddress,
                              MM_IP_ACL_MAX_ADDR_STR );

            /* mask */
            sAddr.s_addr = mIPACLMask[i];
            idlOS::inet_ntop( AF_INET,
                              (void*)&sAddr,
                              sACLInfo.mMask,
                              MM_IP_ACL_MAX_MASK_STR );
        }

        IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                              aMemory,
                                              (void *)&(sACLInfo) )
                  != IDE_SUCCESS );
    }

    sLocked = ID_FALSE;
    unlock();

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if (sLocked == ID_TRUE)
    {
        mmuAccessList::unlock();
    }

    return IDE_FAILURE;
}

UInt mmuAccessList::convertToChar( void   * /* aBaseObj */,
                                   void   * aMember,
                                   UChar  * aBuf,
                                   UInt     aBufSize )
{
    UInt sSize = 0;

    if ( aMember != NULL )
    {
        sSize = idlOS::strlen( (SChar *)aMember );

        if ( sSize > aBufSize )
        {
            sSize = aBufSize;
        }
        else
        {
            /* Nothing To Do */
        }

        idlOS::memcpy( (void*)aBuf, aMember, sSize );
    }
    else
    {
        /* Nothing To Do */
    }

    return sSize;
}
