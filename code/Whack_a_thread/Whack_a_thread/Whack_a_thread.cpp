#pragma comment(lib, "winmm.lib")
#include<Windows.h>
#include<stdio.h>
#include<time.h>
#include<sys/timeb.h>

VOID ErrorExit(LPCSTR lpszMessage)
{
	fprintf(stderr, "%s\n", lpszMessage);

	ExitProcess(0);
}

DWORD WINAPI ThreadToWack(LPVOID param)
{
	DWORD time_to_live = *(LPDWORD)param; 
	DWORD init_time = timeGetTime();

	HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
	INPUT_RECORD buf[32];
	DWORD num_read;
	srand(time(NULL));
	int ascii_expected = (rand() % (122 - 97)) + 97;
	printf("Expected %d\n", ascii_expected);
	char expected = (char)ascii_expected;

	printf("New thread spawned, hit %c to kill it \n", expected);

	DWORD nevents;
	while (true)
	{
		DWORD elapsed = timeGetTime() - init_time;

		if (elapsed >= time_to_live)
		{
			ExitThread(-1);
		}
		BOOL console_input_nres = GetNumberOfConsoleInputEvents(
			hstdin,
			&nevents
		);

		if (nevents == 0)
			continue;

		if (!ReadConsoleInput(
			hstdin,      // input buffer handle
			buf,     // buffer to read into
			32,         // size of read buffer
			&num_read)) // number of records read
			ErrorExit("ReadConsoleInput");

		for (DWORD i = 0; i < num_read; ++i)
		{
			if (buf[i].EventType == KEY_EVENT && buf[i].Event.KeyEvent.bKeyDown)
			{
				printf("You Hit %c \n", buf[i].Event.KeyEvent.uChar.AsciiChar);
				if (buf[i].Event.KeyEvent.uChar.AsciiChar == expected)
				{
					printf("Thread killed, +1 point \n");
					ExitThread(1);
				}
			}
		}
	}

	return 0;
}

int main()
{
	srand(time(NULL));

	HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hstdin == INVALID_HANDLE_VALUE)
		ErrorExit("GetStdHandle");

	DWORD fdwMode = (ENABLE_MOUSE_INPUT) & ~ENABLE_QUICK_EDIT_MODE;
	INPUT_RECORD irInBuf[128];
	DWORD num_read;

	printf("Press one key to start\n");
	if (!SetConsoleMode(hstdin, fdwMode))
		ErrorExit("SetConsoleMode");

	if (!ReadConsoleInput(
		hstdin,      // input buffer handle
		irInBuf,     // buffer to read into
		128,         // size of read buffer
		&num_read)) // number of records read
		ErrorExit("ReadConsoleInput");

	if (irInBuf[0].EventType == KEY_EVENT && irInBuf[0].Event.KeyEvent.bKeyDown)
	{
		//printf("%c", irInBuf[i].Event.KeyEvent.uChar);
		printf("Starting \n");
	}

	LONG64 score = 0;
		
	DWORD time_to_live; 
	while (true) {
		int rnd = (rand() % 1500) + 1000;
		Sleep(rnd);

		time_to_live = (rand() % 2000) + 600;

		HANDLE thread = CreateThread(NULL, 0, ThreadToWack, (LPVOID)&time_to_live, NULL, NULL);
		if (thread == NULL) {
			printf("Error creating thread, exiting\n");
			ExitProcess(3);
		}

		DWORD res = WaitForSingleObject(thread, INFINITE);

		DWORD exit_from_thread;
		BOOL exit_res = GetExitCodeThread(thread, &exit_from_thread);
		printf("THREAD RETURNED WITH EXIT %d \n", exit_from_thread);
		score += exit_from_thread; 
		printf("Score is now %li \n", score);
		CloseHandle(thread);
	}

	return 0;
}
