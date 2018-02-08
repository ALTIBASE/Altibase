/******************************************************************
 * SAMPLE : BINARY TYPE
 *          1. Using as input host variables
 *          2. Using as output host variables
 *          3. Using APRE_BINARY
 *          4. Using APRE_BYTES
 *          5. Using APRE_NIBBLE
 *          +-------------+--------------------+
 *          | COLUMN TYPE | HOST VARIALBE TYPE |
 *          +-------------+--------------------+
 *          | CLOB        | APRE_CLOB          |
 *          +-------------+--------------------+
 *          | BLOB        | APRE_BLOB          |
 *          |             | APRE_BINARY        |
 *          +-------------+--------------------+
 *          | BYTE        | APRE_BYTES         |
 *          +-------------+--------------------+
 *          | NIBBLE      | APRE_NIBBLE        |
 *          +-------------+--------------------+
 ******************************************************************/

void f_clob();
void f_blob();
void f_binary();
void f_bytes();
void f_nibble();

int main()
{
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];
    EXEC SQL END DECLARE SECTION;

    printf("<BINARY TYPE>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    EXEC SQL DROP TABLE T_CLOB;
    EXEC SQL DROP TABLE T_BLOB;
    EXEC SQL DROP TABLE T_BINARY;
    EXEC SQL DROP TABLE T_BYTES;
    EXEC SQL DROP TABLE T_NIBBLE;

    EXEC SQL CREATE TABLE T_CLOB (I1 CLOB);
    EXEC SQL CREATE TABLE T_BLOB (I1 BLOB);
    EXEC SQL CREATE TABLE T_BINARY (I1 BLOB);
    EXEC SQL CREATE TABLE T_BYTES (I1 BYTE(5));
    EXEC SQL CREATE TABLE T_NIBBLE (I1 NIBBLE(10));

    f_clob();
    f_blob();
    f_binary();
    f_bytes();
    f_nibble();

    /* disconnect */
    EXEC SQL DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}

/* column type is CLOB
 * host variable type is APRE_CLOB
 * should use indicator variables together */
