#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
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
#pragma comment(lib,"Ws2_32.lib")
#define RAND_MAX 32767
#define RAND_MIN -32767

#define N 10                //кол-во записывания/считывания 
#define count 5             //кол-во тестов
#define NMMX 200000000      //для цикла в ммх
double size = 690000000;    //размер буфера для каналов

LARGE_INTEGER frequency, start, finish;
double *test_result;


int fun1(); //unnamed pipe
int fun2(); //named pipe
int fun3(); //tcp
int fun4(); //mmx
int fun5(); //cuda nvidia toolkit

void resultpoints();
LPWSTR cmd();  //преобразование числа для записи к командную строку 
 
void main(){
	setlocale(LC_ALL, "Russian");
	int n;
	printf("\nBenchmark for OS Windows\n");
	do{
		printf("\nДля начала теста введите '1'\nПоказать результаты: '2'\nВыход: '3'\n");
		scanf("%d", &n);
		switch (n){
		case 1:{
			test_result = (double*)malloc(count*sizeof(double));
			if (!fun1())printf("\nError test unnamed pipe\n"); else printf(" Test for unnamed pipe complete!\n");
			if (!fun2())printf("\nError test named pipe\n"); else printf("Test for named pipe complete!\n");
			if (!fun3())printf("\nError test tcp socket\n"); else printf("Test for tcp socket complete!\n");
			if (!fun4())printf("\nError test MMX\n"); else printf(" Test for MMX complete!\n");
			if (!fun5())printf("\nError test GPU\n"); else printf(" Test for GPU complete!\n");
			break; 
		}
		case 2:{
			if (test_result)
			resultpoints();
			else
				printf("Тест еще не был проведен\n");
			break; 
		}
		case 3:{
			free(test_result);
			return ;
			break;
		}
		default: scanf("%d", &n);
		}
	} while (1);
}
LPWSTR cmd(){
	char str[5] = ("");
	LPWSTR res;
	DWORD res_len = MultiByteToWideChar(1251, 0, _itoa(N, str, 10), -1, NULL, 0);
	res = (LPWSTR)GlobalAlloc(GPTR, (res_len + 1) * sizeof(WCHAR));
	MultiByteToWideChar(1251, 0, _itoa(N, str, 10), -1, res, res_len);
	return res;
}
int fun1(){  
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof si);
	si.cb = sizeof si;
	ZeroMemory(&pi, sizeof pi);
	TCHAR *lol = TEXT("D://TiP/labsvis/Benchmark/unnamedpipe.exe");
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	if (!CreateProcess(lol, cmd(), NULL, NULL, false, NULL, NULL, NULL, &si, &pi))
		return 0;
	WaitForSingleObject(pi.hProcess, INFINITE);
	QueryPerformanceCounter(&finish);
	double delay = ((finish.QuadPart - start.QuadPart)*1000.0f / (double)frequency.QuadPart);
	test_result[0] = (N * size * 2 / (delay/1000)) / 1000000;
	return 1;
}      
int fun2(){
	TCHAR	*lol = TEXT("D://TiP/labsvis/Benchmark/namedpipe.exe"), name[] = TEXT("sem");
	HANDLE  Pipe, sem;
	DWORD chit;
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { sizeof(si) };
	char *buf;
	buf = (char*)malloc(size*sizeof(char));
	Pipe = CreateNamedPipe(L"\\\\.\\pipe\\pipename", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, size, size, INFINITE, NULL);
	if (Pipe == INVALID_HANDLE_VALUE){
		printf("\npipe!!\n");
		return 0;
	}
	LARGE_INTEGER frequency, start, finish;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	if (!CreateProcess(lol, cmd(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)){
		printf("Error proc\n");
		return 0;
	}
	if (!(sem = CreateSemaphore(NULL, 0, 1,name))){
		printf("Error\n");
		return 0;
	}
	if (!ConnectNamedPipe(Pipe, NULL)){
		printf("Connect Error\n");
		CloseHandle(Pipe);
		CloseHandle(sem);
		return 0;
	}
	for (int i = 0; i < N; i++){
		WaitForSingleObject(sem, INFINITE);
		ReadFile(Pipe, buf, size, &chit, NULL);
		ReleaseSemaphore(sem,1,NULL);
	}
	QueryPerformanceCounter(&finish);
	double delay = ((finish.QuadPart - start.QuadPart)*1000.0f / (double)frequency.QuadPart);
	test_result[1] = ((N * 2 * size / (delay/1000)) / 1000000);
	CloseHandle(Pipe);
	CloseHandle(sem);
	free(buf);
	return 1;
}
int fun3(){
	WSADATA WsaData;
	TCHAR	*lol = TEXT("D://TiP/labsvis/Benchmark/stcp.exe"), name[] = TEXT("sema");
	HANDLE  sem;
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { sizeof(si) };
	int size1 = 700000;
	int count1 = 50;
	char *buf;
	buf = (char*)malloc(size1*sizeof(char));
	int wsa = WSAStartup(0x0101, &WsaData);
	if (wsa == SOCKET_ERROR){
		printf("WSAStartup() failed\n");
		return 0;
	}
	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == INVALID_SOCKET){
		printf("Error create socket\n");
		WSACleanup();
		return 0;
	}
	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	sin.sin_addr.s_addr = INADDR_ANY;
	wsa = bind(server, (LPSOCKADDR)&sin, sizeof(sin));
	if (server == SOCKET_ERROR){
		printf("Error bind");
		closesocket(server);
		WSACleanup();
		return 0;
	}
	LARGE_INTEGER frequency, start, finish;
	wsa = listen(server, 10);           
	if (wsa == SOCKET_ERROR){
		printf("Error listen\n");
		closesocket(server);
		WSACleanup();
		return 0;
	}
	SOCKET client;
	if (!(sem = CreateSemaphore(NULL, 0, 1, name))){
		printf("Error\n");
		return 0;
	}
	if (!CreateProcess(lol, cmd(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)){
		printf("Error proc\n");
		return 0;
	}
	client = accept(server, NULL, NULL);   // здесь жду
	if (client == INVALID_SOCKET){
		printf("Error accept\n");
		closesocket(server);
		CloseHandle(sem);
		WSACleanup();
		return 0;
	}	
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	for (int i = 0; i < count1; i++){
		WaitForSingleObject(sem, INFINITE);
		recv(client, buf, size1, 0);
		ReleaseSemaphore(sem, 1, NULL);
	}
	QueryPerformanceCounter(&finish);
	double delay = ((finish.QuadPart - start.QuadPart)*1000.0f / (double)frequency.QuadPart);
	test_result[2] = ((count1 * 2 * size1 / (delay / 1000)) / 1000000);
	closesocket(client);
	closesocket(server);
	CloseHandle(sem);
	free(buf);
	WSACleanup();

	return 1;
}
int fun4(){
	short ms[16], ms1[16], ms2[16];
	int cnt = 0;
	srand(time(NULL));
	for (int i = 0; i < 16; i++){
		ms[i] = rand();
		ms1[i] = rand();
	}
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	_asm
	{
		pusha
		xor ecx, ecx
		mov ecx, NMMX
	loop_start2 :
		xor esi, esi
		mov cnt, 16
	loop2 :
		  movq MM0, ms[esi]
		  movq MM1, ms1[esi]
		  pminsw MM0, MM1
		  pmulhw MM0, MM1
		  psubw MM0, MM1
		  movd ms2[esi], MM0
		  add esi, 4
		  sub cnt, 2
		  jnz loop2
		  xor esi, esi
		  mov cnt, 16
	  loop3 :
		movq MM0, ms[esi]
		movq MM1, ms1[esi]
		movq MM2, ms2[esi]
		pmaxsw MM1, MM2
		psubw MM1, MM2
		pmulhw MM0, MM1
		movd ms2[esi], MM0
		add esi, 4
		sub cnt, 2
		jnz loop3
		loop loop_start2
		emms
		popa
	}
	QueryPerformanceCounter(&finish);
	double delay = (finish.QuadPart - start.QuadPart)*1000.0f / frequency.QuadPart;
	test_result[3] = delay / 1000;
	return 1;
}
int fun5(){
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof si);
	si.cb = sizeof si;
	ZeroMemory(&pi, sizeof pi);
	TCHAR *lol = TEXT("D://TiP/labsvis/Benchmark/test-gpu.exe");
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	if (!CreateProcess(lol, NULL, NULL, NULL, false, NULL, NULL, NULL, &si, &pi))
		return 0;
	WaitForSingleObject(pi.hProcess, INFINITE);
	QueryPerformanceCounter(&finish);
	double delay = ((finish.QuadPart - start.QuadPart)*1000.0f / (double)frequency.QuadPart);
	test_result[4] = delay / 1000;
	return 1;
}

