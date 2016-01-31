
/*
	TODO:
	Blog post sorting
	Convert dates to nice print format
*/

#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <sys/stat.h>

#include "shared.c"

typedef struct
{
	s32 Day;
	s32 Month;
	s32 Year;
} date;

typedef struct
{
	char *FileName;
	char *Data;
	b32 Post;
	char *Title;
	char *Desc;
	char *DateString;
	date Date;
	char *Url;
	char *Image;
} file;

typedef struct
{
	file Files[256];
	u32 FileCount;
} file_list;

typedef struct
{
	char Name[64];
	u64 ModifiedTime;
} file_state;

file_state FileStates[64];

void AddFileStat (char *FileName)
{
	struct _stat64 Stat;
	_stat64(FileName, &Stat);
}

char *ReadFileDataOrError (char *FileName)
{
	FILE *FileHandle = fopen(FileName, "r");
	if (!FileHandle)
	{
		Error("Unable to open file");
		Error(FileName);
		printf(strerror(errno));
	}

	fseek(FileHandle, 0, SEEK_END);
	u32 FileSize = ftell(FileHandle);
	fseek(FileHandle, 0, SEEK_SET);

	char *FileData = PushMemory(FileSize + 1);
	fread(FileData, sizeof(char), FileSize, FileHandle);
	fclose(FileHandle);

	return FileData;
}

/*
	Stuff blog post loops need
	- url
	- title
	- date
	- image
	- description
*/

void OutputChar (char Char, FILE *File)
{
	Assert(Char);
#if 1
	fputc(Char, File);
#else
	printf("%c", Char);
#endif
}