void f_clob()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    /* column type must be CLOB */
    APRE_CLOB ins_clob[10+1];
    APRE_CLOB sel_clob[10+1];

    /* declare indicator variables */
    SQLLEN ins_clob_ind;
    SQLLEN sel_clob_ind;
    EXEC SQL END DECLARE SECTION;

    printf("------------------------------------------------------------------\n");
    printf("[APRE_CLOB]\n");
    printf("------------------------------------------------------------------\n");

    EXEC SQL AUTOCOMMIT OFF;

    memset(ins_clob, 0x41, 10);
    /* set length of ins_clob value to indicator variable */
    ins_clob_ind = 10;

    /* use APRE_CLOB as input host variables */
    EXEC SQL INSERT INTO T_CLOB VALUES (:ins_clob :ins_clob_ind);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success insert with APRE_CLOB\n");
    }
    else
    {
        printf("Error : [%d] %s\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
    EXEC SQL COMMIT;

    /* use APRE_CLOB as output host variables */
    EXEC SQL SELECT * INTO :sel_clob :sel_clob_ind FROM T_CLOB;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        sel_clob[sel_clob_ind] = '\0';
        printf("sel_clob     = %s\n", sel_clob);
        printf("sel_clob_ind = %d\n\n", sel_clob_ind);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL AUTOCOMMIT ON;
}

/* column type is BLOB
 * host variable type is APRE_BINARY
 * should use indicator variables together */
void f_blob()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    /* column type must be BLOB */
    APRE_BLOB ins_blob[10+1];
    APRE_BLOB sel_blob[10+1];

    /* declare indicator variables */
    SQLLEN ins_blob_ind;
    SQLLEN sel_blob_ind;
    EXEC SQL END DECLARE SECTION;

    printf("------------------------------------------------------------------\n");
    printf("[APRE_BLOB]\n");
    printf("------------------------------------------------------------------\n");

    EXEC SQL AUTOCOMMIT OFF;

    memset(ins_blob, 0x21, 10);
    /* set length of ins_blob value to indicator variable */
    ins_blob_ind = 10;

    /* use APRE_BLOB as input host variables */
    EXEC SQL INSERT INTO T_BLOB VALUES (:ins_blob :ins_blob_ind);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success insert with APRE_BLOB\n");
    }
    else
    {
        printf("Error : [%d] %s\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
    EXEC SQL COMMIT;

    /* use APRE_BLOB as output host variables */
    EXEC SQL SELECT * INTO :sel_blob :sel_blob_ind FROM T_BLOB;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        sel_blob[sel_blob_ind] = '\0';
        printf("sel_blob     = %s\n", sel_blob);
        printf("sel_blob_ind = %d\n\n", sel_blob_ind);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL AUTOCOMMIT ON;
}

/* column type is BLOB
 * host variable type is APRE_BINARY
 * should use indicator variables together */
void f_binary()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    /* column type must be BLOB */
    APRE_BINARY ins_blob[10+1];
    APRE_BINARY sel_blob[10+1];

    /* declare indicator variables */
    SQLLEN ins_blob_ind;
    SQLLEN sel_blob_ind;
    EXEC SQL END DECLARE SECTION;

    printf("------------------------------------------------------------------\n");
    printf("[APRE_BINARY]\n");
    printf("------------------------------------------------------------------\n");

    EXEC SQL AUTOCOMMIT OFF;

    memset(ins_blob, 0x21, 10);
    /* set length of ins_blob value to indicator variable */
    ins_blob_ind = 10;

    /* use APRE_BINARY as input host variables */
    EXEC SQL INSERT INTO T_BINARY VALUES (:ins_blob :ins_blob_ind);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success insert with APRE_BINARY\n");
    }
    else
    {
        printf("Error : [%d] %s\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
    EXEC SQL COMMIT;

    /* use APRE_BINARY as output host variables */
    EXEC SQL SELECT * INTO :sel_blob :sel_blob_ind FROM T_BINARY;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        sel_blob[sel_blob_ind] = '\0';
        printf("sel_blob     = %s\n", sel_blob);
        printf("sel_blob_ind = %d\n\n", sel_blob_ind);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL AUTOCOMMIT ON;
}

/* column type is BYTE
 * host variable type is APRE_BYTES
 * should use indicator variables together */
void f_bytes()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    /* column type must be BYTE */
    APRE_BYTES ins_bytes[5+1];
    APRE_BYTES sel_bytes[5+1];

    /* declare indicator variables */
    SQLLEN ins_bytes_ind;
    SQLLEN sel_bytes_ind;
    EXEC SQL END DECLARE SECTION;

    printf("------------------------------------------------------------------\n");
    printf("[APRE_BYTES]\n");
    printf("------------------------------------------------------------------\n");

    memset(ins_bytes, 0x21, 5);
    /* set length of ins_bytes value to indicator variable */
    ins_bytes_ind = 5;

    /* use APRE_BYTES as input host variables */
    EXEC SQL INSERT INTO T_BYTES VALUES (:ins_bytes :ins_bytes_ind);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success insert with APRE_BYTES\n");
    }
    else
    {
        printf("Error : [%d] %s\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use APRE_BYTES as output host variables */
    EXEC SQL SELECT * INTO :sel_bytes :sel_bytes_ind FROM T_BYTES;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        sel_bytes[sel_bytes_ind] = '\0';
        printf("sel_bytes     = %s\n", sel_bytes);
        printf("sel_bytes_ind = %d\n\n", sel_bytes_ind);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}

/* column type is NIBBLE
 * host variable type is APRE_NIBBLE
 * should use indicator variables together when output host variable */
void f_nibble()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    /* column type must be NIBBLE
     * NIBBLE = 4 bits */
    APRE_NIBBLE ins_nibble[5+2];
    APRE_NIBBLE sel_nibble[5+2];

    /* declare indicator variables */
    SQLLEN sel_nibble_ind;
    EXEC SQL END DECLARE SECTION;

    printf("------------------------------------------------------------------\n");
    printf("[APRE_NIBBLE]\n");
    printf("------------------------------------------------------------------\n");

    memset(ins_nibble+1, 0x21, 5);
    /* set length of ins_nibble value to ins_nibble[0] */
    ins_nibble[0] = 10;

    /* use APRE_NIBBLE as input host variables
     * have no need indicator variables */
    EXEC SQL INSERT INTO T_NIBBLE VALUES (:ins_nibble);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success insert with APRE_NIBBLE\n");
    }
    else
    {
        printf("Error : [%d] %s\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use APRE_NIBBLE as output host variables */
    EXEC SQL SELECT * INTO :sel_nibble :sel_nibble_ind FROM T_NIBBLE;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        sel_nibble[sel_nibble_ind] = '\0';
        printf("sel_nibble     = %s\n", sel_nibble+1);
        printf("sel_nibble_ind = %d\n", sel_nibble_ind);
        printf("sel_nibble[0]  = %d\n\n", sel_nibble[0]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
