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
 
#include <idl.h>
#include <ida.h>
#include <ideErrorMgr.h>
#include <idErrorCode.h>
#include <ideMsgLog.h>
#include <iduVersion.h>

#define SHM_TEST_COUNT 10
#define ERR_EXIT(message) fprintf(stderr, "%s\n", message);exit(-1);
#define	ERR_PRINT(message) fprintf(stderr, "%s\n", message);

void press_any_key()
{
	char flag[3];
	idlOS::printf("PRESS ANY KEY !!\n");
    idlOS::gets(flag, 2);
}

void print_last_error()
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	printf( "----------WIN32 LAST ERROR ---------\n");
	printf( "%s", (LPCTSTR)lpMsgBuf);
	// Free the buffer.
	LocalFree( lpMsgBuf );
}


/*
to test
	idlOS::kill
	idlOS::fsync

*/
#define TEST_FILE_NAME "altibase_home"IDL_FILE_SEPARATORS"logs"IDL_FILE_SEPARATORS"a10byte.file"
#define TEST_FILE_SIZE 10
/*
if (hMap != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
{
    CloseHandle(hMap);
    hMap = NULL;
}
*/

int file_io_test_win32()
{
	HANDLE file_handle, file_mapping;
	void *addr_mapping;

	file_handle = ::CreateFile(	TEST_FILE_NAME,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	IDE_TEST_RAISE(file_handle == PDL_INVALID_HANDLE, error_create_file)


	file_mapping = ::CreateFileMapping (file_handle,
                                         PDL_OS::default_win32_security_attributes (NULL),
                                         PAGE_READWRITE,
                                         0,
                                         0,
                                         0);
	IDE_TEST_RAISE(file_mapping == 0, error_create_file_mapping)

	addr_mapping = ::MapViewOfFile (file_mapping,
                                        FILE_MAP_WRITE,
                                        0,
                                        0,
                                        TEST_FILE_SIZE);
	IDE_TEST_RAISE(addr_mapping == NULL, error_map_view_of_file)

	IDE_TEST_RAISE(!UnmapViewOfFile(addr_mapping), error_unmap_view_of_file)

	IDE_TEST_RAISE(!CloseHandle(file_mapping), error_close_mapping_handle)

	IDE_TEST_RAISE(!CloseHandle(file_handle), error_close_file_handle)

	return IDE_SUCCESS;

	IDE_EXCEPTION(error_create_file);
	{
		ERR_PRINT("CreateFile")
	}
	IDE_EXCEPTION(error_create_file_mapping);
	{
		ERR_PRINT("CreateFileMapping")
	}
	IDE_EXCEPTION(error_map_view_of_file);
	{
		ERR_PRINT("MapViewOfFile")
	}
	IDE_EXCEPTION(error_unmap_view_of_file);
	{
		ERR_PRINT("UnmapViewOfFile")
	}
	IDE_EXCEPTION(error_close_mapping_handle);
	{
		ERR_PRINT("CloseHandle(file_mapping)")
	}
	IDE_EXCEPTION(error_close_file_handle);
	{
		ERR_PRINT("CloseHandle(file_handle)")
	}
	IDE_EXCEPTION_END;
	return IDE_FAILURE;
}

int file_io_test() {
	PDL_HANDLE fd;
	SChar *base;
	SInt rc;

    fd = idlOS::open( TEST_FILE_NAME, O_RDWR);
    IDE_TEST_RAISE(fd == IDL_INVALID_HANDLE, error_open);

    base = (SChar *)idlOS::mmap(0, TEST_FILE_SIZE, PROT_READ|PROT_WRITE,
                    MAP_SHARED, fd, 0);
    IDE_TEST_RAISE(base == MAP_FAILED, error_mmap);


    rc = idlOS::munmap(base, TEST_FILE_SIZE);
    IDE_TEST_RAISE(rc != 0,  error_munmap);

    rc = idlOS::close(fd);
    IDE_TEST_RAISE(rc != 0,  error_close);


	return IDE_SUCCESS;

    IDE_EXCEPTION(error_open);
    {
		ERR_PRINT("idlOS::open")
	}
    IDE_EXCEPTION(error_mmap);
    {
		ERR_PRINT("idlOS::mmap")
	}
    IDE_EXCEPTION(error_munmap);
    {
		ERR_PRINT("idlOS::munmap")
	}
    IDE_EXCEPTION(error_close);
    {
		ERR_PRINT("idlOS::close")
	}
    IDE_EXCEPTION_END;
	return IDE_FAILURE;
}

const int SHM_DUMMY_SIZE = 1048576;
typedef struct
{
	SInt field1;
	SInt field2;
	SChar dummy[SHM_DUMMY_SIZE];
} shm_test_dummy_t;

const int SHM_SIZE = sizeof(shm_test_dummy_t);
const int SHM_KEY = 999;
const int SHM_FV1 = 403;
const int SHM_FV2 = 2848;
#define SHM_DUMMYV "I'm a boy. You are a girl?"


int shm_test()
{
	SInt i;
	SInt rc;
	PDL_HANDLE shmId;
	PDL_HANDLE secShmId;
	shm_test_dummy_t *shmPtr1;
	shm_test_dummy_t *shmPtr2;
	shm_test_dummy_t *secShmPtr;
	shm_test_dummy_t tstDummy;

	// 공유메모리 생성
	shmId = idlOS::shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
	IDE_TEST_RAISE(shmId==PDL_INVALID_HANDLE, shmget_error);

	// 공유메모리 중복 (-1 나와야함)
	rc = idlOS::shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
	IDE_TEST_RAISE(rc!=-1, duplicate_shmget_not_detected_error);


	shmPtr1 = (shm_test_dummy_t*)idlOS::shmat(shmId, 0, 0666 );
	IDE_TEST_RAISE(shmPtr1 == NULL, shm_at_error);

	shmPtr1->field1 = SHM_FV1;
	for (i=0; i<SHM_DUMMY_SIZE; i++)
	{
		shmPtr1->dummy[i] = 'A' + i % 26;
	}
	shmPtr1->dummy[SHM_DUMMY_SIZE-1] = '\0';


	shmPtr2 = (shm_test_dummy_t*)idlOS::shmat(shmId, 0, 0666 | IPC_CREAT | IPC_EXCL);
	IDE_TEST_RAISE(shmPtr2 == NULL, shm_at_error2);

	shmPtr2->field2 = SHM_FV2;
	strncpy(shmPtr2->dummy, SHM_DUMMYV, strlen(SHM_DUMMYV));

	memcpy(&tstDummy, shmPtr2, sizeof(tstDummy));

	// 공유메모리 생성
	secShmId = idlOS::shmget(SHM_KEY, SHM_SIZE, 0666 );
	IDE_TEST_RAISE(secShmId==PDL_INVALID_HANDLE, sec_shmget_error);


	secShmPtr = (shm_test_dummy_t*)idlOS::shmat(secShmId, 0, 0666 );
	IDE_TEST_RAISE(secShmPtr == NULL, sec_shm_at_error);


	if (secShmPtr->field1 != tstDummy.field1 ||
		secShmPtr->field2 != tstDummy.field2 ||
		strcmp(	secShmPtr->dummy, tstDummy.dummy) != 0 )
	{
		ERR_PRINT("Shared Memory error [Invalid values found on shared memory]")
		printf("field1:[%d]/field2:[%d]/dummy:[%s]",
				secShmPtr->field1,
				secShmPtr->field2,
				secShmPtr->dummy );
		return IDE_FAILURE;
	}

	rc = idlOS::shmdt(secShmPtr);
	IDE_TEST_RAISE(rc == -1, sec_shmdt_error);

	rc = idlOS::shmdt(shmPtr1);
	IDE_TEST_RAISE(rc == -1, shmdt_error1);

	rc = idlOS::shmdt(shmPtr2);
	IDE_TEST_RAISE(rc == -1, shmdt_error2);

	rc = idlOS::shmctl(shmId, IPC_RMID, 0);
	IDE_TEST_RAISE(rc == -1, shmctl_error);

	return IDE_SUCCESS;

	IDE_EXCEPTION(shmget_error);
	{
		ERR_PRINT("shmget_error")
	}
	IDE_EXCEPTION(duplicate_shmget_not_detected_error);
	{
		ERR_PRINT("duplicate_shmget_not_detected_error")
	}
	IDE_EXCEPTION(shm_at_error);
	{
		ERR_PRINT("shm_at_error")
	}
	IDE_EXCEPTION(shm_at_error2);
	{
		ERR_PRINT("shm_at_error2")
	}
	IDE_EXCEPTION(sec_shmget_error);
	{
		ERR_PRINT("sec_shmget_error")
	}
	IDE_EXCEPTION(sec_shm_at_error);
	{
		ERR_PRINT("sec_shm_at_error")
	}
	IDE_EXCEPTION(sec_shmdt_error);
	{
		ERR_PRINT("sec_shmdt_error")
	}
	IDE_EXCEPTION(shmdt_error1);
	{
		ERR_PRINT("shmdt_error1")
	}
	IDE_EXCEPTION(shmdt_error2);
	{
		ERR_PRINT("shmdt_error2")
	}
	IDE_EXCEPTION(shmctl_error);
	{
		ERR_PRINT("shmctl_error")
	}
	IDE_EXCEPTION_END;
	return IDE_FAILURE;
}

int main(int argc, char *argv[])
{

	// 아무일도 안하지만 에러는 나면 안된다.
	// hjohn modify
	if ( idlVA::daemonize(".", 0) != 0) {
		ERR_EXIT("set_handle_limit")
	}

	if ( idlVA::set_handle_limit() != 0 ) {
		ERR_EXIT("set_handle_limit")
	}

	SInt maxHandles = idlVA::max_handles();
	printf("max_handles -> %d\n", maxHandles);
	if ( maxHandles < 400 ) {
		ERR_EXIT("max_handles")
	}

	SLong space1 = idlVA::getDiskFreeSpace("altibase_home"IDL_FILE_SEPARATORS"logs");
	printf("getDiskFreeSpace(\"altibase_home"IDL_FILE_SEPARATORS"logs\") -> %ld\n", space1);
	if (space1 <= 0) {
		ERR_EXIT("getDiskFreeSpace(\"altibase_home"IDL_FILE_SEPARATORS"logs\")")
	}

	SLong space2 = idlVA::getDiskFreeSpace("altibase_home"IDL_FILE_SEPARATORS"logs"IDL_FILE_SEPARATORS"non_existing_file");
	printf("getDiskFreeSpace(\"altibase_home"IDL_FILE_SEPARATORS"logs"IDL_FILE_SEPARATORS"non_existing_file\") -> %ld\n", space2);
	if (space2 <= 0) {
		ERR_EXIT("getDiskFreeSpace(\"altibase_home"IDL_FILE_SEPARATORS"logs"IDL_FILE_SEPARATORS"non_existing_file\")")
	}

	SInt nProcessorCnt = idlVA::getProcessorCount();
	printf("processor count -> %d\n", nProcessorCnt);
	if (nProcessorCnt <=0) {
		ERR_EXIT("getProcessorCount")
	}

	struct tms tm;
	clock_t times_ret = idlOS::times( &tm );
	printf(	"times(tm) = %d / "
			"tm.tms_stime = %d / "
			"tm.tms_cstime = %d / "
			"tm.tms_utime = %d / "
			"tm.tms_cutime = %d \n "
			, (SInt)times_ret, tm.tms_stime, tm.tms_cstime, tm.tms_utime, tm.tms_utime, tm.tms_cutime );
	if (times_ret < (clock_t) 0) {
		ERR_EXIT("times")
	}


	if (file_io_test_win32() != IDE_SUCCESS) {
		ERR_EXIT("file_io_test_win32")
	}

	if (file_io_test() != IDE_SUCCESS) {
		ERR_EXIT("file_io_test")
	}

	for (int i=0; i<SHM_TEST_COUNT; i++)
	{
		printf("[%dth shared memory test]\n", i);
		if (shm_test() != IDE_SUCCESS)
		{
			ERR_EXIT("shm_test")
		}
	}

	idlOS::printf("SUCCESS !! \n");
	press_any_key();
	return 0;
}