void resultpoints(){
	int point[count], max_procent = 100, result_point = 0;
	double best_score[count], coef[5];
	best_score[0] = 5900; 
	best_score[1] = 6666;
	best_score[2] = 1240;
	best_score[3] = 2.7809;
	best_score[4] = 6.540;
	coef[0] = 0.17;
	coef[1] = 0.17;
	coef[2] = 0.10;
	coef[3] = 0.21;
	coef[4] = 0.35;
	for (int i = 0; i < count; i++){
		if (i == 3 || i == 4){
			point[i] = (best_score[i] * max_procent) / test_result[i];
			printf("\nТест %d: %d баллов", i + 1, point[i]);
		}
		else{
			point[i] = (test_result[i] * max_procent) / best_score[i];
			printf("\nТест %d: %d баллов", i + 1, point[i]);
		}
	}
	for (int i = 0; i < count; i++)
	{
		result_point += point[i] * coef[i];
	}
	int k;
	printf("\nВаш компьтер набрал: %d баллов из 100\n", result_point);
	printf("Если желаете узнать подробные результаты тестировки введите '5'\n");
	scanf("%d", &k);
	if (k==5)
	{
		printf("\nСкорость передачи данных по unnamed pipe: %.0fMB/s\n",  test_result[0]);
		printf("Скорость передачи данных по named pipe: %.0fMb/s\n", test_result[1]);
		printf("Скорость передачи данных через TCP: %.0fMb/s\n", test_result[2]);
		printf("Время высчитывания сложной функции MMX : %f sec\n", test_result[3]);
		printf("Время высчитывания сложной функции GPU : %f sec\n", test_result[4]);	
	}
	fflush(stdin);
	getchar();
	system("cls");
}