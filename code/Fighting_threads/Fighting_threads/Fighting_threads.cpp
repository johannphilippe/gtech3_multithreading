// Thread_isolated_task.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//
#include<Windows.h>
#include <cstdio>
#include<processthreadsapi.h>
#include <ctime>
#include<cstdlib>

LONG goals;
LPCRITICAL_SECTION critical_section; 
DWORD WINAPI ProcessData(LPVOID param)
{
    DWORD thread_id = GetCurrentThreadId();

    LONG* increment = (LONG*)param;

    while (true) {
        InterlockedAdd(&goals, *increment);
        LONG res = InterlockedCompareExchange(&goals, 0, 0);
        EnterCriticalSection(critical_section);
        printf("Goal score is now %d : %d \n", thread_id, res);
        LeaveCriticalSection(critical_section);
        //INT ms = rand() % 500;
        //Sleep(ms);
    }
    return 0;

};

int main()
{
    srand(time(0));

    InitializeCriticalSection(critical_section);

    DWORD main_thread_id = GetCurrentThreadId();
    SIZE_T stack_size = 0;
    LONG params[2];
    params[0] = 1;
    params[1] = -1;

    DWORD thread_id;
    HANDLE thread = CreateThread(NULL, 0, ProcessData, &params[0], 0, &thread_id);

    if (thread == NULL) {
        printf("Failed to create thread, thread ID : %d \n", thread_id);
        ExitProcess(3);
    }

    // This is blocking, but thread has been launched already 
    DWORD res = ProcessData(&params[1]);

    WaitForSingleObject(thread, INFINITE);

    CloseHandle(thread);

    DeleteCriticalSection(critical_section);
    return 0;
}
