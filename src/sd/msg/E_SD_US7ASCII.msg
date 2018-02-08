# $Id: E_SD_US7ASCII.msg 32106 2009-04-13 01:53:28Z kimmkeun $
#
# Error Message File (used by genErrMsg)
#

# 에러코드 영역 번호
# 0 => id
# 1 => sm 
# 2 => mt 
# 3 => qp
# 4 => mm 
# 5 => ul
# 6 => rp 
# 9 => ut
# 10 => st
# 11 => lk
# 12 => dk
# 14 => sd

# 에러코드 할당 정책은 아래 위키 페이지를 참조하시기 바랍니다.
# http://wiki.altibase.in/wiki/index.php/Altimsg
# 브랜치에서 에러코드를 할당받으시면 안됩니다.
# 반드시 main trunk에서 번호를 할당받은 후 브랜치에서 사용하셔야 합니다.
# 의문 사항이 있으시면 AK팀 혹은 QP(품질본부)팀에 문의 부탁드립니다.
# Last Number = 999

SECTION = 14

###############################################################################
#########                 SDM ERROR CODE (1~)                         #########
###############################################################################

1,sdERR_ABORT_SDM_SHARD_META_NOT_CREATED = There is no shard meta <0%s>.
# *Cause: The shard meta has not been created.
# *Action: Create shard meta.

2,sdERR_ABORT_SDM_SHARD_NODE_OVERFLOW = There is overflow in the number of shard data nodes.
# *Cause: The shard data nodes are exceeded.
# *Action: Refer to the manual to verify the number of permissible shard data nodes.

3,sdERR_ABORT_SDM_SHARD_NODE_NOT_EXIST = The shard data node cannot be found.
# *Cause: The specified shard data node does not exist.
# *Action: Verify if the name of shard data node is correct.

4,sdERR_ABORT_SDM_SHARD_TABLE_NOT_EXIST = The shard object cannot be found.
# *Cause: The shard object does not exist.
# *Action: Verify if the shard object is correct.

5,sdERR_ABORT_SDM_SHARD_KEY_COLUMN_NOT_EXIST = The shard key <0%s>.<1%s>.<2%s> cannot be found.
# *Cause: The specified shard key does not exist.
# *Action: Verify if the shard key is correct.

6,sdERR_ABORT_SDM_UNSUPPORTED_SHARD_KEY_COLUMN_TYPE = The data type of shard key <0%s>.<1%s>.<2%s> is not supported.
# *Cause: The data type is not supported by the shard key.
# *Action: Verify if the data type used in the shard key is correct.

7,sdERR_ABORT_SDM_INVALID_RANGE_FUNCTION = The split method in the shard key does not correspond.
# *Cause: The split method in the shard key does not correspond.
# *Action: Verify if the split method is correct.

8,sdERR_ABORT_SDM_AREADY_EXIST_SHARD_OBJECT = The object already exists.
# *Cause: The object is a pre-existing shard object.
# *Action: Verify the object name.

9,sdERR_ABORT_SDM_SYSTEM_OBJECT = A shard object cannot be created with a meta object.
# *Cause: A shard object cannot be created with a meta object.
# *Action: Verify if the object privilege is correct.

10,sdERR_ABORT_SDM_CHECK_META_VERSION = Confirmation of shard version failed.
# *Cause: The shard version cannot be confirmed.
# *Action: Verify the shard version with the altibase-v command to see if the version is correct.

11,sdERR_ABORT_SDM_MISMATCH_META_VERSION = The shard version between meta node and data node is mismatched.
# *Cause: The shard version between meta and data nodes does not correspond.
# *Action: Verify the shard version with the altibase-v command to see if the version is correct.

13,sdERR_ABORT_SDM_DUPLICATED_RANGE_VALUE = The range value of shard key is duplicated.
# *Cause: The range value of shard key is duplicated.
# *Action: Verify if the range value of shard key is correct.

###############################################################################
#########                 SDA ERROR CODE (101~)                       #########
###############################################################################

101,sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD = The statement is not supported in Altibase sharding due to the following reason: <0%s>.
# *Cause: Altibase sharding does not support the statement.
# *Action: Verify if the statement is correct.

102,sdERR_ABORT_SDA_INVALID_SHARD_KEY_CONDITION = Invalid shard key value expression was used.
# *Cause: The expression of shard key value is invalid.
# *Action: Verify if the shard key expression is correct.

103,sdERR_ABORT_SDA_NOT_EXIST_SHARD_KEY_CONDITION = The shard key value cannot be found.
# *Cause: The shard key value does not exist.
# *Action: Verify if the shard key exists.

104,sdERR_ABORT_SDA_DATA_NODE_NOT_FOUND = The data node corresponding to the shard key cannot be found.
# *Cause: The data node that matches the shard key cannot be found due to inappropriate setting of shard tables.
# *Action: Verify the distribution setting or shard key value.

