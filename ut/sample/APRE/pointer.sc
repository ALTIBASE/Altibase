/******************************************************************
 * SAMPLE : POINT
 *          1. Using point of char host variables
 *          2. Using point of structure host variables
 *          3. Reference : array host variables - arrays1.sc
 ******************************************************************/

EXEC SQL BEGIN DECLARE SECTION;
typedef struct TAG
{
    char n1[11];
    int  n2;
} TAG;
EXEC SQL END DECLARE SECTION;

void ins_t1(char* data_t1);
void ins_t2(TAG* data_t2);

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    /* point of char type */
    char* data_t1;

    /* point of structure type */
    TAG *data_t2;

    EXEC SQL END DECLARE SECTION;

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    data_t1 = (char*)(malloc( 11 ));
    strcpy(data_t1, "A");

    data_t2 = (TAG*)(malloc(sizeof(TAG)));
    strcpy(data_t2->n1, "B");
    data_t2->n2 = 2;

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* drop table t1 */
    EXEC SQL DROP TABLE T1;

    /* create table t1 */
    EXEC SQL CREATE TABLE T1 ( i1 char(10) );
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* drop table t2 */
    EXEC SQL DROP TABLE T2;

    /* create table t2 */
    EXEC SQL CREATE TABLE T2 ( i1 char(10), i2 integer );
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    ins_t1(data_t1);
    ins_t2(data_t2);

    /* disconnect */
    EXEC SQL DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}

void ins_t1(char* data_t1)
{
    EXEC SQL BEGIN ARGUMENT SECTION;
    char* data_t1;
    EXEC SQL END ARGUMENT SECTION;

    /* using point of char host variable */
    EXEC SQL INSERT INTO T1 VALUES (:data_t1);

    printf("------------------------------------------------------------------\n");
    printf("[Point of Char Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n\n", sqlca.sqlerrd[2]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}

void ins_t2(TAG* data_t2)
{
    EXEC SQL BEGIN ARGUMENT SECTION;
    TAG *data_t2;
    EXEC SQL END ARGUMENT SECTION;

    /* using point of structure host variable */
    EXEC SQL INSERT INTO T2 VALUES (:data_t2->n1, :data_t2->n2);

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n\n", sqlca.sqlerrd[2]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
