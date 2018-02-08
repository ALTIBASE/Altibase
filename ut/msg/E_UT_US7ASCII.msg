# $Id: E_UT_US7ASCII.msg 31777 2009-03-19 13:40:35Z bbaejae $
#
# UT 에러 메시지 화일 (used by genErrMsg)
#

# 에러코드 할당 정책은 아래 위키 페이지를 참조하시기 바랍니다.
# http://wiki.altibase.in/wiki/index.php/Altimsg
# 브랜치에서 에러코드를 할당받으시면 안됩니다.
# 반드시 main trunk에서 번호를 할당받은 후 브랜치에서 사용하셔야 합니다.
# 의문 사항이 있으시면 AK팀 혹은 QP(품질본부)팀에 문의 부탁드립니다.
# Last Number = 322

SECTION = 9

############# iSQL ##############

1, HY000, utERR_IGNORE_NoError = No UT module error
# *Cause: This is not an error.
# *Action: You can safely ignore this error message.

2, HY000, utERR_ABORT_alreadyExistFileError = File (<0%s>) already exists.
# *Cause: The file already exists.
# *Action: Remove or rename the file.

3, HY000, utERR_ABORT_env_not_exist = The environment (<0%s>) does not exist.
# *Cause: The ALTIBASE environment has not been set.
# *Action: Set the ALTIBASE environment value.

4, HY000, utERR_ABORT_Invalid_dataType_Error = Invalid data type specified.
# *Cause: The data type to be bound is not valid.
# *Action: Check the data type to be bound.

5, HY000, utERR_ABORT_name_length_overflow_Error = The host variable name length must be less than <0%d> characters.
# *Cause: Host variable name length overflow.
# *Action: Decrease the length of the host variable name.

6, HY000, utERR_ABORT_memory_error = Memory allocation error.(<0%s>:<1%d>)
# *Cause: The application failed to allocate memory for an environment handle with SQLAllocEnv.
# *Action: Verify that there is enough memory to run the application. If necessary, take action to make more memory available for the application.

7, HY000, utERR_ABORT_host_variable_not_defined_error = The host variable <0%s> was not defined.
# *Cause: Host variable not defined.
# *Action: Define the host variable.

8, HY000, utERR_ABORT_type_mismatch_error = Type mismatch error.
# *Cause: The types do not match.
# *Action: Ensure that the data types are matched.

9, HY000, utERR_ABORT_too_large_error = Too large data specified.
# *Cause: The length of the specified data was too large.
# *Action: Decrease the length of the data.

10, HY000, utERR_ABORT_value_OutOfRange_error = Host value out of range.
# *Cause: Host value out of range.
# *Action: Check the host value and ensure that it is within range.

11, HY000, utERR_ABORT_account_privilege_error = Privilege error for sysdba user account.
# *Cause: Sysdba users must be connected with ALTIBASE user accounts.
# *Action: Check the user account.

12, HY000, utERR_ABORT_stat_error = stat() system call error (errno=<0%d>)
# *Cause: The stat() call encountered an error.
# *Action: Check the file system.

13, HY000, utERR_ABORT_Unterminated_error = Unterminated user command
# *Cause: An unterminated user command was input.
# *Action: Check the command.

14, HY000, utERR_ABORT_open_Syspasswd_FileError = could not open syspassword file (<0%s>)
# *Cause: The syspassword does not exist, or you do not have sufficient permission.
# *Action: Ensure that the file exists and that you have sufficient permission to open the file.

15, HY000, utERR_ABORT_Incorrect_Password = Incorrect Password
# *Cause: The password was incorrect.
# *Action: Check the password.

16, HY000, utERR_ABORT_Syntax_Error = Syntax Error
# *Cause: Syntax Error.
# *Action: Check the syntax.

17, HY000, utERR_ABORT_No_History_Error = No user command saved
# *Cause: There are no saved commands.
# *Action: Specify a command and retry.

18, HY000, utERR_ABORT_History_Range_Error = History number must be between 1 and <0%d>.
# *Cause: An incorrect history number was specified.
# *Action: Change the history number and retry.

19, HY000, utERR_ABORT_Too_Long_Query_Error = The query was too long; the maximum length is <0%d>.
# *Cause: The query was too long.
# *Action: Shorten the query.

261, HY000, utERR_ABORT_User_No_Exist_Error = The user (<0%s>) does not exist.
# *Cause: No user with that name exists.
# *Action: Check the user name and try again.

20, HY000, utERR_ABORT_Table_No_Exist_Error = The table (<0%s>) does not exist.
# *Cause: No table with that name exists.
# *Action: Check the table name and try again.

21, 08S01, utERR_ABORT_Comm_Failure_Error = Communication failure.
# *Cause: The network connection was closed.
# *Action: Check the dbms server.

22, HY000, utERR_ABORT_Sequence_No_Exist_Error = The sequence does not exist.
# *Cause: No sequence with that name exists.
# *Action: Check the sequence name.

