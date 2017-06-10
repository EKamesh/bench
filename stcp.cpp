#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include <iostream>
#include <tchar.h>
#include <time.h>
#include <malloc.h>
#include <conio.h>
#include <string>
#include <stdio.h>
#pragma comment(lib,"Ws2_32.lib")

int main()
{
	setlocale(LC_ALL, "Russian");
	WSADATA WsaData;
	TCHAR name[] = TEXT("sema");
	HANDLE sem;
	int count = 50;
	int size1 = 700000;
	char *buf;
	buf = (char*)malloc(size1 * sizeof(char));
	int wsa = WSAStartup(0x0101, &WsaData);
	if (wsa == SOCKET_ERROR){
		printf("WSAStartup() failed: %ld\n", GetLastError());
		return 0;
	}
	sem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, name);
	LPHOSTENT hostEnt;
	hostEnt = gethostbyname("localhost");
	if (!hostEnt){
		printf("Error localhostname\n");
		WSACleanup();
		return 0;
	}
	SOCKET client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == SOCKET_ERROR){
		printf("Error create socket\n");
		WSACleanup();
		return 0;
	}

	SOCKADDR_IN anAddr;
	anAddr.sin_family = AF_INET;
	anAddr.sin_port = htons(80);
	anAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	wsa = connect(client, (struct sockaddr *)&anAddr, sizeof(struct sockaddr));
	if (wsa == SOCKET_ERROR){
		printf("Error connect\n");
		closesocket(client);
		WSACleanup();
		return 0;
	}
	printf(" ");
	for (int i = 0; i < count; i++){
		send(client, buf, size1, 0); 
		ReleaseSemaphore(sem, 1, NULL);
		WaitForSingleObject(sem, INFINITE);
	}
	closesocket(client);
	CloseHandle(sem);
	WSACleanup();

	return 1;
}