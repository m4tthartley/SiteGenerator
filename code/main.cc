
/*
	TODO:
	  Generate different sized images and try that srcset thing
	  How should footers work?
*/

/*
	Writting format idea

	This is the first paragraph.

	This another paragraph.

	image.png Caption for image
*/

#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef _WIN32
#	include <windows.h>
#	include <direct.h>
#endif

#ifdef __APPLE__
#	include <errno.h>

void mkdir (char *name) {
	mkdir(name, 0755);
}
#endif

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

enum page_content_type {
	CONTENT_HEADER,
	CONTENT_SECONDARY_HEADER,
	CONTENT_PARAGRAPH,
	CONTENT_IMAGE,
	CONTENT_AUTHOR,
	CONTENT_BLOG_LIST,
	CONTENT_VIDEO,
	CONTENT_YOUTUBE,
};

enum paragraph_word_type {
	PARAGRAPH_WORD_WORD,
	PARAGRAPH_WORD_LINK,
	PARAGRAPH_LINE_BREAK,
};

struct page_content {
	page_content_type type;
	union {
		struct {
			char str[256];
		} header;
		struct {
			// char str[1024];
			struct {
				paragraph_word_type type;
				union {
					char str[32];
					struct {
						char url[128];
						char str[64];
					} link;
				};
				b32 noSpace;
			} words[256];
			s32 wordCount;
		} paragraph;
		struct {
			char fileName[64];
			char caption[256];
		} image;
		struct {
			char fileName[64];
			char caption[256];
		} video;
		struct {
			char str[256];
		} author;
	};

	page_content *next;
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

	page_content *content;
	// s32 contentCount;
};

struct page_list {
	page pages[256];
	u32 count;
};

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

char *GetPrintDate (page *post)
{
	char *Result = PushMemory(16);
	sprintf(Result, "%i %s %i", post->Date.Day, Months[post->Date.Month], post->Date.Year);
	return Result;
}

#include "output.cc"

#if 0
void AddFileStat (char *FileName)
{
	struct _stat64 Stat;
	_stat64(FileName, &Stat);
}
#endif

char *ReadFileDataOrError (char *FileName)
{
	char *FileData = NULL;
	FILE *FileHandle = fopen(FileName, "r");
	if (FileHandle)
	{
		fseek(FileHandle, 0, SEEK_END);
		u32 FileSize = ftell(FileHandle);
		fseek(FileHandle, 0, SEEK_SET);

		FileData = PushMemory(FileSize + 1);
		fread(FileData, sizeof(char), FileSize, FileHandle);
		fclose(FileHandle);
	} else {
		Error("Unable to open file");
		Error(FileName);
		printf(strerror(errno));
	}

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
	TOKEN_OPEN_SQUARE_BRACKET,
	TOKEN_CLOSE_SQUARE_BRACKET,
	TOKEN_OPEN_BRACKET,
	TOKEN_CLOSE_BRACKET,
	TOKEN_COMMENT,
	TOKEN_WORD,
	TOKEN_FILE,
	TOKEN_BACKSLASH,
};

struct token {
	token_type type;
	char str[1024]; // TODO: Used to be 64 // TODO: Might need increasing
};

struct tokenizer {
	char *str;
	char *strp;
	char *currentLine;
};

tokenizer InitTokenizer (char *str)
{
	tokenizer t = {};
	t.str = str;
	t.strp = str;
	t.currentLine = str;
	return t;
}

void RestartLine (tokenizer *tizer)
{
	tizer->strp = tizer->currentLine;
}

b32 CharIsAlpha (char c)
{
	if ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z')) {
		return true;
	} else {
		return false;
	}
}