23, HY000, utERR_ABORT_Invalid_Command_Error = The use of this command is not allowed in a script file.
# *Cause: Invalid command use.
# *Action: Do not use this command in a script file.

24, HY000, utERR_ABORT_sysdba_privilege_Error = Please reconnect as sysdba.
# *Cause: sysdba user must be used for this action.
# *Action: Reconnect as sysdba.

25, HY000, utERR_ABORT_command_buffer_Error = <0%s> must be greater than <1%d>.
# *Cause: The ISQL_BUFFER_SIZE(or XDB_ISQL_BUFFER_SIZE for XDB) was too small.
# *Action: Increase the value of the ISQL_BUFFER_SIZE(or XDB_ISQL_BUFFER_SIZE for XDB) environment variable.

26, HY000, utERR_ABORT_property_omit_Error = Property (<0%s>) not found.
# *Cause: The property was not found.
# *Action: Check whether the property exists.

27, HY000, utERR_ABORT_Column_Size_Error = ColSize option value out of range (<0%d> - <1%d>)
# *Cause: An invalid ColSize value was specified.
# *Action: Check the ColSize value.

28, HY000, utERR_ABORT_Page_Size_Error = PageSize option value out of range (<0%d> - <1%d>)
# *Cause: An invalid PageSize value was specified.
# *Action: Check the PageSize value.

29, HY000, utERR_ABORT_already_spool_on_Error = Spool is already in an ON state. (<0%s>)
# *Cause: Spool already ON.
# *Action: Turn off the spool and retry.

30, HY000, utERR_ABORT_cant_spool_off_Error = Spool is not in an ON state.
# *Cause: Can't turn off a spool that is already off.
# *Action: Only attempt this action in a spool-on state.

31, HY000, utERR_ABORT_closeFileError = Could not close the file (<0%s>).
# *Cause: The application failed to close a file.
# *Action: This could be a system problem or programming error. Verify that there is no problem with your file system. If there is no such problem, check the error number from the trace log and contact Altibase's Support Center (http://support.altibase.com).

32, HY000, utERR_ABORT_Not_Connected_Error = No Connection State
# *Cause: No Connection exists.
# *Action: Connect first and try again.

33, HY000, utERR_ABORT_Line_Size_Error = Line Size option value out of range (<0%d> - <1%d>)
# *Cause: An invalid line size was specified.
# *Action: Check the line size.

34, HY000, utERR_ABORT_UNDISPLAYABLE_DATATYPE_Error = LOB and GEOMETRY type data cannot be displayed
# *Cause: Can't print LOB or GEOMETRY data using the console.
# *Action: Use a user CLI Application for printing.

37, HY000, utERR_ABORT_WRONG_DATATYPE_Error = Unknown data types cannot be displayed
# *Cause: Can't print unknown data types using the console.
# *Action: Use a user CLI Application for printing.

38, HY000, utERR_ABORT_Proc_No_Exist_Error = The procedure (<0%s>) does not exist.
# *Cause: There is no procedure with the name you specified.
# *Action: Check the procedure name and try again.

250, HY008, utERR_ABORT_Operation_Canceled = Operation cancelled
# *Cause: SQLCancel was called on the StatementHandle.
# *Action: No action is necessary.

251, CIDLE, utERR_ABORT_Connected_Idle_Instance_Error = Connected to idle instance
# *Cause: No error has occurred, just notification of connection to an idle instance of Altibase.
# *Action: No action is necessary.

252, HY000, utERR_ABORT_Incorrect_User = Incorrect user name used (<0%s>).
# *Cause: Only "sys" user can use "-sysdba" option.
# *Action: Login as "sys" user name to use "-sysdba" option.

257, HY000, utERR_ABORT_LOB_AUTOCOMMIT_MODE_ERR = Connection is in autocommit mode. One cannot operate on LOB data when autocommit mode is on.
# *Cause: An attempt to execute a LOB operation was made while the connection was in autocommit mode.
# *Action: Turn off autocommit mode.

292, HY000, utERR_ABORT_GRACE_TIME_WARN = The password will expire within <0%d> days.
# *Cause: The user's account has expired and the password needs to be changed.
# *Action: Change the user's password.

293, HY000, utERR_ABORT_Num_Width_Error = NumWidth option value out of range (<0%d> - <1%d>).
# *Cause: An invalid numwidth was specified.
# *Action: Check the numwidth.

294, HY000, utERR_ABORT_INVALID_CONN_ATTR = Invalid connection attribute pair: <0%s> = <1%s>
# *Cause: The connection attribute is invalid.
# *Action: Check the connection attribute.

307, HY000, utERR_ABORT_Exceed_Max_String = String is too long. Maximum size is <0%d> characters.
# *Cause: The string is longer than the maximum length.
# *Action: Use a string shorter than the maximum length.

308, HY000, utERR_ABORT_Failed_Open_Path = Failed to open the path.(<0%s>: <1%s>)
# *Cause: Either an invalid path was specified or you have insufficient persmission.
# *Action: Verify that you have entered the correct path or that you have sufficient permission.

