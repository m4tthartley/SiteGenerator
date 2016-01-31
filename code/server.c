
// #include <windows.h>
#include <winsock2.h>
#include <stdint.h>

#include "shared.c"

typedef uint32_t u32;
typedef int32_t s32;

#define PORT "1111"

typedef struct
{
	char *Request;
	char *Get;
	char *Host;

	s32 TokenizerIndex;
} server_request;

char *ReadToken (server_request *ServerRequest)
{
	char *c = &ServerRequest->Request[ServerRequest->TokenizerIndex];
	if (!*c)
	{
		return NULL;
	}
	char *Start = c;
	s32 Length = 0;
	while (*c != 0 &&
		   *c != ' ' &&
		   *c != '\n' &&
		   *c != '\r')
	{
		++Length;
		++c;
	}

	if (!Length)
	{
		++ServerRequest->TokenizerIndex;
		return ReadToken(ServerRequest);
	}
	
	ServerRequest->TokenizerIndex += Length;

	char *Token = PushMemory(Length + 1);
	strncpy(Token, Start, Length);
	Token[Length] = 0;
	return Token;
}

typedef struct
{
	char *Extension;
	char *HttpFileType;
} file_type;
file_type FileTypes[] =
{
	{"html", "text/html"},
	{"css", "text/css"},
	{"js", "text/js"},
	{"txt", "text/txt"},
	{"png", "image/png"},
	{"jpg", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"bmp", "image/bmp"},
};

char *GetFileTypeForExtension (char *Extension)
{
	s32 Size = sizeof(FileTypes)/sizeof(file_type);
	forc (Size)
	{
		if (!strcmp(FileTypes[i].Extension, Extension))
		{
			return FileTypes[i].HttpFileType;
		}
	}

	return NULL;
}

