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
 * $Id: utProperties.h 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/
#ifndef _UTO_PROPERTIES_H_
#define _UTO_PROPERTIES_H_ 1

#include <idp.h>
#include <utdb.h>

/* processing mode */

typedef enum
{
    DIFF  =  0,  // DIFF print difference of rows
    SYNC      ,  // SYNChronize mode
    DUMMY
} pmode_t;

typedef enum
{
    /* Master DML */
    MI = 0,   /* INSERT */
    MU    ,   /* UPDATE */

    /* Slave DML */
    SI    ,   /* INSERT */
    SD    ,   /* DELETE */
    SU    ,   /* UPDATE */

    DML_MAX
} dml_t;


struct utTableProp
{
    utTableProp * next;

    SChar  * master; // Master table name
    SChar  *  slave; // Slave  table name

    SChar  *  schema; // Slave schema
    SChar  *   where; // Condition
    SChar  * exclude; // exclude columns
    UInt      mTabNo; // table number
};
typedef utTableProp utTableProp;


class utProperties
{
    SChar     * mDSNMaster;
    SChar     * mDSNSlave;

    dbDriver    mMasterDB;
    dbDriver    mSlave_DB;

    PDL_mutex_t mLock;

public:  /* Global configs */

    pmode_t     mMode; // Mode could be  DUMMY/SYNC/DIFF - DUMMY is default
    SInt        mMaxThread; // Maximum threads for processing

    static bool mVerbose; // show some information

    SInt        mCountToCommit; // Count of oops before commit
    SInt        mTimeInterval; // Check properties time interval

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    // write to CSV file.
    SInt        mMaxArrayFetch; // array fetch size

    /* Processing police */
    bool        mDML[DML_MAX];

    utProperties();
    ~utProperties();

    IDE_RC finalize();
    IDE_RC initialize(int argc, char **argv);

    void printUsage();
    void printConfig(FILE* = stdout);
    void printTab(FILE *,utTableProp *);
    /* 
     * BUG-32566
     *
     * iloader와 같이 Version 출력되도록 수정
     */
    void printVersion();

    IDE_RC getTabProp(utTableProp**); // remove from list of table properties for execution

    IDE_RC log(const SChar*, ... ); // write information into log file

    inline UShort       size     (){ return mSize;                      }
    inline       FILE*  getFLog  (){ return flog;                       }
    inline SChar *      getLogDir(){ return (logDir)?logDir:(SChar*)".";}

    inline void   lock() { IDE_ASSERT(idlOS::mutex_lock  (&mLock) == 0 ); }
    inline void unlock() { IDE_ASSERT(idlOS::mutex_unlock(&mLock) == 0 ); }

    Connection * newMaConn() { return mMasterDB.connection(); };
    Connection * newSlConn() { return mSlave_DB.connection(); };

private:

    SChar   *logFName;   // Log File Name
    SInt    log_level;   // Log Level
    SChar     *logDir;   // Log Directory

    FILE    *    flog;   // Log file descriptor

    utTableProp *mTab;   // list of properties
    UShort      mSize;   // size

    SChar       *memp;   // memory pool of string properties


    IDE_RC setProperty (SInt*    // pointer to Integer
                        , const SChar*    // pointer to Key1
                        , const SChar*    // pointer to Key2
                        ,       SChar* ); // String buffer value

    IDE_RC setProperty( bool  *
                        ,const SChar*
                        ,const SChar*
                        ,      SChar*);

    IDE_RC setProperty( SChar**
                        ,const SChar*
                        ,const SChar*
                        ,      SChar*, bool = false);

    IDE_RC setProperty(pmode_t*
                       ,const SChar*
                       ,const SChar*
                       ,      SChar*);

    IDE_RC checkProperty( const SChar *, const SChar *);

    IDE_RC prepare(const SChar*,const SChar*);

    utTableProp*  nextTabProp( SChar*); // get properties of tab

    inline SChar  * skips( SChar* aStr )
    {
            while ( *aStr == ' ' || *aStr == '\t' || *aStr == '\n' ) aStr++;
            return aStr;
    }

    inline void removeComment(SChar * aStr)
    {
        SChar * sStr;
        UInt    sCntDoubleQuote = 0;

        sStr = aStr;
        while(*sStr != '\0')
        {
            if(*sStr == '\"')
            {
                sCntDoubleQuote ++;
            }
            if(*sStr == '#')
            {
                if((sCntDoubleQuote % 2) == 0)
                {
                    break;
                }
            }
            sStr ++;
        }

        if (sStr > aStr)
        {
            do
            {
                sStr --;
            }
            while((*sStr == ' ') || (*sStr == '\t') || (*sStr == '\r'));

            sStr++;
        }
        *sStr = '\0';
    }

    IDE_RC tokenize( SChar**, // key
                     SChar**, // Value
                     SChar*&);// Input String

    inline static bool ident(SChar c, bool isDoubleQuotationMark)
    {
        UInt  i;  
        SChar sSpecialCharacter[] = 
        {' ', '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '+', '|'};

        if(((c >= 'A') && (c <= 'Z')) ||
           ((c >= 'a') && (c <= 'z')) ||
           ((c >= '0')  && (c <= '9')) ||
           (c == '_'))
        {
            return ID_TRUE;
        }

        if(isDoubleQuotationMark)
        {
            for(i = 0; i < sizeof(sSpecialCharacter); i++)
            {
                if(c == sSpecialCharacter[i])
                {
                    return ID_TRUE;
                }
            }
        }

        return ID_FALSE;
    }

    inline void _strcat(SChar * des, SChar * src)
        {
            while(*des != '\0') des++;
            while(*src != '\0')
            {
                *des = *src;
                src++;
                des++;
            }
            *des = '\0';
        }

    inline UInt  _tail(SChar * s)
        {
            SInt sz = idlOS::strlen(s);
            
            //BUG-35544 [ux] [XDB] codesonar warning at ux Warning 222602.2262125
            if( sz > 0)
            {

                sz--;

                while( ( sz >= 0 ) &&
                       ( s[sz] == '\n' || s[sz] == '\r' || s[sz] == '\t' || s[sz] == ' ' ) )
                {
                    s[sz--]='\0';
                }

                sz = (sz < 0)? 0:sz;
            }
            else
            {
                // Do nothing.
            }
            
            return sz;
        }
    inline SChar* _basename(SChar * name)
        {
            SChar * i;
            for(i = name; *name != '\0'; name++ )
            {
                if(*name == '\\') i = name+1;
            }
            return i;
        }
};
#endif
