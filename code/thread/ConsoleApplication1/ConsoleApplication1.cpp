// ConsoleApplication1.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//
#include<Windows.h>
#include<cstdio>
#include<processthreadsapi.h>

int global_var = 0;

struct ThreadData
{
    int val;
};
 
DWORD WINAPI StartingPoint(LPVOID param)
{
    DWORD main_thread_id = GetCurrentThreadId() ;
    global_var += 1;
    printf("New Thread ID from inside : %d \n", main_thread_id);
    return 0; 
};

int main()
{
    DWORD main_thread_id = GetCurrentThreadId();
    printf("Main Thread ID : %d \n", main_thread_id );
    
    printf("Global variable is %d \n", global_var);

    /*
        Ici la déclaration/définition des arguments pour la création du thread 
    */
    SIZE_T stack_size = 0; 
    LPVOID params = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ThreadData));

    DWORD creation_flags = 0;
    DWORD thread_id;
    HANDLE thread = CreateThread(NULL, stack_size, StartingPoint, params, creation_flags, &thread_id);
    
    if (thread == NULL) {
        printf("Failed to create thread, thread ID : %d", thread_id);
        ExitProcess(3);
    }

    printf("New Thread ID : %d \n", thread_id );
    WaitForSingleObject(thread, INFINITE);

    printf("Global variable is now %d \n", global_var);

    CloseHandle(thread);
    HeapFree(GetProcessHeap(), 0, params);
    return 0;
}