312, HY000, utERR_ABORT_Cancel_Not_Supported = Canceling is not supported.
# *Cause: The driver does not support the SQLCancel function.
# *Action: Wait for the statement to be completed or close this session using ALTER DATABASE by another session.

313, HY000, utERR_ABORT_Illegal_Format_String = Illegal FORMAT String (<0%s>).
# *Cause: An invalid format string was specified.
# *Action: Retry again with a valid format string.

314, HY000, utERR_ABORT_Column_Not_Defined = Column not defined (<0%s>).
# *Cause: The specified column name was not defined.
# *Action: Retry again with a valid column name.

295, HY000, utERR_FATAL_FLOCK_INIT = Failed to invoke the flock_init() system function [<0%s>]
# *Cause:
#    - The flock_init() system call failed.
# *Action: Please send a bug report to the vendor.

296, HY000, utERR_FATAL_FLOCK_DESTROY = Failed to invoke the flock_destroy() system function [<0%s>]
# *Cause:
#    - The flock_destroy() system call failed.
# *Action: Please send a bug report to the vendor.

297, HY000, utERR_FATAL_FLOCK_TRYWRLOCK = Failed to invoke the flock_trywrlock() system function
# *Cause:
#    - The flock_trywrlock() system call failed.
# *Action: Please send a bug report to the vendor.

298, HY000, utERR_FATAL_FLOCK_UNLOCK = Failed to invoke the flock_unlock() system function
# *Cause:
#    - The flock_unlock() system call failed.
# *Action: Please send a bug report to the vendor.

317, HY000, utERR_ABORT_Failed_To_Cancel = Cancel cannot be done at the moment.
# *Cause: The cancel request cannot be accepted at the moment.
# *Action: Wait until the statement is completed.

319, HY000, utERR_ABORT_Failed_To_Sysdba_Connect_Remotely = Unable to connect to deactivated Altibase by remotely using -sysdba option.
# *Cause: The remote Altibase is not operating.
# *Action: Try again after starting Altibase.

321, HY000, utERR_ABORT_PrefetchRows_Error = PrefetchRows option value out of range (<0%d> - <1%d>)
# *Cause: An invalid PrefetchRows value was specified.
# *Action: Check the PrefetchRows value.

322, HY000, utERR_ABORT_AsyncPrefetch_Auto_Warning = Unable to run auto tuning due to the CLI restriction. 'AsyncPrefetch on' will be automatically applied.
# *Cause: The auto tuning is only supported on Linux environment.
# *Action: 'AsyncPrefetch on' is automatically applied on operating systems other than Linux. However, 'AsyncPrefetch on' should be specified whether or not to display a warning.

###### AEXPORT #########
############# iLoader ##############

39, HY000, utERR_ABORT_Dup_Option_Error = Option (<0%s>) is used more than once.
# *Cause: A duplicate option was specified.
# *Action: Check the options.

40, HY000, utERR_ABORT_Unknown_Option_Error = An unknown Option (<0%s>) was specified.
# *Cause: An unknown option was specified.
# *Action: Check the option and try again.

41, HY000, utERR_ABORT_Option_Value_Range_Error =  The option value (<0%s>) must be greater than (<1%d>).
# *Cause: The option value was not within the valid range.
# *Action: Change the option value.

42, HY000, utERR_ABORT_Not_Support_dataType_Error = The data type (<0%s><1%s>) <2%s><3%s> is not supported.
# *Cause: An invalid data type was used.
# *Action: Check the data type.

43, HY000, utERR_ABORT_Token_Value_Range_Error =  Token value length overflow. Maximum token length=<0%d>. Column =<1%s>, Value=<2%s>
# *Cause: The token value length exceeded the valid range.
# *Action: Decrease the token length.

44, HY000, utERR_ABORT_Parse_Command_Error = Input command parser error
# *Cause: The command parse token was invalid.
# *Action: Check the token.

