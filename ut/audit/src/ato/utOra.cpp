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
 
/*******************************************************************************
 * $Id: utOra.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utOra.h>

OCIEnv *utOraConnection::envhp = NULL;

extern "C" {
#if defined(HP_HPUX) || defined(IA64_HP_HPUX)
    SInt _init(void) { return 0; }
    SInt _fini(void) { return 0; }
#endif
    /* global static environment handle */
    // static OCIEnv *envhp = NULL;

    /* static initializer */
    IDE_RC _dbd_initialize(void *)
    {
      /*  
        idlOS::fprintf(stderr," Oracle DBD initialize\n");      
        OCIInitialize ((ub4)( OCI_DEFAULT | OCI_THREADED | OCI_OBJECT), NULL,
                      (dvoid * (*)(dvoid *, size_t)) 0,
                      (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                      (void (*)(dvoid *, dvoid *)) 0 );
       */ 
        IDE_TEST( OCIEnvCreate((OCIEnv **) &utOraConnection::envhp, (ub4) OCI_DEFAULT | OCI_THREADED | OCI_OBJECT,
                               (dvoid *) 0, (dvoid * (*)(dvoid *,size_t)) 0,
                               (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
                               (void (*)(dvoid *, dvoid *)) 0, (size_t) 0, (dvoid **) 0) != OCI_SUCCESS);

        return IDE_SUCCESS;

         IDE_EXCEPTION_END;

        return IDE_FAILURE;
    }
                
    /* static connection initialize */ 
    Connection * _new_connection(dbDriver *dbd)
    {
        return new utOraConnection(dbd);
    }

    ub2  sqlTypeToOracle(SInt type)
    {
        switch (type)
        {
            case SQL_CHAR   :
            case SQL_VARCHAR:
            case SQL_FLOAT  :
            case SQL_NUMERIC:
            case SQL_DECIMAL:
                type = SQLT_STR;
                break;

            case SQL_INTEGER:
            case SQL_SMALLINT:
            case SQL_BIGINT:
                type = SQLT_INT;
                break;

            case SQL_DATE   :
            case SQL_TIME   :
            case SQL_TIMESTAMP:
            case SQL_TYPE_TIMESTAMP:
                //type = SQLT_TIMESTAMP;
                type = SQLT_ODT;
                break;

            case SQL_BINARY :
                type = SQLT_LBI;
                break;

            case SQL_REAL:
            case SQL_DOUBLE:
                type = SQLT_FLT;
                break;

            default :
                type = SQLT_STR;
                break;
        }
        return type;
    }


    SInt oraTypeToSql(ub2 type)
    {
        SInt ret = SQL_VARCHAR;
        switch(type)
        {
            case SQLT_AFC:                                        /* Ansi fixed char */
            case SQLT_AVC:                                          /* Ansi Var char */
            case SQLT_CHR:                         /* (ORANET TYPE) character string */
            case SQLT_VST:                                         /* OCIString type */
            case SQLT_STR:                                 /* zero terminated string */
            case SQLT_VCS:                              /* Variable character string */
            case SQLT_LVC:                                    /* Longer longs (char) */
                ret = SQL_VARCHAR;
                break;

            case SQLT_FLT:                     /* (ORANET TYPE) Floating point number */
            case SQLT_NUM:                            /* (ORANET TYPE) oracle numeric */
            case SQLT_VNU:                          /* NUM with preceding length byte */
                ret = SQL_NUMERIC;
                break;

            case SQLT_LNG:                                                    /* long */
                ret = SQL_BIGINT;
                break;

            case SQLT_ODT:                                            /* OCIDate type */
            case SQLT_DAT:                                   /* date in oracle format */
            case SQLT_TIMESTAMP:                                      /* OCITimeStamp */
                ret = SQL_TYPE_TIMESTAMP;
                break;

            case SQLT_INT:                                   /* (ORANET TYPE) integer */
            case SQLT_UIN:                                        /* unsigned integer */
                ret = SQL_INTEGER;
                break;

            case SQLT_LBI:
            case SQLT_BLOB:                                             /* binary lob */
            case SQLT_BFILEE:                                      /* binary file lob */
            case SQLT_CFILEE:                                   /* character file lob */
                ret = SQL_BINARY;
                break;

            default:
                break;
        }
        return ret;
    }


    IDE_RC atbToOraOCIString (void* aFrom, void* aTo,sb2 &ind)
    {
        ind = ( *(UChar*)aFrom == '\0' )?-1:0;
        if( aFrom != aTo )
        {
//          idlOS::memmove(aTo,aFrom, idlOS::strlen(aFrom) )
            idlOS::strcpy((SChar*)aTo,(SChar*)aFrom);
        }
        return IDE_SUCCESS;
    }


    IDE_RC oraOCIDateToDATETIME(void       * aFrom,
                                void       * aTo,
                                OCIError   * err,
                                OCISession * seshp)
    {
        ORADate              * ociDate;
        SQL_TIMESTAMP_STRUCT   sqlDate; // BUG-25123

        IDE_TEST(aFrom == NULL);
        IDE_TEST(aTo == NULL);

        ociDate = (ORADate *)aFrom;
        idlOS::memset(&sqlDate, 0x00, sizeof(SQL_TIMESTAMP_STRUCT));

        if(ociDate->yy)
        {
            sqlDate.year = ociDate->yy;
            sqlDate.month    = ociDate->mm;
            sqlDate.day      = ociDate->dd;
            sqlDate.hour     = ociDate->ts.hh;
            sqlDate.minute   = ociDate->ts.mi;
            sqlDate.second   = ociDate->ts.ss;

            // OCIDate에는 fraction이 없다
            //sqlDate.fraction = ociDate->ts.fs;  // Oracle use nanosecond
        }
        else
        {
            sqlDate.year = (SShort) -32768;
        }

        *((SQL_TIMESTAMP_STRUCT *)aTo) = sqlDate;

        return IDE_SUCCESS;

        IDE_EXCEPTION_END;

        return IDE_FAILURE;
    }
/*
    {
        OCIError             * errhp = NULL;
        OCIDateTime          * ociDate;
        sb2                    year;
        ub1                    month;
        ub1                    day;
        ub1                    hour;
        ub1                    min;
        ub1                    sec;
        ub4                    fsec;
        SQL_TIMESTAMP_STRUCT   sqlDate; // BUG-25123

        IDE_TEST(aFrom == NULL);
        ociDate = (OCIDateTime *) aFrom;

        IDE_TEST(aTo == NULL);
        idlOS::memset(&sqlDate, 0x00, sizeof(SQL_TIMESTAMP_STRUCT));

        OCIDateTimeGetDate((dvoid *) seshp,
                           err,
                           ociDate,
                           &year,
                           &month,
                           &day);

        OCIDateTimeGetTime((dvoid *) seshp,
                           err,
                           ociDate,
                           &hour,
                           &min,
                           &sec,
                           &fsec);

        if(year)
        {
            sqlDate.year = year;

            sqlDate.month    = month;
            sqlDate.day      = day;
            sqlDate.hour     = hour;
            sqlDate.minute   = min;
            sqlDate.second   = sec;
            sqlDate.fraction = fsec;    // Oracle use nanosecond
        }
        else
        {
            sqlDate.year = (SShort) -32768;
        }

        *((SQL_TIMESTAMP_STRUCT *)aTo) = sqlDate;

        return IDE_SUCCESS;

        IDE_EXCEPTION_END;

        return IDE_FAILURE;
    }
*/

    IDE_RC atbToOraOCIDate(void       * aFrom,
                           void       * aTo,
                           sb2        & ind,
                           OCIError   * err,
                           OCISession * seshp)
    {
        SQL_TIMESTAMP_STRUCT   sqlDate;
        ORADate              * ociDate;

        IDE_TEST(aFrom == NULL);
        sqlDate = *(SQL_TIMESTAMP_STRUCT *)aFrom;    // BUG-20128

        IDE_TEST(aTo == NULL);
        ociDate = (ORADate *)aTo;

        if(sqlDate.year == -32768)
        {
            ind = -1;
        }
        else
        {
            ind = 0;

            ociDate->yy    = sqlDate.year;
            ociDate->mm    = sqlDate.month;
            ociDate->dd    = sqlDate.day;
            ociDate->ts.hh = sqlDate.hour;
            ociDate->ts.mi = sqlDate.minute;
            ociDate->ts.ss = sqlDate.second;

            /* Oracle basically use nanosacond */
            ociDate->ts.fs = sqlDate.fraction;
            ociDate->ts.tz = 0;
        }

        return IDE_SUCCESS;

        IDE_EXCEPTION_END;

        return IDE_FAILURE;
    }
/*
    {
        SQL_TIMESTAMP_STRUCT   sqlDate = *((SQL_TIMESTAMP_STRUCT *) aFrom); // BUG-25123
        OCIDateTime          * ociDate = (OCIDateTime *) aTo;

        if(sqlDate.year == -32768)
        {
            ind = -1;
        }
        else
        {
            ind = 0;

            IDE_TEST(OCIDateTimeConstruct((dvoid *) seshp,
                                          err,
                                          ociDate,
                                          sqlDate.year,
                                          sqlDate.month,
                                          sqlDate.day,
                                          sqlDate.hour,
                                          sqlDate.minute,
                                          sqlDate.second,
                                          sqlDate.fraction,
                                          NULL,
                                          0)
                     != OCI_SUCCESS);
        }

        return IDE_SUCCESS;

        IDE_EXCEPTION_END;
        {
            SChar msgbuf[1024] = {0};
            sb4   errcode = 0;

            OCIErrorGet((dvoid *)   err,
                        (ub4)       1,
                        (OraText *) NULL,
                        &errcode,
                        (OraText*)  msgbuf,
                        (ub4)       sizeof(msgbuf),
                        (ub4)       OCI_HTYPE_ERROR);
            idlOS::fprintf(stderr, "%d >>>>>>> %s\n", errcode, msgbuf);
        }

        return IDE_FAILURE;
    }
*/
}//extern "C"