void Compile ()
{
	printf("Compiling... \n");

	InitMemory();

	file_list FileList = {0};

	{
		char *WildCard = "*.html";
		WIN32_FIND_DATAA FindData;
		HANDLE FileHandle = FindFirstFileA(WildCard, &FindData);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				char *Mem = PushMemory(strlen(FindData.cFileName) + 1);
				strcpy(Mem, FindData.cFileName);
				*(Mem + strlen(FindData.cFileName)) = 0;
				FileList.Files[FileList.FileCount].FileName = Mem;

				char *FileData = ReadFileDataOrError(Mem);
				FileList.Files[FileList.FileCount].Data = FileData;

				++FileList.FileCount;

				if (!FindNextFileA(FileHandle, &FindData))
				{
					break;
				}
			}
			while (FileHandle != INVALID_HANDLE_VALUE);
		}
	}

	{
		char *WildCard = "posts/*.html";
		WIN32_FIND_DATAA FindData;
		HANDLE FileHandle = FindFirstFileA(WildCard, &FindData);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				char *Mem = PushMemory(strlen("posts/") + strlen(FindData.cFileName) + 1);
				strcpy(Mem, "posts/");
				strcpy(Mem + strlen("posts/"), FindData.cFileName);
				*(Mem + strlen(FindData.cFileName) + strlen("posts/")) = 0;
				FileList.Files[FileList.FileCount].FileName = Mem;
				FileList.Files[FileList.FileCount].Post = TRUE;

				char *FileData = ReadFileDataOrError(Mem);
				// The stupidest possible thing
				b32 ParsingMetaData = TRUE;
				while (ParsingMetaData)
				{
					if (FileData[0] == 't' &&
						FileData[1] == 'i' &&
						FileData[2] == 't' &&
						FileData[3] == 'l' &&
						FileData[4] == 'e' &&
						FileData[5] == ':')
					{
						FileData += 6;
						s32 ValueLen = 0;
						if (FileData[0] == ' ')
						{
							++FileData;
						}
						while (FileData[ValueLen] != '\n')
						{
							++ValueLen;
						}
						FileData[ValueLen] = 0;
						char *P = PushMemory(ValueLen + 1);
						strcpy(P, FileData);
						FileData += ValueLen + 1;

						FileList.Files[FileList.FileCount].Title = P;
					}
					else if (FileData[0] == 'd' &&
							 FileData[1] == 'e' &&
							 FileData[2] == 's' &&
							 FileData[3] == 'c' &&
							 FileData[4] == ':')
					{
						FileData += 5;
						s32 ValueLen = 0;
						if (FileData[0] == ' ')
						{
							++FileData;
						}
						while (FileData[ValueLen] != '\n')
						{
							++ValueLen;
						}
						FileData[ValueLen] = 0;
						char *P = PushMemory(ValueLen + 1);
						strcpy(P, FileData);
						FileData += ValueLen + 1;

						FileList.Files[FileList.FileCount].Desc = P;
					}
					else if (FileData[0] == 'd' &&
							 FileData[1] == 'a' &&
							 FileData[2] == 't' &&
							 FileData[3] == 'e' &&
							 FileData[4] == ':')
					{
						FileData += 5;
						s32 ValueLen = 0;
						if (FileData[0] == ' ')
						{
							++FileData;
						}
						while (FileData[ValueLen] != '\n')
						{
							++ValueLen;
						}
						FileData[ValueLen] = 0;
						char *P = PushMemory(ValueLen + 1);
						strcpy(P, FileData);
						FileData += ValueLen + 1;

						FileList.Files[FileList.FileCount].DateString = P;
					}
					else
					{
						ParsingMetaData = FALSE;
					}
				}

				FileList.Files[FileList.FileCount].Data = FileData;

				++FileList.FileCount;

				if (!FindNextFileA(FileHandle, &FindData))
				{
					break;
				}
			}
			while (FileHandle != INVALID_HANDLE_VALUE);
		}
	}

	forc(FileList.FileCount)
	{
		// printf("url: %s \n", FileList.Files[i].FileName);
		FileList.Files[i].Url = PushMemory(strlen(FileList.Files[i].FileName) + 2);
		sprintf(FileList.Files[i].Url, "/%s\0", FileList.Files[i].FileName);
		FileList.Files[i].Image = PushMemory(strlen("/assets/") + (strlen(FileList.Files[i].FileName)-1) + 1);
		sprintf(FileList.Files[i].Image, "/assets/%s", FileList.Files[i].FileName);
		char *P = FileList.Files[i].Image + strlen(FileList.Files[i].Image) - 4;
		P[0] = 'p';
		P[1] = 'n';
		P[2] = 'g';
		P[3] = 0;
	}

	// FILE *TemplateFileHandle = fopen("template.html", "r");
	// if (!TemplateFileHandle)
	// {
	// 	Error("Unable to open template file");
	// 	printf(strerror(errno));
	// }

	// fseek(TemplateFileHandle, 0, SEEK_END);
	// u32 TemplateFileSize = ftell(TemplateFileHandle);
	// fseek(TemplateFileHandle, 0, SEEK_SET);

	// char *TemplateFileData = PushMemory(TemplateFileSize + 1);
	// fread(TemplateFileData, sizeof(char), TemplateFileSize, TemplateFileHandle);
	// fclose(TemplateFileHandle);

	char *TemplateFileData = ReadFileDataOrError("template.html");

	// printf("Tempalte file: %s \n", TemplateFileData);

	mkdir("output");
	mkdir("output/posts");

	forc(FileList.FileCount)
	{
		if (strcmp(FileList.Files[i].FileName, "template.html") != 0)
		{
			char *FileData = FileList.Files[i].Data;

			char *OutputFileName = PushMemory(strlen(FileList.Files[i].FileName) + strlen("output/") + 1);
			sprintf(OutputFileName, "output/%s\0", FileList.Files[i].FileName);
			// printf("Output file: %s \n", OutputFileName);


			FILE *OutputFileHandle = fopen(OutputFileName, "w");
			if (!OutputFileHandle)
			{
				// Error("Unable to open template file");
				printf("FileError: %s \n", strerror(errno));
			}
			u32 TemplateFileSize = strlen(TemplateFileData);
			fori(TemplateFileSize, TemplateFileIndex)
			{
				Assert(OutputFileHandle);
				Assert(TemplateFileData[TemplateFileIndex]);

				if (TemplateFileIndex + 8 <= TemplateFileSize &&
					TemplateFileData[TemplateFileIndex+0] == '{' &&
					TemplateFileData[TemplateFileIndex+1] == 'c' &&
					TemplateFileData[TemplateFileIndex+2] == 'o' &&
					TemplateFileData[TemplateFileIndex+3] == 'n' &&
					TemplateFileData[TemplateFileIndex+4] == 't' &&
					TemplateFileData[TemplateFileIndex+5] == 'e' &&
					TemplateFileData[TemplateFileIndex+6] == 'n' &&
					TemplateFileData[TemplateFileIndex+7] == 't' &&
					TemplateFileData[TemplateFileIndex+8] == '}')
				{
					u32 FileDataSize = strlen(FileData);
					fori(FileDataSize, FileIndex)
					{
						if (FileData[FileIndex + 0] == '{' &&
							FileData[FileIndex + 1] == 'b' &&
							FileData[FileIndex + 2] == 'l' &&
							FileData[FileIndex + 3] == 'o' &&
							FileData[FileIndex + 4] == 'g' &&
							FileData[FileIndex + 5] == '}')
						{
							FileIndex += 6;
							b32 ParsingBlogLoop = TRUE;
							s32 ParseIndex = 0;
							while (ParsingBlogLoop)
							{
								if (FileData[FileIndex + ParseIndex + 0] == '{' &&
									FileData[FileIndex + ParseIndex + 1] == 'e' &&
									FileData[FileIndex + ParseIndex + 2] == 'n' &&
									FileData[FileIndex + ParseIndex + 3] == 'd' &&
									FileData[FileIndex + ParseIndex + 4] == 'b' &&
									FileData[FileIndex + ParseIndex + 5] == 'l' &&
									FileData[FileIndex + ParseIndex + 6] == 'o' &&
									FileData[FileIndex + ParseIndex + 7] == 'g' &&
									FileData[FileIndex + ParseIndex + 8] == '}')
								{
									FileData[FileIndex + ParseIndex] = 0;
									char *BlogLoopData = PushMemory(ParseIndex);
									strcpy(BlogLoopData, FileData + FileIndex);
									FileIndex += ParseIndex + 9;
									ParsingBlogLoop = FALSE;
									// printf("Blog loop: %s \n", BlogLoopData);

									fori(FileList.FileCount, BlogFilesIndex)
									{
										if (FileList.Files[BlogFilesIndex].Post)
										{
											fori(strlen(BlogLoopData), BlogLoopIndex)
											{
												// Title
												if (BlogLoopData[BlogLoopIndex + 0] == '{' &&
													BlogLoopData[BlogLoopIndex + 1] == 't' &&
													BlogLoopData[BlogLoopIndex + 2] == 'i' &&
													BlogLoopData[BlogLoopIndex + 3] == 't' &&
													BlogLoopData[BlogLoopIndex + 4] == 'l' &&
													BlogLoopData[BlogLoopIndex + 5] == 'e' &&
													BlogLoopData[BlogLoopIndex + 6] == '}')
												{
													if (FileList.Files[BlogFilesIndex].Title)
													{
														fputs(FileList.Files[BlogFilesIndex].Title, OutputFileHandle);
													}
													BlogLoopIndex += 7;
												}
												// Desc
												else if (BlogLoopData[BlogLoopIndex + 0] == '{' &&
													BlogLoopData[BlogLoopIndex + 1] == 'd' &&
													BlogLoopData[BlogLoopIndex + 2] == 'e' &&
													BlogLoopData[BlogLoopIndex + 3] == 's' &&
													BlogLoopData[BlogLoopIndex + 4] == 'c' &&
													BlogLoopData[BlogLoopIndex + 5] == '}')
												{
													if (FileList.Files[BlogFilesIndex].Desc)
													{
														fputs(FileList.Files[BlogFilesIndex].Desc, OutputFileHandle);
													}
													BlogLoopIndex += 6;
												}
												// Date
												else if (BlogLoopData[BlogLoopIndex + 0] == '{' &&
													BlogLoopData[BlogLoopIndex + 1] == 'd' &&
													BlogLoopData[BlogLoopIndex + 2] == 'a' &&
													BlogLoopData[BlogLoopIndex + 3] == 't' &&
													BlogLoopData[BlogLoopIndex + 4] == 'e' &&
													BlogLoopData[BlogLoopIndex + 5] == '}')
												{
													if (FileList.Files[BlogFilesIndex].DateString)
													{
														fputs(FileList.Files[BlogFilesIndex].DateString, OutputFileHandle);
													}
													BlogLoopIndex += 6;
												}
												// Url
												else if (BlogLoopData[BlogLoopIndex + 0] == '{' &&
													BlogLoopData[BlogLoopIndex + 1] == 'u' &&
													BlogLoopData[BlogLoopIndex + 2] == 'r' &&
													BlogLoopData[BlogLoopIndex + 3] == 'l' &&
													BlogLoopData[BlogLoopIndex + 4] == '}')
												{
													if (FileList.Files[BlogFilesIndex].Url)
													{
														fputs(FileList.Files[BlogFilesIndex].Url, OutputFileHandle);
													}
													BlogLoopIndex += 5;
												}
												// Image
												else if (BlogLoopData[BlogLoopIndex + 0] == '{' &&
													BlogLoopData[BlogLoopIndex + 1] == 'i' &&
													BlogLoopData[BlogLoopIndex + 2] == 'm' &&
													BlogLoopData[BlogLoopIndex + 3] == 'a' &&
													BlogLoopData[BlogLoopIndex + 4] == 'g' &&
													BlogLoopData[BlogLoopIndex + 5] == 'e' &&
													BlogLoopData[BlogLoopIndex + 6] == '}')
												{
													if (FileList.Files[BlogFilesIndex].Image)
													{
														fputs(FileList.Files[BlogFilesIndex].Image, OutputFileHandle);
													}
													BlogLoopIndex += 7;
												}
												else
												{

												}

												OutputChar(BlogLoopData[BlogLoopIndex], OutputFileHandle);
											}
										}
									}
								}

								++ParseIndex;
							}
						}

						if (FileData[FileIndex])
						{
							OutputChar(FileData[FileIndex], OutputFileHandle);
						}
					}

					TemplateFileIndex += 9;
				}

				OutputChar(TemplateFileData[TemplateFileIndex], OutputFileHandle);
			}
			fclose(OutputFileHandle);
		}
	}

	printf("Memory used: %i/%i \n", MemoryUsed, MemorySize);
}

