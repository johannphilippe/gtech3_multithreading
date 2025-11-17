# La création de threads avec la WIN32 API 

## Créer un thread 

La fonction pour créer un thread est la suivante : 
``` cpp
HANDLE CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes, // Ou NULL
    SIZE_T dwStackSize, // Ou 0 pour la stack size par défaut (1Mb)
    LP_THREAD_START_ROUTINE lpStartAddress, // Point d'entrée du thread
    __drv_aliasesMem LPVOID lpParameter, // paramètres à passer au point d'entrée du Thread 
    DWORD dwCreationFlags, // Permet de créer soit un thread qui s'exécute immédiatement, soit un thread suspendu et de spécifier des paramètres de stack 
    LPDWORD lpThreadId, // Ou NULL si on n'en a pas besoin 
);
```
Le thread va ensuite s'exécuter de manière parallèle, autonome, jusqu'à ce qu'on appelle la fonction suivante : 

``` cpp
    WaitForSingleObject(HANDLE, DWORD milliseconds_to_wait);
    // INFINITE can be used for wait time
```
Cette dernière va bloquer le thread appelant (le principal dans l'exemple suivant) jusqu'à ce que le thread attendu termine son exécution. 

Remarque : 
- Le nombre maximum de threads dans un programme est corrélé au paramètre `dwStackSize`


``` cpp 
    BOOL GetExitCodeThread(
    HANDLE  hThread, // Note handle de thread 
    LPDWORD lpExitCode // Un pointeur vers la valeur de retour du thread
    );
```

### Exemple : 
``` cpp
DWORD WINAPI ThreadRoutine(LPVOID params)
{
    LPDWORD array = (LPDWORD)params;

    // On peut récupérer le HANDLE du thread avec la fonction suivante : 
    HANDLE this_thread = GetCurrentThread(); 

    while(true) {
        // Do something with data

        if(anything_wrong) 
            ExitThread(SOME_ERROR_CODE);

        if(nothing_to_do)
            Sleep(10); // Milliseconds, pour éviter le polling permanent
    }
    return 0;
}

int main() 
{
    DWORD thread_id; 

    // Dummy memory allocation 
    LPVOID params = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DWORD) * 100);

    // On créé le thread
    HANDLE thread = CreateThread(
        NULL, // Security attributes 
        0, // Stack size 
        ThreadRoutine, 
        params, 
        0, // Le thread démarre immédiatement 
        &thread_id
    );

    // On vérifie si le thread est bien créé 
    if (thread == NULL) {
        printf("Failed to create thread, thread ID : %d", thread_id);
        ExitProcess(3);
    }

    // Sans cet appel, on créé un problème : le thread principal (fonction main) arrive à la fin de son exécution avant que le thread créé n'ait terminé son exécution : le programme s'interrompt, sans qu'on ait pu valider si la tâche du thread s'est terminée 
    DWORD wait_res = WaitForSingleObject(thread, INFINITE); 
    if(wait_res != 0) {
        printf("Thread returned with error : %d \n", res);
    }

    // Supprimer l'objet thread 
    BOOL thread_destruction_res = CloseHandle(thread); 
    // Penser aussi à supprimer la mémoire allouée "params" 

    return wait_res;
}
```

## La manipulation des threads depuis le thread parent 

On peut arrêter ou reprendre l'exécution d'un thread depuis un autre thread, grâce à son `HANDLE`.

``` cpp 
    // Si la fonction renvoie -1, c'est un échec, sinon elle renvoie le nombre de suspensions précédentes 
    DWORD SuspendThread(
        HANNDLE thread
    );

    // Utilisé lorsqu'un thread a été suspendu ou qu'il a été créé avec le flag de suspension 
    // Renvoie la même chose que SuspendThread
    DWORD ResumeThread(
        HANDLE thread
    );
```

## Exercice Whack a thread 

On créé un programme qui fait apparaître des threads aléatoirement, et l'utilisateur doit utiliser une touche pour le "tuer". 
- Chaque thread apparaît pour une durée de vie de 0.5 à quelques secondes 
- Une touche aléatoire sur laquelle il faut appuyer pour le faire disparaître (à chaque fois différente)
- Si l'utilisateur appuie dans le temps imparti, le thread est tué, et l'utilisateur gagne un point
- S'il ne parvient pas à temps, il perd un point 
- Un réglage de difficulté pour faire apparaître des threads plus vite & pour moins longtemps 

## Le Thread Local Storage

Le thread local storage est un outil permettant à chaque Thread de stocker une valeurs locales du thread. Chaque thread a son emplacement pour l'index renvoyé par TlsAlloc. 

Plusieurs patterns possibles pour gérer plusieurs objets dans la mémoire locale des threads : 
- Plusieurs indexes de stockage (plusieurs TlsAlloc)
- La valeur stockée est un pointeur opaque vers un objet d'une structure de donnée complexe et composite 

``` cpp
    DWORD TlsAlloc(); // Renvoie un index TLS  
    BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue); //True si réussi
    LPVOID TlsGetValue(DWORD dwTlsIndex); 
    BOOL TlsFree(DWORD dwTlsIndex); // True si réussi, sinon appeler GetLastError 
```

## La priorité des Threads 

On peut spécifier la priorité système d'un thread, et modifier la manière donc le scheduler du système va allouer du temps de calcul à ce thread. 

[Thread Priority](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-setthreadpriority)

``` cpp
    // Renvoie True si réussi
    BOOL SetThreadPriority(
        HANDLE hThread,  // le Handle du thread dont on change la priorité 
        int nPriority // La valeur de priorité 
    );
```

Cette fonction entre en interaction serrée avec un autre attribut de priorité : la classe de priorité du processus lui-même. 


[Process Priority class](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-setpriorityclass)

``` cpp 
    BOOL SetPriorityClass(
        HANDLE hProcess, 
        DWORD dwPriorityClass,  // La priorité 
    );

```

### Remarque 

On s'intéresse rarement à la priorité des processus & des threads. 
Ces notions correspondent à des programmes soumis à des contraintes très spécifiques, dont le jeu vidéo peut faire partie. Les logiciels audio temps-réel font appel à la priorité de processus REALTIME, et la priorité de thread TIME_CRITICAL. 
Mais dans des cas plus triviaux, il n'est pas nécessaire de s'intéresser à cette question (par défaut, les threads sont prévus par le système pour s'exécuter aussi vite que possible, de manière fluide et en symbiose avec les autres threads).