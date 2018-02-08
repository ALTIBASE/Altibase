#ifndef DEMO_COMMON_H
#define DEMO_COMMON_H 1



#define CDBC_TEST_RAISE(aExpr, aLabel) do\
{\
    if (aExpr)\
    {\
        printf("ERRLINE %d : ", __LINE__);\
        goto aLabel;\
    }\
} while (0)

#define PRINT_DBC_ERROR(aAB) do\
{\
	if (altibase_errno(aAB) == 0)\
	{\
        printf("ERRLINE %d : ", __LINE__);\
		printf("no error info\n");\
	}\
	else\
	{\
        printf("ERRLINE %d : ", __LINE__);\
        printf("[%05X] %s %s\n", altibase_errno(aAB),\
               altibase_sqlstate(aAB), altibase_error(aAB));\
	}\
} while (0)

#define PRINT_STMT_ERROR(aStmt) do\
{\
	if (altibase_stmt_errno(aStmt) == 0)\
	{\
        printf("ERRLINE %d : ", __LINE__);\
		printf("no error info\n");\
	}\
	else\
	{\
        printf("ERRLINE %d : ", __LINE__);\
        printf("[%05X] %s %s\n", altibase_stmt_errno(aStmt),\
               altibase_stmt_sqlstate(aStmt), altibase_stmt_error(aStmt));\
	}\
} while (0)

#define SET_TIMESTAMP(aTsPtr, aYear, aMonth, aDay, aHour, aMin, aSec, aFraction) do\
{\
    (aTsPtr)->year     = aYear;\
    (aTsPtr)->month    = aMonth;\
    (aTsPtr)->day      = aDay;\
    (aTsPtr)->hour     = aHour;\
    (aTsPtr)->minute   = aMin;\
    (aTsPtr)->second   = aSec;\
    (aTsPtr)->fraction = aFraction;\
} while (0)

#define SAFE_FREE(aPtr) do\
{\
	if ((aPtr) != NULL)\
	{\
		free(aPtr);\
	}\
} while (0)



#endif /* DEMO_COMMON_H */
