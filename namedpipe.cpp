#include <conio.h>
#include <string>
#include <process.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <iostream>
#include <tchar.h>
#include <time.h>
#include <malloc.h>


int main(int arg, char* argv[]) {
	setlocale(LC_ALL, "Russian");
	int  count = atoi(argv[0]);
	double size = 690000000;
	TCHAR nam[] = TEXT("sem");
	DWORD chit;
	HANDLE  Pipe, sem;
	char *buf;
	buf = (char*)malloc(size * sizeof(char));
	Pipe = CreateFile(L"\\\\.\\pipe\\pipename", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (Pipe == INVALID_HANDLE_VALUE)
	{
		printf("Error create file\n");
		return 0;
	}
	sem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, nam);
	printf(" ");
	for (int i = 0; i < count; i++){
		WriteFile(Pipe, buf, size, &chit, NULL);
		ReleaseSemaphore(sem, 1, NULL);
		WaitForSingleObject(sem, INFINITE);
	}
	CloseHandle(Pipe);
	CloseHandle(sem);
	free(buf);
	return 1;
}