###############################################################################
#########                 SDF ERROR CODE (201~)                       #########
###############################################################################

201,sdERR_ABORT_SDF_INVALID_SHARD_NODE = Invalid shard data node was used.
# *Cause: The host IP and port number for shard data node cannot be found.
# *Action: Verify the host IP and port number for shard data node.

202,sdERR_ABORT_SDF_AREADY_EXIST_SHARD_NODE = The shard data node of identical IP and port already exists.
# *Cause: The shard data node of identical IP and port already exists.
# *Action: Verify the IP and port of the shard data node.

203,sdERR_ABORT_SDF_SHARD_USER_NAME_TOO_LONG = The object user name is too long.
# *Cause: The user name of a shard object is permitted to contain less than 128 characters.
# *Action: Verify the length of the object user name.

204,sdERR_ABORT_SDF_SHARD_TABLE_NAME_TOO_LONG = The object name is too long.
# *Cause: The shard object name is permitted to contain less than 128 characters.
#Action : Verify the length of the object name.

205,sdERR_ABORT_SDF_SHARD_NODE_NAME_TOO_LONG = The name of shard data node is too long.
# *Cause: The name of shard data node is permitted to contain less than 40 characters.
# *Action: Verify the length of the shard data node name.

206,sdERR_ABORT_SDF_SHARD_MAX_VALUE_TOO_LONG = The maximum value for shard split method is too large.
# *Cause: The maximum value for shard split method is too large.
# *Action: Refer to the manual to verify permissible range for the shard split method.

207,sdERR_ABORT_SDF_SHARD_KEYCOLUMN_NAME_TOO_LONG = The shard key name is too long.
# *Cause: The shard key name is permitted to contain less than 40 characters.
# *Action: Verify the length of the shard key name.

208,sdERR_ABORT_SDF_INVALID_SHARD_SPLIT_METHOD_NAME = The shard split method is invalid.
# *Cause: Invalid shard split method is used.
# *Action: Refer to the manual verify if the shard split method is correct.

209,sdERR_ABORT_SDF_INVALID_SHARD_TABLE = The specified object cannot be found.
# *Cause: The specified object does not exist.
# *Action: Retry after verifying the object and user name.

210,sdERR_ABORT_SDF_INVALID_RANGE_VALUE = The permissible range of shard key <0%s> is invalid.
# *Cause: The permissible range for the shard key is invalid.
# *Action: Refer to the manual to verify the permissible range for the shard key.

211,sdERR_ABORT_SDF_INVALID_SUB_SHARD_KEY_NAME = Invalid sub-shard key name.
# *Cause: Sub-shard key name and shard key name are the same.
# *Action: Verify if the name of sub-shard key is correct.

212,sdERR_ABORT_SDF_UNSUPPORTED_SUB_SHARD_KEY_SPLIT_TYPE = The split method of sub-shard key is not supported.
# *Cause: Unsupported sub-shard key split method.
# *Action: Verify that the split method of the sub-shard key.

213,sdERR_ABORT_SDF_UNSUPPORTED_SHARD_SPLIT_METHOD_NAME = The shard split method is not supported.
# *Cause: The shard split method is not supported.
# *Action: Verify that the split method name.

###############################################################################
#########                 SDL ERROR CODE (301~)                       #########
###############################################################################

301,sdERR_ABORT_SHARD_LIBRARY_ERROR = An error occurred in the library function call when executing <1%s> for shard data node <0%s>.
# *Cause: There is an error in library function call of shard data node.
# *Action: Verify the state of shard data node.

302,sdERR_ABORT_SHARD_LIBRARY_ERROR_1 = The following error occurs when <1%s> of shard data node <0%s> is performed.: <2%s>
# *Cause: An error occurred during the library function call of the shard data node.
# *Action: Verify the state of shard data node.

303,sdERR_ABORT_SHARD_LIBRARY_ERROR_2 = The following error occurs when <1%s> of shard data node <0%s> is performed.: <2%s><3%s>
# *Cause: An error occurred during the library function call of the shard data node.
# *Action: Verify the state of shard data node.

304,sdERR_ABORT_SHARD_LIBRARY_ERROR_3 = The following error occurs when <1%s> of shard data node <0%s> is performed.: <2%s><3%s><4%s>
# *Cause: An error occurred during the library function call of the shard data node.
# *Action: Verify the state of shard data node.

305,sdERR_ABORT_SHARD_LIBRARY_ERROR_4 = The following error occurs when <1%s> of shard data node <0%s> is performed.: <2%s><3%s><4%s><5%s>
# *Cause: An error occurred during the library function call of the shard data node.
# *Action: Verify the state of shard data node.

306,sdERR_ABORT_SHARD_LIBRARY_LINK_FAILURE_ERROR = The link failed when performing <1%s> on shard data node <0%s>.
# *Cause: The link on the shard data node failed.
# *Action: Verify the state of link on the shard data node.

