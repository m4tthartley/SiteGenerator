
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define KiloBytes(Amount) (Amount*1024)
#define MegaBytes(Amount) (Amount*1024*1024)
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t s32;
typedef s32 b32;

#define TRUE 1
#define FALSE 0

#define Assert(Expression) if (!(Expression)) {*((char*)(0)) = 0;}

#define forc(Count) for (s32 i = 0;\
						 i < Count;\
						 ++i)
#define fori(Count, Index) for (s32 Index = 0;\
								Index < Count;\
								++Index)

#define fiz(size) for (s32 i = 0;\
					   i < size;\
					   ++i)
#define fjz(size) for (s32 j = 0;\
					   j < size;\
					   ++j)
#define fkz(size) for (s32 k = 0;\
					   k < size;\
					   ++k)

#define FILE_LIST_MAX 64
#define FILE_LIST_STRING_MAX 64

typedef struct
{
	// TODO: Use pointer and PushMemory for this
	char name[FILE_LIST_STRING_MAX];
	FILETIME writeTime;
	// u64 WriteTime; // for linux
} file;
typedef struct
{
	file files[FILE_LIST_MAX];
	s32 count;
} file_list;

typedef struct
{
	char *Data;
	u32 Size;
} file_data;

void Error (char *Msg)
{
	printf("Critical Error: %s \n", Msg);
}

LARGE_INTEGER GetPerformanceFrequency ()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq;
}

LARGE_INTEGER globalPerformanceFrequency = GetPerformanceFrequency();

char *Memory;
u32 MemorySize = MegaBytes(100);
u32 MemoryUsed = 0;

char *PushMemory (u32 Size)
{
	if (MemoryUsed + Size > MemorySize)
	{
		Error("Ran out of memory");
	}
	
	char *Result = Memory + MemoryUsed;
	MemoryUsed += Size;
	return Result;
}

void ClearMemory ()
{
	memset(Memory, 0, MemorySize);
	MemoryUsed = 0;
}

void InitMemory ()
{
	Memory = (char*)malloc(MemorySize);
	ClearMemory();
}

file_data FRead (char *FileName)
{
	FILE *FileHandle = fopen(FileName, "rb");
	// char *FileData = NULL;
	file_data fd = {0};
	if (!FileHandle)
	{
		// Error("Unable to open file");
		// Error(FileName);
		// printf(strerror(errno));
	}
	else
	{
		fseek(FileHandle, 0, SEEK_END);
		fd.Size = ftell(FileHandle);
		fseek(FileHandle, 0, SEEK_SET);

		fd.Data = PushMemory(fd.Size);
		fread(fd.Data, sizeof(char), fd.Size, FileHandle);
		fclose(FileHandle);
	}

	return fd;
}

b32 FExists (char *FileName)
{
	FILE *h = fopen(FileName, "rb");
	if (h)
	{
		fclose(h);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

file_data Win32ReadFile (char *FileName)
{
	file_data fd = {0};

	HANDLE FileHandle = CreateFileA(FileName,
		GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize64;
		if (GetFileSizeEx(FileHandle, &FileSize64))
		{
			u32 FileSize = (u32)FileSize64.QuadPart;
			// fd.Data = (char*)VirtualAlloc(0, FileSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			fd.Data = (char*)PushMemory(FileSize);
			if (fd.Data)
			{
				DWORD BytesRead;
				if (ReadFile(FileHandle, fd.Data, FileSize, &BytesRead, 0)
					&& FileSize == BytesRead)
				{
					fd.Size = FileSize;
				}
				else
				{
					// VirtualFree(fd.Data, 0, MEM_RELEASE);
					fd.Data = 0;
				}
			}
		}

		CloseHandle(FileHandle);
	}

	return fd;
}

// HANDLE GetFirstDirFile ()
// {

// }

// void GetNextDirFile (char *WildCard)
// {
// 	// char *WildCard = "*.html";
// 	WIN32_FIND_DATAA FindData;
// 	HANDLE FileHandle = FindFirstFileA(WildCard, &FindData);
// }

file_list GetFileList (char *WildCard)
{
	file_list fileList = {0};

	// char *WildCard = "posts/*.html";
	WIN32_FIND_DATAA FindData;
	HANDLE FileHandle = FindFirstFileA(WildCard, &FindData);
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		while (TRUE)
		{
			strcpy(fileList.files[fileList.count].name, FindData.cFileName);
			fileList.files[fileList.count].writeTime = FindData.ftLastWriteTime;
			++fileList.count;

			// DWORD fileAttributes = GetFileAttributes(FindData.cFileName);
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				int x = 0;
			}

			if (!FindNextFileA(FileHandle, &FindData))
			{
				break;
			}
		}

		FindClose(FileHandle);
	}

	return fileList;
}

file_list ConcatFileList (file_list fl0, file_list fl1)
{
	file_list fileList = {0};
	forc (fl0.count)
	{
		strcpy(fileList.files[fileList.count].name, fl0.files[i].name);
		fileList.files[fileList.count].writeTime = fl0.files[i].writeTime;
		++fileList.count;
	}
	forc (fl1.count)
	{
		strcpy(fileList.files[fileList.count].name, fl1.files[i].name);
		fileList.files[fileList.count].writeTime = fl1.files[i].writeTime;
		++fileList.count;
	}

	return fileList;
}

struct perf_t {
	u64 time;
};

perf_t GetPerfTime ()
{
	perf_t perf = {};
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

	perf.time = time.QuadPart;
	return perf;
}

float SecondsElapsed (perf_t perf)
{
	perf_t newPerf = GetPerfTime();
	float diff = (float)(newPerf.time - perf.time);
	float seconds = diff / (float)globalPerformanceFrequency.QuadPart;
	return seconds;
}

void TestRecursiveFileList ()
{
	file_list fileList = {0};

	WIN32_FIND_DATAA FindData;
	HANDLE FileHandle = FindFirstFileA("*", &FindData);
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		while (TRUE)
		{
			strcpy(fileList.files[fileList.count].name, FindData.cFileName);
			fileList.files[fileList.count].writeTime = FindData.ftLastWriteTime;
			++fileList.count;

			// DWORD fileAttributes = GetFileAttributes(FindData.cFileName);
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				int x = 0;
			}

			if (!FindNextFileA(FileHandle, &FindData))
			{
				break;
			}
		}

		FindClose(FileHandle);
	}

	return fileList;
}