46, HY000, utERR_ABORT_Result_Attr_Count_Error = Result attribute count is different. (Form=<0%d>, User=<1%d>
# *Cause: The table data type information was different.
# *Action: Check the table information.

47, HY000, utERR_ABORT_Delete_Table_Error = Failed to delete record from table (<0%s>)
# *Cause: Delete operation failed.
# *Action: Check the table recordset.

48, HY000, utERR_ABORT_Arg_omit_Error = Argument (<0%s>) not found.
# *Cause: A necessary argument was omitted.
# *Action: Provide the argument and try again.

49, HY000, utERR_ABORT_Option_Incorrect_Error = The <0%s> option (<1%s>) was not used, or the precedence option is not correct.
# *Cause: Option use error.
# *Action: Ensure that you are using the options correctly.

50, HY000, utERR_ABORT_Field_Terminator_Error = Field, Row and Enclosingchar terminators must be different.
# *Cause: A terminator conflict occurred.
# *Action: Use a different terminator.

51, HY000, utERR_ABORT_SQLColumn_Error = Field, Row and Enclosingchar terminators must be different.
# *Cause: A terminator conflict occurred.
# *Action: Use a different terminator.

#52, HY000, utERR_ABORT_no_index_Error = The table (<0%s>) does not have an index.
# *Cause: The table does not have an index.
# *Action: Add an index to the table.

53, HY000, utERR_ABORT_too_many_Seq_Error = Too many sequences were used. (LIMIT=<0%d>)
# *Cause: The number of sequences exceeded the valid range.
# *Action: Decrease the number of sequences.

54, HY000, utERR_ABORT_too_many_Attr_Error = Too many attributes were used. (LIMIT=<0%d>)
# *Cause: The number of attributes exceeded the valid range.
# *Action: Decrease the number of attributes.

55, HY000, utERR_ABORT_No_Column_Sequence_Error = No existing column linked to sequence (<0%s>)
# *Cause: No column is linked to the sequence.
# *Action: Check the form file.

56, HY000, utERR_ABORT_Many_Column_Sequence_Error = Duplicate column (<0%s>) to sequence (<1%d>) links
# *Cause: A column is linked to the sequence two or more times.
# *Action: Check form file.

57, HY000, utERR_ABORT_Unkown_Datatype_Error = An unknown datatype <0%s> was used.
# *Cause: An assign attempt was made using an unknown data type.
# *Action: Check the data type used for binding.

58, HY000, utERR_ABORT_Wrong_Column_Count_Error = Column count mismatch (size=<0%d>
# *Cause: Invalid data fetch processing.
# *Action: Check the table information.

59, HY000, utERR_ABORT_Invalid_Option_Error = Invalid option (<0%s>)
# *Cause: An invalid option was used.
# *Action: Check the options.

60, HY000, utERR_ABORT_Option_Value_Overflow_Error =  The option value for <0%s> must not exceed the upper limit (<1%d> <2%s>).
# *Cause: The option value exceeded the upper limit.
# *Action: Change the option value.

61, HY000, utERR_ABORT_Parsing_Error = Data parsing error (Column : <0%s>)
# *Cause: Data parsing error.
# *Action: Check the token.

62, HY000, utERR_ABORT_UserID_Omit_Error = A user ID must be specified.
# *Cause: The userID was omitted.
# *Action: Enter a userID.

63, HY000, utERR_ABORT_Password_Omit_Error = A password must be specified.
# *Cause: The password was omitted.
# *Action: Enter a password.

64, HY000, utERR_ABORT_ServerName_Omit_Error = A ServerName must be specified.
# *Cause: The ServerName was omitted.
# *Action: Enter a serverName.

291, HY000, utERR_ABORT_Port_Omit_Error = No port number was specified.
# *Cause: The -port option value was omitted.
# *Action: Specify a port number for the port option and try again.

65, HY000, utERR_ABORT_LOB_Opt_Str_Error = Missing or invalid LOB option string.
# *Cause: A LOB option string is missing or invalid.
# *Action: Check the LOB option string.

66, HY000, utERR_ABORT_Invalid_Value = Invalid data value
# *Cause:  The value of the data exceeds the valid range.
# *Action: Check the data value.

287, HY000, utERR_ABORT_Invalid_Value_errno = Invalid data value (errno=<0%d>)
# *Cause:  The value of the data exceeds the valid range.
# *Action: Check the data value.

67, HY000, utERR_ABORT_Invalid_Value_Len = Invalid data length
# *Cause:  The length of the data exceeds the valid range.
# *Action: Check the length of the data.

68, HY000, utERR_ABORT_Data_File_IO_Error = Error occurred during data file I/O.
# *Cause:  The file does not exist, there is no more space in the filesystem, the file size is too big, or a similar error has occurred.
# *Action: Check the filesystem.

69, HY000, utERR_ABORT_LOB_File_IO_Error = Error occurred during LOB file I/O.
# *Cause:  The file does not exist, there is no more space in the filesystem, the file size is too big, or a similar error has occurred.
# *Action: Check the filesystem.

70, HY000, utERR_ABORT_Nls_Use_Error = ALTIBASE_NLS_USE does not match DATA_NLS_USE
# *Cause: ALTIBASE_NLS_USE does not match DATA_NLS_USE in formout file.
# *Action: Change ALTIBASE_NLS_USE to match DATA_NLS_USE and retry.

71, HY000, utERR_ABORT_CSV_Option_Error = The -rule csv option can't be used with the -t or -e options.
# *Cause:  An attempt was made to use the -rule csv option with the -t or -e option.
# *Action: Check the options and try again.

253, HY000, utERR_ABORT_Invalid_CSV_File_Format_Error = Invalid CSV file format token. Column=<0%s>, Value=<1%s>.
# *Cause:  The CSV file format is wrong.
# *Action: Check the CSV data file.

256, HY000, utERR_ABORT_File_Lock_Error = File [<0%s>] open failed. Already in use.
# *Cause: The specified file is already in use.
# *Action: Check whether the file is already in use.

258,HY000, utERR_ABORT_Form_Parser_Error = Input form parser error.
# *Cause:  The form file is wrong.
# *Action: Check the form file.

259,HY000, utERR_FATAL_Mutex = Mutex Operation Error.(<0%s>)
# *Cause:  The application encountered an unexpected error with the mutex.
# *Action: This is possibly a programming error. Check the error number from the trace log and contact Altibase's Support Center (http://support.altibase.com)

260, HY000, utERR_ABORT_OutFile_IO_Error = An error occurred during file output.
# *Cause:  The file does not exist, there is no more space in the file system, the file size is too big, or a similar error has occurred.
# *Action: Check the file system.

262, HY000, utERR_ABORT_Bad_File_IO_Error = An error occurred during bad file I/O.
# *Cause:  The file does not exist, there is no more space in the file system, the file size is too big, or a similar error has occurred.
# *Action: Check the file system.

263, HY000, utERR_ABORT_Log_File_IO_Error = An error occurred during log file I/O.
# *Cause:  The file does not exist, there is no more space in the file system, the file size is too big, or a similar error has occurred.
# *Action: Check the file system.

315, HY000, utERR_ABORT_Param_Count_Mismatch = Parameter count mismatch. Required=(<0%d>), INSERT statement=(<1%s>).
# *Cause: The number of parameters in the INSERT statement is different from the number of table columns.
# *Action: Check the form file.

318, HY000, utERR_ABORT_Prefetch_Row_Error = -prefetch_row option value out of range (<0%d> - <1%d>)
# *Cause: An invalid PrefetchRow value was specified.
# *Action: Check the PrefetchRow value.

320, HY000, utERR_ABORT_Async_Prefetch_Auto_Warning = Unable to run auto tuning due to the CLI restriction. '-async_prefetch on' will be automatically applied.
# *Cause: The auto tuning is only supported on Linux environment.
# *Action: '-async_prefetch on' is automatically applied on operating systems other than Linux to execute an iloader out command. However, '-async_prefetch on' should be specified whether or not to display a warning.

###### AEXPORT #########

100, HY000, utERR_ABORT_password_Error = Cound not find the password for user (<0%s>)
# *Cause: Could not get user information.
# *Action: Check the user name and try again.

101, HY000, utERR_ABORT_aexport_Property_Error =  Invalid property value (<0%s>).
# *Cause: An invalid property value was assigned, or the value was omitted.
# *Action: Check the property value.

102, HY000, utERR_ABORT_Property_Loading_Error =  Property File Loading Error.(<0%s>)
# *Cause: An invalid or nonexistent property path was specified.
# *Action: Check the property file.


############# AUDIT ##############

200, HY000, utERR_ABORT_AUDIT_Alloc_Handle_Error = Can't get the Handle from <0%s>
# Altibase could not initialize SQLEnv; an OCIEnv handle could not be allocated.
# *Cause: The application failed to allocate memory for an environment handle with SQLAllocEnv. 
# *Action: Verify that there is enough memory to run the application. If necessary, make more memory available for the application.

201, HY000, utERR_ABORT_AUDIT_Already_Conn_Error = Already connected to server.
# Connection already initialized!
# *Cause: The application attempted to use an uninitialized connection handle.
# *Action: This is possibly a programming error. Check the error number from the trace log and contact Altibase's Support Center (http://support.altibase.com). 

202, HY000, utERR_ABORT_AUDIT_Create_Instance_Error = Failed to create object <0%s>
# Memory metaColumns error!
# *Cause: The application failed to allocate memory to create an object.
# *Action: Verify that there is enough memory to run the application. If necessary, make more memory available for the application. 

#203, HY000, utERR_ABORT_AUDIT_Plugin_Load_Error = Failed to load the Oracle handle <0%s>
# ERROR: Plugin load error
# *Cause: Failed to load the Oracle handle.
# *Action:

204, HY000, utERR_ABORT_AUDIT_Check_Log_File_Error = Failed to open the log file <0%s>
# ERROR: %s, Check your log dir and log file properties.
# *Cause: Failed to open the log file.
# *Action: Check the log file designated in the properties and the permissions mode of the log file.

205, HY000, utERR_ABORT_AUDIT_Wrong_Parameter_Error = A parameter is incorrect
# ERROR: A parameter is incorrect.
# *Cause: An invalid parameter was used.
# *Action: Check the parameters.

206, HY000, utERR_ABORT_AUDIT_Not_Found_Parameter_Error = Invalid Property Name <0%s> Assigned.
# ERROR: Properties parameter (%s) is not defined.
# *Cause: An invalid property name has been defined.
# *Action: Check the property name in the property file.

207, HY000, utERR_ABORT_AUDIT_ERANGE_Error = The value of the properties parameter <0%s> was out of range
# ERROR: The value of the properties parameter %s was out of range.
# *Cause: The parameter's value is incorrect.
# *Action: Check the value of the parameter.

208, HY000, utERR_ABORT_AUDIT_EINVAL_Error = Properties parameter <0%s> no conversion was performed, wrong parameter
# ERROR:Properties parameter %s  no conversion was performed, wrong parameter
# *Cause: The parameter was incorrect.
# *Action: Check the parameter.

209, HY000, utERR_ABORT_AUDIT_NULL_Parameter_Error = Undefined Property <0%s>.
# ERROR: The properties parameter %s key is NULL.
# *Cause: Property not defined in the property file.
# *Action: Check the property file.

210, HY000, utERR_ABORT_AUDIT_Invalid_Parameter_Error = An invalid property value <0%s> was defined.
# Invalid property: %s
# *Cause: An invalid property value was defined in the property file.
# *Action: Check the property value.

211, HY000, utERR_ABORT_AUDIT_Wrong_URL_Error = Invalid DBMS URL.
# ERROR[ PROP ] Incorrect DBMS connection URL.
# *Cause: The URL for connection to the DBMS is incorrect.
# *Action: Check the URL.

212, HY000, utERR_ABORT_AUDIT_Alloc_Memory_Error = Memory Allocation failed.
# ERROR[ PROP ] Could not allocate memory
# *Cause: The application failed to allocate memory.
# *Action: Verify that there is enough memory to run the application. If necessary, make more memory available for the application. 

213, HY000, utERR_ABORT_AUDIT_Different_Column_Error = The column counts for (<0%s>) and (<1%s>) are different.
# The tables (%s,%s) have different column counts.
# *Cause: The master and slave have different table column counts.
# *Action: Ensure that the table schema for the master and slave are the same.

214, HY000, utERR_ABORT_AUDIT_DIFF_File_Open_Error = Failed to open <0%s> diff file <1%s>.
# utScanner can't open DIFF log file:'%s'.
# *Cause: Failed to open log file.
# *Action: Check the log file name and permissions mode of the files.

215, HY000, utERR_ABORT_AUDIT_PK_Different_Order_Error = The order of the primary key on the master(<0%s>) is different from that on the slave (<1%s>).
# The master (%s) and slave (%s) have different primary key orders.
# *Cause: The primary key orders on the master and slave are different.
# *Action: Ensure that the table schema for the master and slave are the same.

216, HY000, utERR_ABORT_AUDIT_Master_No_PK_Error = No primary key column exists (<0%s>:<1%s>)
# The tables (%s,%s) have no primary keys.
# *Cause: The master table does not have a primary key column.
# *Action: Change the master table so that is has a primary key column. 

217, HY000, utERR_ABORT_AUDIT_Different_PK_Columns_Error = The PK column count for (<0%s>) is different than that for (<1%s>)
# The primary key structures for the tables(%s,%s) are different.
# *Cause: The number of primary key columns in the master table is different from that in the slave table.
# *Action: Ensure that the table schema for the master and slave are the same.

218, HY000, utERR_ABORT_AUDIT_PK_CanNotBe_EXCLUDE_Error = Primary key columns cannot be defined as EXCLUDE columns (<0%s>)
# Exclusion column cannot be primary key field '%s'.
# *Cause: Primary key columns cannot be defined as EXCLUDE columns.
# *Action: Check the EXCLUDE property value.

219, HY000, utERR_ABORT_AUDIT_NOT_Fount_Column_EXCLUDE_Error = Invalid column defined in EXCLUDE property (<0%s>)
# Exclusion column '%s' is not found.
# *Cause: An invalid column was defined in the property file's EXCLUDE property.
# *Action: Check the EXCLUDE property value.

220, HY000, utERR_ABORT_AUDIT_Thread_Join_Error = thread_join() <0%d> failed.
# ERROR [ MNG ] Join #%d failure!
# *Cause: The application encountered an error while joining threads. 
# *Action: This is possibly a programming error. Check the error number from the trace log and contact Altibase's Support Center (http://support.altibase.com). 

221, HY000, utERR_ABORT_AUDIT_Connect_TO_Master_Error = Failed to connect to master <0%s> server
# Cannot connect to master server: '%s'
# *Cause: Failed to connect to master server.
# *Action: Check the connection information.

222, HY000, utERR_ABORT_AUDIT_Connect_TO_Slaver_Error = Failed to connect to slave <0%s> server
# Cannot connect to slave server: '%s'
# *Cause: Failed to connect to slave server.
# *Action: Check the connection information.

223, HY000, utERR_ABORT_openFileError = Could not open file (<0%s>)
# *Cause: The file does not exist, or does not have enough permissions mode.
# *Action: Check the permissions mode of the file and whether the file exists.

224, HY000, utERR_ABORT_sysdba_connect_Error = You cannot connect to the server remotely with the sysdba option.
# *Cause: You cannot connect to a server remotely with the sysdba option.
# *Action: Check the SERVER and PORT_NO options.

225, HY000, utERR_ABORT_AUDIT_Managing_Thread_Filemode = Thread management failed for the file mode with table <0%s> and <1%s>.
# Thread management is failed.
# *Cause: The application encountered an error while handling a thread. 
# *Action: This is possibly a programming error. Check the error number from the trace log and contact Altibase's Support Center (http://support.altibase.com). 

226, HY000, utERR_ABORT_AUDIT_File_Open_4CSVfile = Could not create or open the <0%s> csv file.
# ERROR: %s, check your csv file properties and disk.
# *Cause: An attempt to create or open a csv file failed.
# *Action: Check the file system disk and try again.

227, HY000, utERR_ABORT_AUDIT_File_Close_4CSVfile = Could not close the <0%s> csv file.
# ERROR: %s, check your csv file properties and disk.
# *Cause: An attempt to close a csv file failed.
# *Action: Check the file system disk and try again.

228, HY000, utERR_ABORT_AUDIT_File_Write_4CSVfile = Could not write to the <0%s> csv file.
# ERROR: %s, check your csv file properties and disk.
# *Cause: An attempt to write to a csv file failed.
# *Action: Check the file system disk and try again.

229, HY000, utERR_ABORT_AUDIT_Cursor_Close = A cursor was not closed correctly.
# *Cause: A cursor was not closed correctly.
# *Action: Check the statement.

230, HY000, utERR_ABORT_AUDIT_File_Read_4CSVfile = Could not read the <0%s> csv file.
# ERROR: %s, check your csv file properties and disk.
# *Cause: An attempt to read a csv file failed.
# *Action: Check the file system disk and try again.

231, HY000, utERR_ABORT_AUDIT_Wrong_CSV_Format = Wrong CSV file format. (<0%s>)
# ERROR: %s, check your csv file content.
# *Cause: The attempt to convert data from the CSV file format to the original data format failed.
# *Action: Check the csv file and try again.

232, HY000, utERR_ABORT_AUDIT_CSV_Token_Buffer_Overflow = Buffer overflow occurred during CSV format conversion. (data:'<0%s>')
# ERROR: %s, check your csv file content.
# *Cause: The attempt to convert data from the CSV file format to the original data format failed.
# *Action: Check the csv file and try again.

316, HY000, utERR_ABORT_Table_Not_Found_Error = The table (<0%s>.<1%s>) specified in the configuration file does not exist on the <2%s> database.
# *Cause: The specified table does not exist in the database.
# *Action: Check to see if the specified table name located in the altiComp configuration file is correct.

############# ILOADER API ##############
264, HY000, utERR_ABORT_UPLOAD_Error = Could not upload the entire data file.
# Cannot upload file
# *Cause: The data file appears to be invalid.
# *Action: Check the data file and try again.

265, HY000, utERR_ABORT_LIB_VERSION_Error = An iLoader library version incompatibility error occurred.
# *Cause: An iLoader library version incompatibility error occurred.
# *Action: Check the iLoader library version.

266, HY000, utERR_ABORT_THREAD_Error = Could not create a new thread.
# *Cause: A platform kernel limit pertaining to the number of threads or memory usage was exceeded.
# *Action: Decrease the number of threads.

267, HY009, utERR_ABORT_INVALID_USE_OF_NULL_POINTER = Invalid use of null pointer. (<0%s>)
# *Cause: A null pointer was used. 
# *Action: Check the pointer and try again.

268, HY000, utERR_ABORT_OPTION_VERSION_Error = A library option version incompatibility error occurred.
# *Cause: Different library option versions are in use.
# *Action: Check the option versions.

############# CHECKSERVER API ##############

269, HY092, utERR_ABORT_INVALID_ATTR_OPTION = Invalid attribute/option identifier : <0%d>.
# *Cause: An invalid attribute/option identifier was used.
# *Action: Send a bug report to the vendor

270, HY000, utERR_ABORT_Unsupported_Feature = Unsupported feature : <0%s>
# *Cause: This feature is not supported.
# *Action: Do not use this feature.

271, HY000, utERR_ABORT_MsgLog_Init_Error = Unable to initialize message log file.
# *Cause: Unable to Initialize a message log file.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

272, HY000, utERR_ABORT_LockFile_Init_Error = Unable to initialize file lock.[<0%s>]
# *Cause: An attempt to initialize a file lock failed.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

273, HY000, utERR_ABORT_LockFile_Destroy_Error = Unable to destroy file lock.[<0%s>]
# *Cause:An attempt to destroy a file lock failed.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

274, HY000, utERR_ABORT_LockFile_Not_Exist = File to be locked does not exist.[<0%s>]
# *Cause: The file to be locked does not exist.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

275, HY000, utERR_ABORT_FileLock_Hold_Error = Could not hold a lock on a file.
# *Cause: A lock on a file could not be held.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

276, HY000, utERR_ABORT_FileLock_Release_Error = Could not release a lock on a file.
# *Cause: A lock on a file could not be released.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

277, HY000, utERR_ABORT_Failed_To_Check_Server_Running = Could not obtain server status; possible socket problem.
# *Cause: The server status could not be determined.
# *Action: Check for a socket problem.

278, HY000, utERR_ABORT_PidFile_Failed_To_Check_Exist = Unable to verify the existence of pid file : <0%s>
# *Cause: The existence of a pid file could not be verified.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

279, HY000, utERR_ABORT_PidFile_Exist = Pid file (<0%s>) exists.
# *Cause: The pid file already exists.
# *Action: Check the checkServer(xdbcheckServer for XDB) process and then run killCheckServer(xdbkillCheckServer for XDB). Alternatively, delete the pid file and run checkServer.

280, HY000, utERR_ABORT_PidFile_Create_Error = Unable to create the pid file [rc:<0%d>]
# *Cause: The pid file could not be created.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

281, HY000, utERR_ABORT_PidFile_Remove_Error = Unable to remove the pid file [rc:<0%d>]
# *Cause: The pid file could not be removed.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

282, HY000, utERR_ABORT_PidFile_Open_Error = Unable to open the pid file [rc:<0%d>]
# *Cause: The pid file could not be opened.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

283, HY000, utERR_ABORT_PidFile_Close_Error = Unable to close pid file [rc:<0%d>]
# *Cause: The pid file could not be closed.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

284, HY000, utERR_ABORT_PidFile_Read_Error = pid structure read error [rc:<0%d>]
# *Cause: The pid could not be read from the pid file.
# *Action: Check that the path and filename are correct, that suitable permissions have been granted, and that the pid file is valid.

285, HY000, utERR_ABORT_PidFile_Write_Error = pid structure write error [rc:<0%d>]
# *Cause: The pid could not be written to the pid file.
# *Action: Check that the path and filename are correct and that suitable permissions have been granted.

286, HY000, utERR_ABORT_Process_Kill_Error = Unable to kill the process [pid:<0%d>]
# *Cause: An attempt to kill a process failed.
# *Action: Check whether the process has already been killed.

288, HY010, utERR_ABORT_FUNCTION_SEQUENCE_ERR = Function sequence error.
# *Cause: The order in which function calls were made is invalid.
# *Action: Refer to the state transition table to determine the correct function call order.

289, HY000, utERR_FATAL_Signal = Signal Operation Error.(<0%s>)
# *Cause: An internal call to a signal function returned an error.
# *Action: Please send a bug report to the vendor.

302, HY000, utERR_ABORT_DAEMON_SUPPORT_ERR = The daemon process supports only tcp connection.
# *Cause: The daemon process supports only tcp connection.
# *Action: Please reconnect to the daemon process with tcp connection.

###### AEXPORT #########
290, HY000, utERR_ABORT_NOT_EXIST_TABLESPACE_ERR = The default tablespace for the user <0%s> does not exist.
# *Cause: The tablespace that was set as the default for this user has been modified or deleted.
# *Action: Set the default tablespace for this user and run aexport(xdbaexport for XDB) again.

299, HY000, utERR_ABORT_Insufficient_Priv_Error = Needs the CREATE TABLE privilege to analyze object dependencies.
# *Cause: The user does not have the CREATE TABLE privilege to analyze object dependencies.
# *Action: Grant the CREATE TABLE or CREATE ANY TABLE privileges to the user and re-execute aexport.

300, HY000, utERR_ABORT_Db_Charset_Fetch_Error = Unable to fetch the database character set.
# *Cause: Failed to fetch the database character set.
# *Action: Set the -NLS_USE parameter or the ALTIBASE_NLS_USE environment variable and re-execute aexport.

301, HY000, utERR_ABORT_Repl_Offline_Log_Path_Fetch_Error = Unable to fetch log directory paths for the replication <0%s> in offline mode.
# *Cause: Failed to fetch log directory paths for the replication in offline mode.
# *Action: This is possibly meta inconsistency. Contact Altibase's Support Center (http://support.altibase.com)

###### altiProfile #########
303, HY000, utERR_ABORT_INVALID_FMT = Invalid file format
# *Cause: Failed to read from the profiling file.
# *Action: Check the versions of the profiling file and the altiProfile tool.

304, HY000, utERR_ABORT_FILE_OPEN = Unable to open file (<0%s>)
# *Cause: The path or filename is invalid.
# *Action: Verify that the path and filename are correct and that you have read/write permission for the file.

305, HY000, utERR_ABORT_FILE_CLOSE = Unable to close file (<0%d>)
# *Cause: An invalid file handle was specified.
# *Action: Memory corruption may occur. Check the error number and contact Altibase’s Support Center (http://support.altibase.com).

306, HY000, utERR_ABORT_INVALID_HANDLE = Invalid Handle
# *Cause: NULL handle was specified.
# *Action: Memory corruption may occur. Check the error number and contact Altibase’s Support Center (http://support.altibase.com).

###### ALTIWRAP ######
309, HY000, utERR_ABORT_INVALID_ALTIWRAP_COMMAND = Invalid command arguments.
# *Cause: Invalid command arguments.
# *Action: Verify that the command arguments are valid.

310, HY000, utERR_ABORT_MEMORY_ALLOCATION = Insufficient memory for PSM Query Encryption.
# *Cause: Insufficient memory.
# *Action: Verify that the system has enough available memory.

311, HY000, utERR_ABORT_TOO_LONG_PATH_NAME = The path name was too long; the maximum length is <0%d>.
# *Cause: The path name was too long.
# *Action: Shorten the path name.