307,sdERR_ABORT_INIT_SDL_ODBCCLI = The library initialization failed and the following error occurred: <0%s>
# *Cause: Library initialization failed.
# *Action: Verify the library of shard meta node.

308,sdERR_ABORT_EXECUTE_NULL_DBC = The connection cannot be found when shard data node <0%s> is <1%s>.
# *Cause: The connection state of shard data node does not exist.
# *Action: Verify the connection state.

309,sdERR_ABORT_EXECUTE_NULL_STMT = The statement cannot be found when shard data node <0%s> is <1%s>.
# *Cause: The statement in the shard data node does not exist.
# *Action: Verify the statement in the shard data node.

310,sdERR_ABORT_UNINITIALIZED_LIBRARY = Shard data node <0%s> fails to perform <1%s> because the library was not initialized.
# *Cause: The library of shard meta node was not initialized.
# *Action: Restart the server after verifying the library of shard meta node.

311,sdERR_ABORT_DBCLINK_ALLOC = <1%s> on shard data node <0%s> failed.
# *Cause: There is insufficient memory for the shard data node connection.
# *Action: Verify the memory usage.

312,sdERR_ABORT_SHARD_XA_LIBRARY_ERROR = An error occurred in the library function call when executing <1%s> for shard <0%s>.
# *Cause: There is an error in library function call of shard xa function
# *Action: Verify the state of shard library.

###############################################################################
#########                 SDP ERROR CODE (401~)                       #########
###############################################################################

401,sdERR_ABORT_SDPJ_SYNTAX = JSON syntax error
# *Cause: The unusable reserved words or delimiter inapplicable was used. Or the reserved word cannot be used.
# *Action: Refer to JSON format.

402,sdERR_ABORT_SDPJ_ALLOC = JSON parsering failed at <0%d>% due to insufficient memory buffer.
# *Cause: There is insufficient memory buffer in JSON parser.
# *Action: Verify the memory buffer size.

403,sdERR_ABORT_SDPJ_CONVERT = Fail to convert the condition to shard analyze infomation. (<0%s>)
# *Cause: The condition does not contain shard information or is incorrect.
# *Action: Verify the condition.

###############################################################################
#########                 SDI ERROR CODE (801~)                       #########
###############################################################################

801,sdERR_ABORT_SDI_SHARD_LINKER_NOT_INITIALIZED = The meta connection cannot be initialized.
# *Cause: The meta connection was not initialized.
# *Action: Verify the setting of shard meta and data is correct.

802,sdERR_ABORT_SDI_INCOMPLETE_RANGE_SET = The shard key range of <0%s>.<1%s> is invalid.
# *Cause: The shard key range is invalid.
# *Action: Verify the key range for shard split method.

803,sdERR_ABORT_SDI_NOT_EXIST_SHARD_ANALYSIS = The result of shard analysis does not exist.
# *Cause: The result of shard analysis does not exist.
# *Action: Verify the distribution setting or shard key value.

804,sdERR_ABORT_SDI_DATA_NODE_NOT_FOUND = The data node corresponding to the shard key cannot be found.
# *Cause: The data node that matches the shard key cannot be found due to inappropriate setting of shard tables.
# *Action: Verify the distribution setting or shard key value.

805,sdERR_ABORT_SDI_DUPLICATED_NODE_NAME = Duplicate node name <0%s>
# *Cause: Duplicate node name
# *Action: Verify that no duplicate node names are specified.

806,sdERR_ABORT_SDI_INVALID_NODE_NAME = Invalid node name <0%s>
# *Cause: Invalid node name
# *Action: Verify that the node name is valid.

807,sdERR_ABORT_SDI_INVALID_NODE_NAME2 = Invalid node name: <0%s>
# *Cause: Invalid node name
# *Action: Verify that the node name is valid.

###############################################################################
#########                 SDC ERROR CODE (901~)                       #########
###############################################################################

901,sdERR_ABORT_EXIST_SHARD_TABLE_OUTSIDE_SHARD_VIEW = The shard table is only available within the shard view.: <0%s>
# *Cause: The shard table is only available within the shard view.
# *Action: Rewrite the shard query.

902,sdERR_ABORT_INVALID_SHARD_QUERY = <0%s> <1%s>
# *Cause: The shard query is invalid.
# *Action: Rewrite the shard query.

903,sdERR_ABORT_UNSUPPORTED_SHARD_DATA_IN_DML = SHARD_DATA feature is not supported in DML statements
# *Cause: The shard query is invalid.
# *Action: Rewrite the shard query.

999,sdERR_ABORT_SDC_UNEXPECTED_ERROR = Unexpected errors have occurred.: <0%s>: <1%s>
# *Cause: An unexpected error has occurred.
# *Action: Verify the error number in the trace log file and contact Altibase Support Center (http://support.Altibase.com).

INTERNAL_TRACE_MESSAGE_BEGIN

INTERNAL_TRACE_MESSAGE_END

