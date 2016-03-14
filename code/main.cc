
/*
	TODO:
	Blog post sorting
	Convert dates to nice print format
*/

/*
	Writting format idea

	This is the first paragraph.

	This another paragraph.

	image.png Caption for image
*/

#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <sys/stat.h>
#include <direct.h>

#include "shared.cc"

struct menu {
	struct {
		char name[64];
		char dest[64];
	} items[64];
	s32 count;
};

struct date {
	s32 Day;
	s32 Month;
	s32 Year;
};

struct page {
	char *FileName;
	char *Data;
	b32 Post;
	char *Title;
	char *Desc;
	char *DateString;
	date Date;
	u32 DateSortKey;
	char *Url;
	char *Image;
};

struct page_list {
	page pages[256];
	u32 count;
};

#include "output.cc"

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

/*s32 StringChars (char *String, char Char)
{
	s32 NumChars = 0;
	fori (strlen(String), CharIndex)
	{
		if (String[CharIndex] == Char)
		{
			++NumChars;
		}
	}

	return NumChars;
}*/

char *Months[] =
{
	"Zero",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

char *GetPrintDate (page post)
{
	char *Result = PushMemory(16);
	sprintf(Result, "%i %s %i", post.Date.Day, Months[post.Date.Month], post.Date.Year);
	return Result;
}

void BubbleSortFilesLatestTop (page_list *pages)
{
	fori (pages->count, i0)
	{
		fori (pages->count-1, i1)
		{
			page f0 = pages->pages[i1];
			page f1 = pages->pages[i1+1];
			if (f0.DateString && f1.DateString)
			{
				if (f1.DateSortKey > f0.DateSortKey)
				{
					page Swap = pages->pages[i1];
					pages->pages[i1] = pages->pages[i1+1];
					pages->pages[i1+1] = Swap;
				}
			}
		}
	}
}

enum token_type {
	TOKEN_UNKNOWN,
	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,
	TOKEN_NEWLINE,
	TOKEN_COLON,
	TOKEN_SLASH,
	TOKEN_END_OF_STREAM,
};

struct token {
	token_type type;
	char str[64]; // TODO: Might need increasing
};

struct tokenizer {
	char *str;
	char *strp;
};

tokenizer InitTokenizer (char *str)
{
	tokenizer t = {};
	t.str = str;
	t.strp = str;
	return t;
}

b32 CharIsAlpha (char c)
{
	if (c >= 'a' && c <= 'z' ||
		c >= 'A' && c <= 'Z') {
		return true;
	} else {
		return false;
	}
}

b32 CharIsAlphaNumeric (char c)
{
	if (c >= 'a' && c <= 'z' ||
		c >= 'A' && c <= 'Z' ||
		c >= '0' && c <= '9') {
		return true;
	} else {
		return false;
	}
}

b32 CharIsNumeric (char c)
{
	if (c >= '0' && c <= '9') {
		return true;
	} else {
		return false;
	}
}

token GetToken (tokenizer *tizer)
{
	token t = {};
	s32 charCount = 0;

	while (*tizer->strp == ' ' || *tizer->strp == '\t') {
		++tizer->strp;
	}

	if (*tizer->strp == 0) {
		t.type = TOKEN_END_OF_STREAM;
		return t;
	}

	if (*tizer->strp == '\n' || *tizer->strp == '\r') {
		t.type = TOKEN_NEWLINE;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (CharIsAlpha(*tizer->strp)) {
		t.type = TOKEN_IDENTIFIER;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
		while (CharIsAlphaNumeric(*tizer->strp)) {
			t.str[charCount] = *tizer->strp;
			++charCount;
			++tizer->strp;
		}
	} else if (CharIsNumeric(*tizer->strp)) {
		t.type = TOKEN_NUMBER;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
		while (CharIsNumeric(*tizer->strp)) {
			t.str[charCount] = *tizer->strp;
			++charCount;
			++tizer->strp;
		}
	} else if (*tizer->strp == ':') {
		t.type = TOKEN_COLON;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (*tizer->strp == '/') {
		t.type = TOKEN_SLASH;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else {
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	}

	return t;
}

token ReadUntilNewLine (tokenizer *tizer)
{
	token t = {};
	s32 charCount = 0;

	while (*tizer->strp == ' ' || *tizer->strp == '\t') {
		++tizer->strp;
	}

	while (*tizer->strp != '\n' && *tizer->strp != '\r' && *tizer->strp != 0) {
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	}

	++tizer->strp;

	return t;
}

token ReadUntilColon (tokenizer *tizer)
{
	token t = {};
	s32 charCount = 0;

	while (*tizer->strp == ' ' || *tizer->strp == '\t') {
		++tizer->strp;
	}

	while (*tizer->strp != ':' && *tizer->strp != 0) {
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	}

	++tizer->strp;

	return t;
}

b32 FirstPartOfStrEquals (char *str, char *checkStr)
{
	s32 len = strlen(checkStr);
	b32 result = true;
	if (strlen(str) >= len) {
		fiz (len) {
			if (str[i] != checkStr[i]) {
				result = false;
				break;
			}
		}
	} else {
		result = false;
	}

	return result;
}

void Compile ()
{
	printf("Compiling... \n");

	InitMemory();

	page_list pageList = {0};

	{
		char *WildCard = "*.html";
		WIN32_FIND_DATAA FindData;
		HANDLE FileHandle = FindFirstFileA(WildCard, &FindData);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				page *currentPage = &pageList.pages[pageList.count];

				char *Mem = PushMemory(strlen(FindData.cFileName) + 1);
				strcpy(Mem, FindData.cFileName);
				*(Mem + strlen(FindData.cFileName)) = 0;
				currentPage->FileName = Mem;

				char *FileData = ReadFileDataOrError(Mem);
				currentPage->Data = FileData;

				++pageList.count;

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

				page *currentPage = &pageList.pages[pageList.count];
				currentPage->FileName = Mem;
				currentPage->Post = TRUE;

				char *FileData = ReadFileDataOrError(Mem);

				tokenizer tizer = InitTokenizer(FileData);
				token t = GetToken(&tizer);
				while (t.type == TOKEN_IDENTIFIER) {
					if (strcmp(t.str, "title") == 0) {
						t = GetToken(&tizer);
						if (t.type == TOKEN_COLON) {

							token titleText = ReadUntilNewLine(&tizer);

							char *title = PushMemory(strlen(titleText.str) + 1);
							strcpy(title, titleText.str);
							currentPage->Title = title;
						}
					} else if (strcmp(t.str, "desc") == 0) {
						t = GetToken(&tizer);
						if (t.type == TOKEN_COLON) {

							token descText = ReadUntilNewLine(&tizer);

							char *desc = PushMemory(strlen(descText.str) + 1);
							strcpy(desc, descText.str);
							currentPage->Desc = desc;
						}
					} else if (strcmp(t.str, "date") == 0) {
						t = GetToken(&tizer);
						if (t.type == TOKEN_COLON) {
							t = GetToken(&tizer);
							if (t.type == TOKEN_NUMBER) {
								currentPage->Date.Day = strtol(t.str, NULL, 0);
								t = GetToken(&tizer);
								if (t.type == TOKEN_SLASH) {
									t = GetToken(&tizer);
									if (t.type == TOKEN_NUMBER) {
										currentPage->Date.Month = strtol(t.str, NULL, 0);
										t = GetToken(&tizer);
										if (t.type == TOKEN_SLASH) {
											t = GetToken(&tizer);
											if (t.type == TOKEN_NUMBER) {
												currentPage->Date.Year = strtol(t.str, NULL, 0);
											}
										}
									}
								}
							}

							currentPage->DateString = "Yes there is a date";
							currentPage->DateSortKey = ((u16)currentPage->Date.Year << 16) | ((u8)currentPage->Date.Month << 8) | ((u8)currentPage->Date.Day);
						}
					} else {
						break;
					}

					t = GetToken(&tizer);
				}

				currentPage->Data = FileData;

				++pageList.count;

				if (!FindNextFileA(FileHandle, &FindData))
				{
					break;
				}
			}
			while (FileHandle != INVALID_HANDLE_VALUE);
		}
	}

	fiz (pageList.count)
	{
		page *currentPage = &pageList.pages[i];

		// TODO: Here we could use FindFile and post_name.* to get all images and choose one which might be eaiser
		// Gen image paths
		currentPage->Url = PushMemory(strlen(currentPage->FileName) + 2);
		sprintf(currentPage->Url, "/%s\0", currentPage->FileName);

		b32 jpg = false;
		{
			char *tempImagePath = PushMemory(strlen("output/assets/") + (strlen(currentPage->FileName)-1) + 1);
			sprintf(tempImagePath, "output/assets/%s", currentPage->FileName);
			char *ext = tempImagePath + strlen(tempImagePath) - 4;
			ext[0] = 'p';
			ext[1] = 'n';
			ext[2] = 'g';
			ext[3] = 0;

			DWORD fileAttributes = GetFileAttributes(tempImagePath);
			if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
				ext[0] = 'j';
				ext[1] = 'p';
				ext[2] = 'g';

				fileAttributes = GetFileAttributes(tempImagePath);
				if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
					jpg = true;
				} else {
					printf("Cannot find image file for %s \n", currentPage->FileName);
				}
			}
		}

		currentPage->Image = PushMemory(strlen("/assets/") + (strlen(currentPage->FileName)-1) + 1);
		sprintf(currentPage->Image, "/assets/%s", currentPage->FileName);
		char *ext = currentPage->Image + strlen(currentPage->Image) - 4;
		if (jpg) {
			ext[0] = 'j';
			ext[1] = 'p';
			ext[2] = 'g';
		} else {
			ext[0] = 'p';
			ext[1] = 'n';
			ext[2] = 'g';
		}
		ext[3] = 0;



#if 0
		// Gen dates
		file *f = &FileList.Files[i];
		if (f->DateString)
		{
			if (StringChars(f->DateString, '/') == 2)
			{
				// strchr(f->DateString, '/')
				char *NewP;
				s32 Day = strtol(f->DateString, &NewP, 0);
				++NewP;
				s32 Month = strtol(NewP, &NewP, 0);
				++NewP;
				s32 Year = strtol(NewP, &NewP, 0);

				f->Date.Day = Day;
				f->Date.Month = Month;
				f->Date.Year = Year;

				f->DateSortKey = ((u16)Year << 16) | ((u8)Month << 8) | ((u8)Day);
				// printf("date %2i %2i %4i, sortkey 0x%8x %i \n", f->Date.Day, f->Date.Month, f->Date.Year, f->DateSortKey, f->DateSortKey);

				int x = 0;
			}
		}
#endif
	}

	BubbleSortFilesLatestTop(&pageList);

	fiz (pageList.count)
	{
		page *p = &pageList.pages[i];
		if (p->DateString)
		{
			// printf("date %2i %2i %4i, sortkey 0x%8x %i \n", f->Date.Day, f->Date.Month, f->Date.Year, f->DateSortKey, f->DateSortKey);
		}
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

	fiz (pageList.count)
	{
		page *currentPage = &pageList.pages[i];

		if (strcmp(currentPage->FileName, "template.html") != 0)
		{
			char *FileData = currentPage->Data;

			char *OutputFileName = PushMemory(strlen(currentPage->FileName) + strlen("output/") + 1);
			sprintf(OutputFileName, "output/%s\0", currentPage->FileName);
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

				if (FirstPartOfStrEquals(&TemplateFileData[TemplateFileIndex], "{content}"))
				{
					u32 FileDataSize = strlen(FileData);
					fori(FileDataSize, FileIndex)
					{
						if (FirstPartOfStrEquals(&FileData[FileIndex], "{blog}"))
						{
							FileIndex += 6;
							b32 ParsingBlogLoop = TRUE;
							s32 ParseIndex = 0;
							while (ParsingBlogLoop)
							{
								if (FirstPartOfStrEquals(&FileData[FileIndex+ParseIndex], "{endblog}"))
								{
									FileData[FileIndex + ParseIndex] = 0;
									char *BlogLoopData = PushMemory(ParseIndex + 1);
									strcpy(BlogLoopData, FileData + FileIndex);
									*(BlogLoopData + ParseIndex) = 0;
									FileIndex += ParseIndex + 9;
									ParsingBlogLoop = FALSE;
									// printf("Blog loop: %s \n", BlogLoopData);

									// fjz (pageList.count, BlogFilesIndex)
									fjz (pageList.count)
									{
										page *currentPost = &pageList.pages[j];

										if (currentPost->Post)
										{
											fori(strlen(BlogLoopData), BlogLoopIndex)
											{
												// Title
												if (FirstPartOfStrEquals(&BlogLoopData[BlogLoopIndex], "{title}"))
												{
													if (currentPost->Title)
													{
														fputs(currentPost->Title, OutputFileHandle);
													}
													BlogLoopIndex += 7;
												}
												// Desc
												else if (FirstPartOfStrEquals(&BlogLoopData[BlogLoopIndex], "{desc}"))
												{
													if (currentPost->Desc)
													{
														fputs(currentPost->Desc, OutputFileHandle);
													}
													BlogLoopIndex += 6;
												}
												// Date
												else if (FirstPartOfStrEquals(&BlogLoopData[BlogLoopIndex], "{date}"))
												{
													if (currentPost->DateString)
													{
														// fputs(FileList.Files[BlogFilesIndex].DateString, OutputFileHandle);
														fputs(GetPrintDate(*currentPost), OutputFileHandle);
													}
													BlogLoopIndex += 6;
												}
												// Url
												else if (FirstPartOfStrEquals(&BlogLoopData[BlogLoopIndex], "{url}"))
												{
													if (currentPost->Url)
													{
														fputs(currentPost->Url, OutputFileHandle);
													}
													BlogLoopIndex += 5;
												}
												// Image
												else if (FirstPartOfStrEquals(&BlogLoopData[BlogLoopIndex], "{image}"))
												{
													if (currentPost->Image)
													{
														fputs(currentPost->Image, OutputFileHandle);
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

void TestLoadMenuConfig ()
{
	file_data MenuConfig = Win32ReadFile("menu.cfg");
	// printf("Menu config: %s \n", MenuConfig.Data);

	menu m = {};
	tokenizer tizer = InitTokenizer(MenuConfig.Data);

	token t = ReadUntilColon(&tizer);
	while (strlen(t.str)) {
		strcpy(m.items[m.count].name, t.str);
		t = ReadUntilNewLine(&tizer);
		strcpy(m.items[m.count].dest, t.str);
		++m.count;

		t = ReadUntilColon(&tizer);
	}

	int x = 0;

	// printf("%s \n", t.str);

	// printf("%s \n", t.str);

	/*token t = GetToken(&tizer);
	while (t.type != TOKEN_END_OF_STREAM) {
		printf("Config token: %s \n", t.str);
		t = GetToken(&tizer);
	}*/
}

int main ()
{
	TestLoadMenuConfig();
	Compile();

	file_list fl0 = GetFileList("*.html");
	file_list fl1 = GetFileList("posts/*.html");
	file_list masterFileList = ConcatFileList(fl0, fl1);

	while (TRUE)
	{
		file_list fl0 = GetFileList("*.html");
		file_list fl1 = GetFileList("posts/*.html");
		file_list fileList = ConcatFileList(fl0, fl1);

		if (fileList.count != masterFileList.count)
		{
			Compile();
			masterFileList = fileList;
		}
		else
		{
			forc (fileList.count)
			{
				if (CompareFileTime(&fileList.files[i].writeTime, &masterFileList.files[i].writeTime))
				{
					Compile();
					masterFileList = fileList;
					break;
				}
			}
		}

		Sleep(1000);
	}

	system("pause");
	return 0;
}