int main ()
{
	GetFileTypeForExtension("");
	InitMemory();

	printf("Staring server... \n");

	// int RequestBufferSize = 1000000;
	// char *RequestBuffer = (char*)malloc(RequestBufferSize);
	// ZeroMemory(RequestBuffer, RequestBufferSize);

	WSADATA WSAData;
	SOCKET ServerSocket = INVALID_SOCKET;
	SOCKET ConnectionSocket = INVALID_SOCKET;

	int StartupResult = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (StartupResult)
	{
		printf("WinSock startup failed \n");
	}

	struct addrinfo *AddressInfo = NULL;
	struct addrinfo AddressInfoRequest = {0};
	AddressInfoRequest.ai_family = AF_INET;
	AddressInfoRequest.ai_socktype = SOCK_STREAM;
	AddressInfoRequest.ai_protocol = IPPROTO_TCP;
	AddressInfoRequest.ai_flags = AI_PASSIVE;
	int AddressInfoResult = getaddrinfo(NULL, PORT, &AddressInfoRequest, &AddressInfo);
	if (AddressInfoResult)
	{
		printf("Get address info failed \n");
	}

	ServerSocket = socket(AddressInfo->ai_family, AddressInfo->ai_socktype, AddressInfo->ai_protocol);
	if (ServerSocket == INVALID_SOCKET)
	{
		printf("Create socket failed \n");
	}

	int BindResult = bind(ServerSocket, AddressInfo->ai_addr, AddressInfo->ai_addrlen);
	if (BindResult == SOCKET_ERROR)
	{
		printf("Bind socket failed \n");
	}

	// Free addrinfo? Hahaha good one

	int ListenResult = listen(ServerSocket, SOMAXCONN);
	if (ListenResult == SOCKET_ERROR)
	{
		printf("Listen failed \n");
	}

	// Close server socket, not sure why

	while (TRUE)
	{
		ClearMemory();

		s32 BufferSize = KiloBytes(1);
		char *Buffer = PushMemory(BufferSize);

		ConnectionSocket = accept(ServerSocket, NULL, NULL);
		if (ConnectionSocket == INVALID_SOCKET)
		{
			printf("Accept failed \n");
		}

		int RequestBytes = recv(ConnectionSocket, Buffer, BufferSize, 0);
		if (RequestBytes > 0)
		{
			// printf("Request bytes: %i \nRequest: %s \n", RecvResult, RequestBuffer);

			server_request ServerRequest = {0};
			ServerRequest.Request = Buffer;

			char *t;
			while (t = ReadToken(&ServerRequest))
			{
				if (!strcmp(t, "GET"))
				{
					char *t1 = ReadToken(&ServerRequest);
					ServerRequest.Get = t1;
				}
				if (!strcmp(t, "Host:"))
				{
					char *t1 = ReadToken(&ServerRequest);
					ServerRequest.Host = t1;
				}

				// printf("t: %s \n", t);
			}

			printf("Request: ");
			printf(ServerRequest.Get);
			printf("\n");

			#define Headers "HTTP/1.1 %i OK\r\n"\
							"Date: %s\r\n"\
							"Server: Linux\r\n"\
							"Accept-Ranges: none\r\n"\
							"Content-Type: %s\r\n"\
							"Content-Length: %i\r\n"\
							"Connection: close\r\n"\
							"\r\n""\0"



			char *Output = PushMemory(KiloBytes(20));
			if (ServerRequest.Get[0] == '/')
			{
				++ServerRequest.Get;
			}
			file_data FileData;
			char *FileName;
			if (*ServerRequest.Get)
			{
				FileName = ServerRequest.Get;
			}
			else
			{
				FileName = "index.html";
			}

			time_t tm;
			time(&tm);
			char *Date = PushMemory(KiloBytes(1));
			strftime(Date, KiloBytes(1), "%a, %d %b %Y %X GMT", localtime(&tm));

			char *Ext = strrchr(FileName, '.');
			if (!Ext)
			{
				char *fn = PushMemory(strlen(FileName) + 6);
				//FileName = fn;
				strcat(fn, FileName);
				strcat(fn, ".html\0");
				if (FExists(fn))
				{
					FileName = fn;
				}
			}

			FileData = FRead(FileName);

			if (FileData.Data && strcmp(FileName, "404.html") && strcmp(FileName, "404"))
			{
				// char *Ext = PathFindExtensionA(FileName);
				char *Ext = strrchr(FileName, '.');
				char *HttpFileType;
				HttpFileType = GetFileTypeForExtension(Ext + 1);
				if (!HttpFileType)
				{
					HttpFileType = "text/html";
				}

				sprintf(Output, Headers, 200, Date, HttpFileType, FileData.Size);
				// memcpy(Output + strlen(Output), FileData.Data, FileData.Size);
				int SendResult;
				s32 HeaderBytesSent = send(ConnectionSocket, Output, strlen(Output), 0);
				s32 DataBytesSent = send(ConnectionSocket, FileData.Data, FileData.Size, 0);
				printf("HeaderBytesSent: %i \n", HeaderBytesSent);
				printf("DataBytesSent: %i \n", DataBytesSent);
				if (SendResult == SOCKET_ERROR)
				{
					printf("Send to client failed \n");
				}
				// SendResult = send(ConnectionSocket, FileData.Data, FileData.Size, 0);
				if (SendResult == SOCKET_ERROR)
				{
					printf("Send to client failed \n");
				}

#if 0
				FILE *f;
				f = fopen("assets/test.bmp", "w");
				if (f)
				{
					fwrite(FileData.Data, sizeof(char), FileData.Size, f);
					fclose(f);
				}
#endif
			}
			else
			{
				// char *PageNotFoundText = "<h1>Page not found</h1>";
				file_data File404 = FRead("404.html");
				if (File404.Data)
				{
					sprintf(Output, Headers, 404, Date, "text/html", File404.Size);
					int SendResult = send(ConnectionSocket, Output, strlen(Output), 0);
					send(ConnectionSocket, File404.Data, File404.Size, 0);
					if (SendResult == SOCKET_ERROR)
					{
						printf("Send to client failed \n");
					}
				}
			}
			
			closesocket(ConnectionSocket);

		}
		else
		{
			closesocket(ConnectionSocket);
		}
	}

	system("pause");
	return 0;
}