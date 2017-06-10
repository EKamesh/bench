#include <conio.h>
#include <string>
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <iostream>
#include <tchar.h>
#include <time.h>
#include <malloc.h>

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");
	int i, count = atoi(argv[0]);
	double size = 690000000;
	HANDLE hWritePipe, hReadPipe;
	DWORD tempbytes;
	char *buf;
	char *buf1;
	buf = (char*)malloc(size * sizeof(char));
	buf1 = (char*)malloc(size * sizeof(char));
 	if (!CreatePipe(&hReadPipe,&hWritePipe,NULL,size)) {
		printf("pipe");
		return 1;
	}
	for (i = 0; i < count; i++){
		WriteFile(hWritePipe, buf, size, &tempbytes, NULL);
		ReadFile(hReadPipe, buf1, size, &tempbytes, NULL);
	}
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	free(buf);
	free(buf1);
	return 0;
}