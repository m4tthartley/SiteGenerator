
#include <stdio.h>

#define KiloBytes(Amount) (Amount*1024)
#define MegaBytes(Amount) (Amount*1024*1024)
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint8_t u8;
typedef int32_t s32;
typedef s32 b32;

#define Assert(Expression) if (!(Expression)) {*((char*)(0)) = 0;}

#define forc(Count) for (s32 i = 0;\
						 i < Count;\
						 ++i)
#define fori(Count, Index) for (s32 Index = 0;\
								Index < Count;\
								++Index)

#define DIRECTORY_LIST_MAX 64
#define DIRECTORY_LIST_STRING_MAX 64

typedef struct
{
	char Name[DIRECTORY_LIST_STRING_MAX];
	FILETIME WriteTime;
} directory_file;
typedef struct
{
	directory_file Files[DIRECTORY_LIST_MAX];
	s32 FileCount;
} directory_list;

typedef struct
{
	char *Data;
	u32 Size;
} file_data;

void Error (char *Msg)
{
	printf("Critical Error: %s \n", Msg);
}

u8 *Memory;
u32 MemorySize = MegaBytes(1);
u32 MemoryUsed = 0;

u8 *PushMemory (u32 Size)
{
	if (MemoryUsed + Size > MemorySize)
	{
		Error("Ran out of memory");
	}
	
	u8 *Result = Memory + MemoryUsed;
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
	Memory = malloc(MemorySize);
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
			fd.Data = VirtualAlloc(0, FileSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
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
					VirtualFree(fd.Data, 0, MEM_RELEASE);
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

directory_list GetDirectoryList (char *WildCard)
{
	directory_list DirList = {0};

	// char *WildCard = "posts/*.html";
	WIN32_FIND_DATAA FindData;
	HANDLE FileHandle = FindFirstFileA(WildCard, &FindData);
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		while (TRUE)
		{
			strcpy(DirList.Files[DirList.FileCount].Name, FindData.cFileName);
			DirList.Files[DirList.FileCount].WriteTime = FindData.ftLastWriteTime;
			++DirList.FileCount;

			if (!FindNextFileA(FileHandle, &FindData))
			{
				break;
			}
		}
	}

	return DirList;
}

directory_list ConcatDirectoryList (directory_list dl0, directory_list dl1)
{
	directory_list DirList = {0};
	forc (dl0.FileCount)
	{
		strcpy(DirList.Files[DirList.FileCount].Name, dl0.Files[i].Name);
		DirList.Files[DirList.FileCount].WriteTime = dl0.Files[i].WriteTime;
		++DirList.FileCount;
	}
	forc (dl1.FileCount)
	{
		strcpy(DirList.Files[DirList.FileCount].Name, dl1.Files[i].Name);
		DirList.Files[DirList.FileCount].WriteTime = dl1.Files[i].WriteTime;
		++DirList.FileCount;
	}

	return DirList;
}