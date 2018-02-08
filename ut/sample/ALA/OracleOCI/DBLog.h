/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __DB_LOG_H__
#define __DB_LOG_H__ 1

/*
 *
 */
typedef enum {

    DB_LOG_TYPE_COMMIT,
    DB_LOG_TYPE_ABORT,
    DB_LOG_TYPE_INSERT,
    DB_LOG_TYPE_UPDATE,
    DB_LOG_TYPE_DELETE,
    DB_LOG_TYPE_REPLICATION_STOP,

    DB_LOG_TYPE_NONE

} DB_LOG_TYPE;

/*
 *
 */
typedef enum {

    DB_LOG_COLUMN_TYPE_NUMERIC,
    DB_LOG_COLUMN_TYPE_FLOAT,
    DB_LOG_COLUMN_TYPE_DOUBLE,
    DB_LOG_COLUMN_TYPE_REAL,
    DB_LOG_COLUMN_TYPE_BIGINT,
    DB_LOG_COLUMN_TYPE_INTEGER,
    DB_LOG_COLUMN_TYPE_SMALLINT,
    DB_LOG_COLUMN_TYPE_CHAR,
    DB_LOG_COLUMN_TYPE_VARCHAR,
    DB_LOG_COLUMN_TYPE_NCHAR,
    DB_LOG_COLUMN_TYPE_NVARCHAR,
    DB_LOG_COLUMN_TYPE_DATE,

    DB_LOG_COLUMN_TYPE_NONE

} DB_LOG_COLUMN_TYPE;

/*
 *
 */
typedef struct DB_LOG_COLUMN {

    char *mName;

    DB_LOG_COLUMN_TYPE mType;

    int mLength;

    char mValue[4096];

} DB_LOG_COLUMN;

/*
 *
 */
typedef struct DB_LOG_INSERT {

    DB_LOG_TYPE mType;

    char *mTableName;

    int mColumnCount;

    DB_LOG_COLUMN *mColumn;

} DB_LOG_INSERT;

/*
 *
 */
typedef struct DB_LOG_DELETE {

    DB_LOG_TYPE mType;

    char *mTableName;

    int mPrimaryKeyCount;

    DB_LOG_COLUMN *mPrimaryKey;

} DB_LOG_DELETE;

/*
 *
 */
typedef struct DB_LOG_UPDATE {

    DB_LOG_TYPE mType;

    char *mTableName;

    int mPrimaryKeyCount;

    DB_LOG_COLUMN *mPrimaryKey;

    int mColumnCount;

    DB_LOG_COLUMN *mColumn;

} DB_LOG_UPDATE;

/*
 *
 */
typedef union DB_LOG {

    DB_LOG_TYPE mType;

    DB_LOG_UPDATE mUpdate;

    DB_LOG_INSERT mInsert;

    DB_LOG_DELETE mDelete;

} DB_LOG;

#endif /* __DB_LOG_H__ */