int main ()
{
	Compile();

	directory_list dl0 = GetDirectoryList("*.html");
	directory_list dl1 = GetDirectoryList("posts/*.html");
	directory_list MasterDirList = ConcatDirectoryList(dl0, dl1);

	while (TRUE)
	{
		// {
		// 	char *WildCard = "*.html";
		// 	WIN32_FIND_DATAA FindData;
		// 	HANDLE FileHandle = FindFirstFileA(WildCard, &FindData);
		// 	if (FileHandle != INVALID_HANDLE_VALUE)
		// 	{
		// 		while (TRUE)
		// 		{
		// 			printf("%s \n", FindData.cFileName);
		// 			AddFileStat(FindData.cFileName);

		// 			if (!FindNextFileA(FileHandle, &FindData))
		// 			{
		// 				break;
		// 			}
		// 		}
		// 	}
		// }
		// {
		// 	char *WildCard = "posts/*.html";
		// 	WIN32_FIND_DATAA FindData;
		// 	HANDLE FileHandle = FindFirstFileA(WildCard, &FindData);
		// 	if (FileHandle != INVALID_HANDLE_VALUE)
		// 	{
		// 		while (TRUE)
		// 		{
		// 			printf("%s \n", FindData.cFileName);
		// 			AddFileStat(FindData.cFileName);

		// 			if (!FindNextFileA(FileHandle, &FindData))
		// 			{
		// 				break;
		// 			}
		// 		}
		// 	}
		// }

		directory_list dl0 = GetDirectoryList("*.html");
		directory_list dl1 = GetDirectoryList("posts/*.html");
		directory_list DirList = ConcatDirectoryList(dl0, dl1);
		// forc (DirList.FileCount)
		// {
		// 	printf("File: %s \n", DirList.Files[i].Name);
		// }

		if (DirList.FileCount != MasterDirList.FileCount)
		{
			Compile();
			MasterDirList = DirList;
		}
		else
		{
			forc (DirList.FileCount)
			{
				if (CompareFileTime(&DirList.Files[i].WriteTime, &MasterDirList.Files[i].WriteTime))
				{
					Compile();
					MasterDirList = DirList;
					break;
				}
			}
		}

		Sleep(1000);
	}

	system("pause");
	return 0;
}