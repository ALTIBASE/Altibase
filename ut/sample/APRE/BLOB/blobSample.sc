#include <ses.h>

#define STR_LEN 128

void blob_select(char  *out_filename);
void blob_insert(char  *src_filename);
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

    blob_insert(argv[1]);
    blob_select(argv[2]);

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

void blob_select(char  *out_filename)
{
    EXEC SQL BEGIN DECLARE SECTION;
    int          i1;
    char         i2_fname[33];
    unsigned int i2_fopt;
    SQLLEN       i2_ind;
    EXEC SQL END DECLARE SECTION;

    strcpy(i2_fname, out_filename);
    i2_fopt = APRE_FILE_CREATE;

    EXEC SQL SELECT * INTO :i1, BLOB_FILE :i2_fname OPTION :i2_fopt   INDICATOR :i2_ind FROM T_LOB;


   if (sqlca.sqlcode == SQL_SUCCESS )
   {
        printf("blob select ok !!!\n");

   }
   else
   {
        printf("blob select fail~\n");
   }

    EXEC SQL COMMIT;
}

void blob_insert(char   *src_filename)
{
    EXEC SQL BEGIN DECLARE SECTION;
    int          i1;
    char         i2_fname[32];
    unsigned int i2_fopt;
    SQLLEN       i2_ind = 0;
    EXEC SQL END DECLARE SECTION;

    i1 = 1;
    strcpy(i2_fname,src_filename);
    i2_fopt = APRE_FILE_READ;

    EXEC SQL INSERT INTO T_LOB VALUES(:i1, BLOB_FILE :i2_fname OPTION :i2_fopt INDICATOR :i2_ind);
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
       printf(" BLOB insert ok !!!\n");
    }
    else
    {
       printf(" BLOB insert fail !!!\n");
    }
    EXEC SQL COMMIT;
}