b32 CharIsAlphaNumeric (char c)
{
	if ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9')) {
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

b32 CharIsPath (char c) {
	if (CharIsAlphaNumeric(c) ||
		c == '_' ||
		c == '-' ||
		c == '.' ||
		c == '/' ||
		c == ':') {
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
	} else if (*tizer->strp == '[') {
		t.type = TOKEN_OPEN_SQUARE_BRACKET;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (*tizer->strp == ']') {
		t.type = TOKEN_CLOSE_SQUARE_BRACKET;
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

token GetContentToken (tokenizer *tizer)
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
		tizer->currentLine = tizer->strp;
	} else if (*tizer->strp == '[') {
		t.type = TOKEN_OPEN_SQUARE_BRACKET;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (*tizer->strp == ']') {
		t.type = TOKEN_CLOSE_SQUARE_BRACKET;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (*tizer->strp == '(') {
		t.type = TOKEN_OPEN_BRACKET;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (*tizer->strp == ')') {
		t.type = TOKEN_CLOSE_BRACKET;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (*tizer->strp == '/' && *(tizer->strp+1) == '/') {
		t.type = TOKEN_COMMENT;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (*tizer->strp == '\\') {
		t.type = TOKEN_BACKSLASH;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else {
		t.type = TOKEN_WORD;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
		b32 dotFound = false;
		while (*tizer->strp != ' ' &&
			   *tizer->strp != '\t' &&
			   *tizer->strp != '\n' &&
			   *tizer->strp != '\r' &&
			   *tizer->strp != 0) {
			if (dotFound) {
				t.type = TOKEN_FILE;
			}
			if (*tizer->strp == '.') {
				dotFound = true;
			}
			t.str[charCount] = *tizer->strp;
			++charCount;
			if (*tizer->strp == '\n' || *tizer->strp == '\r') {
				tizer->currentLine = ++tizer->strp;
			} else {
				++tizer->strp;
			}
		}
	}

	/*else if (*tizer->strp == '.') {
		t.type = TOKEN_DOT;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	} else if (*tizer->strp == ',') {
		t.type = TOKEN_COMMA;
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	}*/

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

	// ++tizer->strp;
	tizer->currentLine = tizer->strp + 1;

	return t;
}

token ReadUntilColon (tokenizer *tizer, b32 *found)
{
	token t = {};
	s32 charCount = 0;

	while (*tizer->strp == ' ' || *tizer->strp == '\t') {
		++tizer->strp;
	}

	while (*tizer->strp != ':' && *tizer->strp != '\n' && *tizer->strp != '\r' && *tizer->strp != 0) {
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	}

	if (*tizer->strp == ':') {
		*found = true;
	} else {
		*found = false;
	}

	++tizer->strp;

	return t;
}

token ReadUntilCloseSquareBracket (tokenizer *tizer)
{
	token t = {};
	s32 charCount = 0;

	while (*tizer->strp == ' ' || *tizer->strp == '\t') {
		++tizer->strp;
	}

	while (*tizer->strp != ']' && *tizer->strp != 0) {
		t.str[charCount] = *tizer->strp;
		++charCount;
		++tizer->strp;
	}

	++tizer->strp;

	return t;
}

token ReadUntilCloseBracket (tokenizer *tizer)
{
	token t = {};
	s32 charCount = 0;

	while (*tizer->strp == ' ' || *tizer->strp == '\t') {
		++tizer->strp;
	}

	while (*tizer->strp != ')' && *tizer->strp != 0) {
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

menu LoadMenuConfig ()
{
	file_data MenuConfig = FRead("menu.cfg");
	// printf("Menu config: %s \n", MenuConfig.Data);

	menu m = {};
	tokenizer tizer = InitTokenizer(MenuConfig.Data);

	token t = GetContentToken(&tizer);
	while (t.type != TOKEN_END_OF_STREAM) {
		if (t.type == TOKEN_WORD) {
			RestartLine(&tizer);
			b32 found;
			t = ReadUntilColon(&tizer, &found);
			if (found) {
				strcpy(m.items[m.count].name, t.str);
				t = ReadUntilNewLine(&tizer);
				strcpy(m.items[m.count].dest, t.str);
			} else {
				printf("Error parsing menu config \n");
			}

			++m.count;
		}

		t = GetContentToken(&tizer);
	}

	int x = 0;

	// printf("%s \n", t.str);

	// printf("%s \n", t.str);

	/*token t = GetToken(&tizer);
	while (t.type != TOKEN_END_OF_STREAM) {
		printf("Config token: %s \n", t.str);
		t = GetToken(&tizer);
	}*/

	return m;
}

// TODO: Will this get slow with big pages? More importantly, will I ever have big pages?
void AddContent (page *p, page_content *content)
{
	if (!p->content) {
		p->content = content;
	} else {
		page_content *c = p->content;
		while (c->next) {
			c = c->next;
		}
		c->next = content;
	}
	/*content->next = p->content;
	p->content = content;*/
}

void ParsePageDate (page *p, char *str)
{
	tokenizer tizer = InitTokenizer(str);

	b32 success = false;

	token t = GetToken(&tizer);
	if (t.type == TOKEN_NUMBER) {
		p->Date.Day = strtol(t.str, NULL, 0);
		t = GetToken(&tizer);
		if (t.type == TOKEN_SLASH) {
			t = GetToken(&tizer);
			if (t.type == TOKEN_NUMBER) {
				p->Date.Month = strtol(t.str, NULL, 0);
				t = GetToken(&tizer);
				if (t.type == TOKEN_SLASH) {
					t = GetToken(&tizer);
					if (t.type == TOKEN_NUMBER) {
						p->Date.Year = strtol(t.str, NULL, 0);
						success = true;
					}
				}
			}
		}
	}

	if (!success) {
		printf("Error parsing date for file %s\n", p->FileName);
	}

	p->DateString = "Yes there is a date";
	p->DateSortKey = ((u16)p->Date.Year << 16) | ((u8)p->Date.Month << 8) | ((u8)p->Date.Day);
}

void GenImagePath (page *p)
{
	s32 extLen = 4;

	b32 jpg = false;
	{
		s32 tempImageLen = strlen("output/assets/posts/") + (strlen(p->FileName)) + 1 + extLen;
		char *tempImagePath = PushMemory(tempImageLen);
		sprintf(tempImagePath, "output/assets/posts/%s", p->FileName);

		s32 actualLen = strlen(tempImagePath);
		char *ext = tempImagePath + strlen(tempImagePath);
		ext[0] = '.';
		ext[1] = 'p';
		ext[2] = 'n';
		ext[3] = 'g';

#ifdef _WIN32
		DWORD fileAttributes = GetFileAttributes(tempImagePath);
		if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
			ext[0] = '.';
			ext[1] = 'j';
			ext[2] = 'p';
			ext[3] = 'g';

			fileAttributes = GetFileAttributes(tempImagePath);
			if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
				jpg = true;
			} else {
				printf("Cannot find image file for %s \n", p->FileName);
			}
		}
#endif
#ifdef __APPLE__
		FILE *fileAttributes = fopen(tempImagePath, "r");
		if (!fileAttributes) {
			ext[0] = '.';
			ext[1] = 'j';
			ext[2] = 'p';
			ext[3] = 'g';

			fileAttributes = fopen(tempImagePath, "r");
			if (fileAttributes) {
				jpg = true;
			} else {
				printf("Cannot find image file for %s \n", p->FileName);
			}
		}
#endif
	}

	p->Image = PushMemory(strlen("posts/") + (strlen(p->FileName)) + 1 + extLen);
	sprintf(p->Image, "posts/%s", p->FileName);
	char *ext = p->Image + strlen(p->Image);
	if (jpg) {
		ext[0] = '.';
		ext[1] = 'j';
		ext[2] = 'p';
		ext[3] = 'g';
	} else {
		ext[0] = '.';
		ext[1] = 'p';
		ext[2] = 'n';
		ext[3] = 'g';
	}
}

void ParsePage (page *p, char *file)
{
	// *p = {};

	s32 len = strlen(p->FileName);
	fjz (len) {
		if (p->FileName[len-1-j] == '.') {
			p->FileName[len-1-j] = 0;
			break;
		} else {
			p->FileName[len-1-j] = 0;
		}
	}

	if (p->Post) {
		p->Url = PushMemory(strlen(p->FileName) + strlen("/posts/") + strlen(".html") + 1);
		sprintf(p->Url, "/posts/%s.html", p->FileName);

		GenImagePath(p);
	} else {
		p->Url = PushMemory(strlen(p->FileName) + strlen("/") + strlen(".html") + 1);
		sprintf(p->Url, "/%s.html", p->FileName);
	}

	tokenizer tizer = InitTokenizer(file);
	token t = GetContentToken(&tizer);

	b32 firstHeaderUsed = false;
	// s32 contentCount = 0;

	while (t.type != TOKEN_END_OF_STREAM) {
		page_content *content = (page_content*)PushMemory(sizeof(page_content));
		if (t.type == TOKEN_OPEN_SQUARE_BRACKET) {
			t = ReadUntilCloseSquareBracket(&tizer);

			if (firstHeaderUsed) {
				content->type = CONTENT_SECONDARY_HEADER;
			} else {
				p->Title = PushMemory(strlen(t.str) + 1);
				strcpy(p->Title, t.str);
				content->type = CONTENT_HEADER;
				firstHeaderUsed = true;
			}

			strcpy(content->header.str, t.str);
			AddContent(p, content);
		} else if (t.type == TOKEN_WORD) {
			if (strcmp(t.str, "title:") == 0) {
				token value = ReadUntilNewLine(&tizer);
				char *s = PushMemory(strlen(value.str) + 1);
				strcpy(s, value.str);
				p->Title = s;
			} else if (strcmp(t.str, "desc:") == 0) {
				token value = ReadUntilNewLine(&tizer);
				char *s = PushMemory(strlen(value.str) + 1);
				strcpy(s, value.str);
				p->Desc = s;
			} else if (strcmp(t.str, "date:") == 0) {
				token value = ReadUntilNewLine(&tizer);
				ParsePageDate(p, value.str);
			} else if (strcmp(t.str, "author:") == 0) {
				token value = ReadUntilNewLine(&tizer);
				content->type = CONTENT_AUTHOR;
				strcpy(content->author.str, value.str);
				AddContent(p, content);
			} else if (strcmp(t.str, "!image") == 0) {
				content->type = CONTENT_IMAGE;
				strcpy(content->image.fileName, p->Image);
				AddContent(p, content);
			} else if (strcmp(t.str, "!blog") == 0) {
				content->type = CONTENT_BLOG_LIST;
				AddContent(p, content);
			} else if (strcmp(t.str, "!video") == 0) {
				content->type = CONTENT_VIDEO;
				t = GetContentToken(&tizer);
				if (t.type == TOKEN_FILE) {
					strcpy(content->video.fileName, t.str);
					t = ReadUntilNewLine(&tizer);
					strcpy(content->video.caption, t.str);
					AddContent(p, content);
				} else {
					printf("Error, missing video path");
				}
			} else if (strcmp(t.str, "!youtube") == 0) {
				content->type = CONTENT_YOUTUBE;
				t = GetContentToken(&tizer);
				if (t.type == TOKEN_FILE) {
					strcpy(content->video.fileName, t.str);
					t = ReadUntilNewLine(&tizer);
					strcpy(content->video.caption, t.str);
					AddContent(p, content);
				} else {
					printf("Error, missing youtube link");
				}
			} else if (strcmp(t.str, "!header") == 0) {
				t = ReadUntilNewLine(&tizer);
				content->type = CONTENT_HEADER;
				strcpy(content->header.str, t.str);
				AddContent(p, content);
			} else {
				/*TODO: New idea for parapraph parsing,
						ReadUntilOpenBracket and save section,
						parse link and save, ReadUntilOpenBracket and save section etc*/

				RestartLine(&tizer);
				content->type = CONTENT_PARAGRAPH;

				t = GetContentToken(&tizer);
				while (t.type != TOKEN_NEWLINE && t.type != TOKEN_END_OF_STREAM) {
					if (t.type == TOKEN_WORD || t.type == TOKEN_FILE) {
						if (t.str[0] == '.' || t.str[0] == ',') {
							content->paragraph.words[content->paragraph.wordCount].noSpace = true;
						}
						content->paragraph.words[content->paragraph.wordCount].type = PARAGRAPH_WORD_WORD;
						strcpy(content->paragraph.words[content->paragraph.wordCount].str, t.str);
						++content->paragraph.wordCount;
					} else if (t.type == TOKEN_OPEN_BRACKET) {
						content->paragraph.words[content->paragraph.wordCount].type = PARAGRAPH_WORD_LINK;
						t = GetContentToken(&tizer);
						if (t.type == TOKEN_FILE) {
							strcpy(content->paragraph.words[content->paragraph.wordCount].link.url, t.str);
							t = ReadUntilCloseBracket(&tizer);
							strcpy(content->paragraph.words[content->paragraph.wordCount].link.str, t.str);
							/*if (t.type == TOKEN_WORD) {
								
								t = GetContentToken(&tizer);
							} else if (t.type == TOKEN_CLOSE_BRACKET) {
								
							}*/
						}

						++content->paragraph.wordCount;
					} else if (t.type == TOKEN_BACKSLASH) {
						t = GetContentToken(&tizer);
						if (t.type == TOKEN_NEWLINE) {
							content->paragraph.words[content->paragraph.wordCount].type = PARAGRAPH_LINE_BREAK;
							// AddContent(p, content);
							++content->paragraph.wordCount;
						}
					}

					t = GetContentToken(&tizer);
				}
				
				// strcpy(content->paragraph.str, t.str);

				AddContent(p, content);
			}
		} else if (t.type == TOKEN_FILE) {
			content->type = CONTENT_IMAGE;
			strcpy(content->image.fileName, t.str);
			t = ReadUntilNewLine(&tizer);
			strcpy(content->image.caption, t.str);
			AddContent(p, content);
		} else if (t.type == TOKEN_COMMENT) {
			ReadUntilNewLine(&tizer);
		}

		t = GetContentToken(&tizer);
	}
}

void Compile ()
{
	perf_t perfStartTime = GetPerfTime();

	printf("\nCompiling... \n");

	ClearMemory();

#if 0
	{
		file_list pageFiles = GetFileList("*.html");
		fiz (pageFiles.count) {
			page *currentPage = &pageList->pages[pageList->count];

			char *Mem = PushMemory(strlen(pageFiles.files[i].name) + 1);
			strcpy(Mem, pageFiles.files[i].name);
			*(Mem + strlen(pageFiles.files[i].name)) = 0;
			currentPage->FileName = Mem;

			char *FileData = ReadFileDataOrError(Mem);
			currentPage->Data = FileData;

			++pageList->count;
		}
	}

	{
		file_list postFiles = GetFileList("posts/*.html");
		fiz (postFiles.count) {
			char *Mem = PushMemory(strlen("posts/") + strlen(postFiles.files[i].name) + 1);
			strcpy(Mem, "posts/");
			strcpy(Mem + strlen("posts/"), postFiles.files[i].name);
			*(Mem + strlen(postFiles.files[i].name) + strlen("posts/")) = 0;

			printf("Parsing %s \n", postFiles.files[i].name);

			page *currentPage = &pageList->pages[pageList->count];
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

			currentPage->Data = tizer.strp;
			// currentPage->Data = FileData;

			++pageList->count;
		}
	}
#endif


	page_list *blogList = (page_list*)PushMemory(sizeof(page_list));
	file_list blogFiles = GetFileList("posts/*.blog");
	fiz (blogFiles.count) {
		page *blogPage = &blogList->pages[blogList->count];

		char s[64];
		sprintf(s, "posts/%s", blogFiles.files[i].name);

		char *blogFileData = ReadFileDataOrError(s);
		if (blogFileData) {
			// page *blogPage = (page*)PushMemory(sizeof(page));
			blogPage->FileName = blogFiles.files[i].name;
			blogPage->Post = true;
			ParsePage(blogPage, blogFileData); // NOTE: ParsePage zeros the structure

			++blogList->count;
		}
	}

	BubbleSortFilesLatestTop(blogList);

	page_list *pageList = (page_list*)PushMemory(sizeof(page_list));
	file_list pageFiles = GetFileList("*.page");
	fiz (pageFiles.count) {
		page *p = &pageList->pages[pageList->count];

		char s[64];
		sprintf(s, "%s", pageFiles.files[i].name);
		char *pageFileData = ReadFileDataOrError(s);
		if (pageFileData) {
			p->FileName = pageFiles.files[i].name;
			ParsePage(p, pageFileData); // NOTE: ParsePage zeros the structure

			++pageList->count;
		}
	}

#if 0
	fiz (pageList->count)
	{
		page *currentPage = &pageList->pages[i];

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
	}
#endif

	fiz (pageList->count) {
		page *p = &pageList->pages[i];
		if (p->DateString) {
			// printf("date %2i %2i %4i, sortkey 0x%8x %i \n", f->Date.Day, f->Date.Month, f->Date.Year, f->DateSortKey, f->DateSortKey);
		}
	}

	menu pageMenu = LoadMenuConfig();
	char *styleFileData = ReadFileDataOrError("style.css");

	mkdir("output");
	mkdir("output/posts");

#if 0
	fiz (pageList->count)
	{
		page *currentPage = &pageList->pages[i];

		if (strcmp(currentPage->FileName, "template.html") != 0)
		{
			char *FileData = currentPage->Data;

			char *OutputFileName = PushMemory(strlen(currentPage->FileName) + strlen("output/") + 1);
			sprintf(OutputFileName, "output/%s\0", currentPage->FileName);

			FILE *OutputFileHandle = fopen(OutputFileName, "w");
			if (!OutputFileHandle) {
				printf("FileError: %s \n", strerror(errno));
			}

			O_TemplateHeader(OutputFileHandle, &pageMenu, styleFileData, currentPage);

			Assert(OutputFileHandle);

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

							fjz (pageList->count)
							{
								page *currentPost = &pageList->pages[j];

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
												fputs(GetPrintDate(currentPost), OutputFileHandle);
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

			O_TemplateFooter(OutputFileHandle);

			fclose(OutputFileHandle);
		}
	}
#endif

	fiz (blogList->count) {
		page *currentPage = &blogList->pages[i];

		char OutputFileName[64];

		if (currentPage->Post) {
			sprintf(OutputFileName, "output/posts/%s.html\0", blogList->pages[i].FileName);
		} else {
			sprintf(OutputFileName, "output/%s.html\0", blogList->pages[i].FileName);
		}
		FILE *OutputFileHandle = fopen(OutputFileName, "w");

		if (!OutputFileHandle)
		{
			// Error("Unable to open template file");
			printf("FileError: %s \n", strerror(errno));
		}

		O_TemplateHeader(OutputFileHandle, &pageMenu, styleFileData, currentPage);
		O_Page(OutputFileHandle, currentPage, blogList);
		// O_BlogList(OutputFileHandle, blogList);
		O_TemplateFooter(OutputFileHandle);

		fclose(OutputFileHandle);
	}

	fiz (pageList->count) {
		page *currentPage = &pageList->pages[i];

		char OutputFileName[64];

		if (currentPage->Post) {
			sprintf(OutputFileName, "output/posts/%s.html\0", currentPage->FileName);
		} else {
			sprintf(OutputFileName, "output/%s.html\0", currentPage->FileName);
		}
		FILE *OutputFileHandle = fopen(OutputFileName, "w");

		if (!OutputFileHandle)
		{
			// Error("Unable to open template file");
			printf("FileError: %s \n", strerror(errno));
		}

		O_TemplateHeader(OutputFileHandle, &pageMenu, styleFileData, currentPage);
		O_Page(OutputFileHandle, currentPage, blogList);
		// O_BlogList(OutputFileHandle, blogList);
		O_TemplateFooter(OutputFileHandle);

		fclose(OutputFileHandle);
	}


	float compileTime = SecondsElapsed(perfStartTime);

	float fMemoryUsed = (float)(MemoryUsed / KiloBytes(1));
	float fMemorySize = (float)(MemorySize / KiloBytes(1));
	float memoryPercentage = (fMemoryUsed / fMemorySize) * 100.0f;
	printf("Finished in %.3fs, memory %ikb %i%% \n", compileTime, MemoryUsed / KiloBytes(1), (s32)memoryPercentage);
}

int main (int argc, char **argv)
{
	/*printf("%i", argc);
	printf("%s", argv[0]);
	printf("%s", argv[1]);*/
	if (argc > 1) {
		if (strcmp(argv[1], "giantjelly") == 0) {
			siteId = 0;
		}
		if (strcmp(argv[1], "matt") == 0) {
			siteId = 1;
		}
	}

	// TestRecursiveFileList();

	InitMemory();

	file_list masterFileList = {};

	/*do*/ {
		file_list fl0 = GetFileList("*.page");
		file_list fl1 = GetFileList("posts/*.blog");
		file_list fl2 = GetFileList("*.cfg");
		file_list fl3 = GetFileList("*.css");
		file_list fileList0 = ConcatFileList(fl0, fl1);
		file_list fileList1 = ConcatFileList(fl2, fl3);
		file_list fileList = ConcatFileList(fileList0, fileList1);

		if (fileList.count != masterFileList.count)
		{
			Compile();
			masterFileList = fileList;
		}
		else
		{
			forc (fileList.count)
			{
				if (
#ifdef _WIN32
					CompareFileTime((FILETIME*)&fileList.files[i].writeTime, (FILETIME*)&masterFileList.files[i].writeTime)
#endif
#ifdef __APPLE__
					fileList.files[i].writeTime != masterFileList.files[i].writeTime
#endif
					)
				{
					Compile();
					masterFileList = fileList;
					break;
				}
			}
		}

		// Sleep(1000);
	} /*while (true);*/
	// note: disabled this looping to try using generator in a sublime build

	// system("pause");
	return 0;
}
