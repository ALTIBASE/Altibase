#include <ses.h>

#define STR_LEN 128

void clob_select(char  *out_filename);
void clob_insert(char  *src_filename);
void ses_connect();
void ses_disconn();

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Insufficient arguments.\n");
        exit(-1);
    }

    ses_connect();

    EXEC SQL AUTOCOMMIT OFF;

    clob_insert(argv[1]);
    clob_select(argv[2]);

    ses_disconn();

    return 0;
}

void ses_connect()
{
EXEC SQL BEGIN DECLARE SECTION;
char ses_user[STR_LEN];
char ses_passwd[STR_LEN];
EXEC SQL END DECLARE SECTION;

    strcpy(ses_user, "SYS");
    strcpy(ses_passwd, "MANAGER");

    exec sql connect :ses_user identified by :ses_passwd;
}

void ses_disconn()
{
    exec sql disconnect;
}

void clob_select(char  *out_filename)
{
    EXEC SQL BEGIN DECLARE SECTION;
    int          i1;
    char         i2_fname[33];
    unsigned int i2_fopt;
    SQLLEN       i2_ind;
    EXEC SQL END DECLARE SECTION;

    strcpy(i2_fname, out_filename);
    i2_fopt = APRE_FILE_CREATE;

    EXEC SQL SELECT * INTO :i1, CLOB_FILE :i2_fname OPTION :i2_fopt   INDICATOR :i2_ind FROM T_LOB;


   if (sqlca.sqlcode == SQL_SUCCESS )
   {
        printf("clob select ok !!!\n");

   }
   else
   {
        printf("clob select fail>>%s\n",sqlca.sqlerrm.sqlerrmc);
   }

    EXEC SQL COMMIT;
}

void clob_insert(char   *src_filename)
{
    EXEC SQL BEGIN DECLARE SECTION;
    int          i1;
    char         i2_fname[32];
    unsigned int i2_fopt;
    SQLLEN       i2_ind;
    EXEC SQL END DECLARE SECTION;

    i1 = 1;
    strcpy(i2_fname,src_filename);
    i2_fopt = APRE_FILE_READ;

    EXEC SQL INSERT INTO T_LOB VALUES(:i1, CLOB_FILE :i2_fname OPTION :i2_fopt INDICATOR :i2_ind);
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
       printf(" CLOB insert ok !!!\n");
    }
    else
    {
       printf(" CLOB insert fail !!!\n");
    }
    EXEC SQL COMMIT;
}
