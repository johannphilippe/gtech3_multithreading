// Thread_isolated_task.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//
#include<Windows.h>
#include <cstdio>
#include<processthreadsapi.h>

struct TaskList
{
    constexpr static const UINT32 size = 40;
	INT data[size];
};

INT Fibonacci(INT n) {
    if (n <= 1) return n;
    return Fibonacci(n - 1) + Fibonacci(n - 2);
}

DWORD WINAPI ProcessData(LPVOID param)
{
    DWORD thread_id = GetCurrentThreadId();
    
    TaskList* task_list = (TaskList*)param;
    for (size_t i = 0; i < TaskList::size; ++i)
    {
        task_list->data[i] = Fibonacci(i);
        printf("Thread %d calculated Fib %d which is : %d \n", thread_id, i, task_list->data[i]);
    }

    printf("Task of thread %d is finished \n", thread_id);

    return 0;
};

int main()
{
    DWORD main_thread_id = GetCurrentThreadId();
    SIZE_T stack_size = 0;
    LPVOID params[2];
    params[0] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TaskList));
    params[1] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TaskList));

    DWORD thread_id;
    HANDLE thread = CreateThread(NULL, 0, ProcessData, params[0], 0, &thread_id);

    if (thread == NULL) {
        printf("Failed to create thread, thread ID : %d \n", thread_id);
        ExitProcess(3);
    }

    // Both main and 2nd thread will process tasks isolated in memory 
    DWORD res = ProcessData(params[1]);
    
    WaitForSingleObject(thread, INFINITE);

    CloseHandle(thread);
    HeapFree(GetProcessHeap(), 0, params);

	return 0; 
}


