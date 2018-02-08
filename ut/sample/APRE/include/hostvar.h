EXEC SQL BEGIN DECLARE SECTION;

typedef struct DEPARTMENT
{
    short dno;
    char  dname[30+1];
    char  dep_location[15+1];
    int   mgr_no;
} DEPARTMENT;

typedef struct DEPARTMENT_IND
{
    SQLLEN dno;
    SQLLEN dname;
    SQLLEN dep_location;
    SQLLEN mgr_no;
} DEPARTMENT_IND;

typedef struct EMPLOYEE
{
    int     eno;
    char    e_lastname[20+1];
    varchar emp_job[15+1];
    char    emp_tel[15+1];
    short   dno;
    double  salary;
    char    sex;
    char    birth[4+1];
    char    join_date[19+1];
    char    status[1+1];
} EMPLOYEE;

typedef struct EMPLOYEE_IND
{
    SQLLEN eno;
    SQLLEN e_lastname;
    SQLLEN emp_job;
    SQLLEN emp_tel;
    SQLLEN dno;
    SQLLEN salary;
    SQLLEN sex;
    SQLLEN birth;
    SQLLEN join_date;
    SQLLEN status;
} EMPLOYEE_IND;

typedef struct CUSTOMER
{
    long long cno;
    char      c_lastname[20+1];
    varchar   cus_job[20+1];
    char      cus_tel[15+1];
    char      sex;
    char      birth[4+1];
    char      postal_cd[9+1];
    varchar   address[60+1];
} CUSTOMER;

typedef struct CUSTOMER_IND
{
    SQLLEN cno;
    SQLLEN c_lastname;
    SQLLEN cus_job;
    SQLLEN cus_tel;
    SQLLEN sex;
    SQLLEN birth;
    SQLLEN postal_cd;
    SQLLEN address;
} CUSTOMER_IND;

typedef struct GOODS
{
    char   gno[10+1];
    char   gname[20+1];
    char   goods_location[9+1];
    int    stock;
    double price;
} GOODS;

typedef struct GOODS_IND
{
    SQLLEN gno;
    SQLLEN gname;
    SQLLEN goods_location;
    SQLLEN stock;
    SQLLEN price;
} GOODS_IND;

typedef struct ORDER
{
    long long ono;
    char      order_date[19+1];
    int       eno;
    long long cno;
    char      gno[10+1];
    int       qty;
    char      arrival_date[19+1];
    char      processing;
} ORDER;

typedef struct ORDER_IND
{
    SQLLEN ono;
    SQLLEN order_date;
    SQLLEN eno;
    SQLLEN cno;
    SQLLEN gno;
    SQLLEN qty;
    SQLLEN arrival_date;
    SQLLEN processing;
} ORDER_IND;

EXEC SQL END DECLARE SECTION;

