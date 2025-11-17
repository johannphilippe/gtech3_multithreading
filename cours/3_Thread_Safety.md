# Thread Safety 

Le problème principal du multithreading est la gestion de la mémoire partagée.
En effet, on souhaite éviter des data racen (la corruption mémoire), qui peut se produire lorsqu'un thread écrit sur une zone mémoire lue par un autre thread simultanément (segmentation fault, ou corruption). 

Comme nous l'avons vu dans la partie théorique, il existe plusieurs mécanismes permettant de sécuriser l'accès à des ressources partagées. 
Ces mécanismes diffèrent de ceux étudiés par Dijkstra, bien qu'on utilise encore la Sémaphore. 

Liste des verrous connus : 
- Opérations atomiques 
  - Pour WIN32, les Interlocked correspondent à ces opérations (sinon std::atomic)
- Mutex (mutual exclusive)
  - Algorithme de Dekker
  - Algorithme de Peterson 
  - Implémentation WIN32 fonction aussi entre processus 
- Critical Section 
  - Implémentation WIN32 : Similaire au Mutex, mais uniquement au sein d'un processus 
  - Très rapide et sûr 
  - Basé sur un spinlock & un sémaphore
- Spinlock 
  - Basé sur des opérations atomiques,  les thread qui tenteent d'acquérir le verrou en attente lorsque ce dernier est déjà occupé vont faire des spins, c'est-à-dire des tentatives successives jusqu'à ce qu'il soit déverouillé 
  - Très rapide, car busy waiting 
  - Ces spins coûtent des cycles CPU 
- Condition Variable
  - Un mécanisme permettant de faire dormir un thread lorsque le verrou n'est pas disponible, et de le réveiller lorsqu'il est à nouveau déverouillé (économisant ainsi des cycles CPU) 
  - Problème : le temps de réveil est un relativement long (on parle de quelques microsecondes)
  - économique en CPU : lazy waiting 
- Semaphore 
  - La sémaphore est une variable qui autorise plusieurs threads à accéder à un signal 
  - A chaque fois qu'un thread entre dans la section critique, la variable est décrémentée, lorsqu'il en sort elle est incrémentée 
  - Lorsque la variable est 0, aucun thread ne peut accéder au signal 
- SRWLock (slim reader writer)
  - Un verrou qui a deux modes : 
    - Accorde la lecture à plusieurs threads en même temps
    - Accorde l'écriture à un seul thread 
  - Il s'agit d'un verrou léger : rapide, faible empreinte mémoire 
  - A chaque tentative de verrouillage, le thread spécifie s'il tente de verrouiller en exclusif ou en partagé 

[Les objets de sécurité mémoire WIN32](https://learn.microsoft.com/fr-fr/windows/win32/sync/critical-section-objects)

## Threading *lock-free*

Les verrous ont une propriété qui peut-être ambigue : ils sont dits *lock-free* lorsqu'ils garantissent qu'au moins un thread peut progresser. 
Il ne sont pas *lock-free* si aucune garantie n'existe à ce sujet. Par exemple, si un thread qui possède un verrou est suspendu, il empêche l'exécution de tous les threads qui tenteront d'accéder à ce verrou. Situation de blocage. 
Le *lock-free* n'a donc rien a voir avec la notion de verrou (du *spinlock* par exemple).

## Les deadlocks

Le *deadlock* (interblocage) est une situation problématique déjà étudiée par Dijkstra et Dekker dans leurs études préliminaires des verrous. 
IL s'agit d'une situation d'attente circulaire dans laquelle au moins deux threads vont s'attendre indéfiniement. 

Thread A détient Data 1. \
Thread B détient Data 2. \
Thread A a besoin de Data 2 pour libérer 1  
Thread B a besoin de Data 1 pour libérer 2. 

Les deux threads vont donc attendre indéfiniement que l'autre thread libère sa ressource, ce qu'il ne fera jamais. 

## Critical Section 

La section critique de la WIN API est un mélange de plusieurs concepts de synchronisation : 
- Un spinlock
- Un sémaphore 
- Je suspecte qu'il y ait une condition variable aussi

Il s'agit d'un objet très "General Purpose", qui fonctionnera de manière optimisée dans la plupart des cas. 

Dans la programmation multithreading, on a (TRES) souvent recours à une SafeQueue.  En effet, elle répond à au moins deux besoins fondamentaux : 
- Une liste de tâches lourdes à exécuter en FIFO. Chaque Thread va aller piocher une nouvelle tâche à exécuter lorsqu'il a fini la précédente. Et, potentiellement, un autre thread va la remplir en même temps. 
- La communication entre Threads : Chaque thread est associé à une queue d'objets de types "Messages". De cette manière, un autre thread peut accéder à cette queue et lui adresser une nouvelle tâche à exécuter, un comportement à adopter ou autre. 


``` cpp 
  // On lui passe un pointeur vers la section critique (objet global, partagé par les threads)
  VOID InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection); 

  // Alternativement, on peut y adjoindre un Spin (avant d'entrer en phase Wait)
  BOOL InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);

  // Si on utilise l'initialisation classique, on peut toujours appeler 
  DWORD SetCriticalSectionSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);

  // On entre dans la section critique, sinon le thread spin & entre en attente (si elle est verouillée).
  // Cette fonction peut lever des exceptions : EXCEPTION_POSSIBLE_DEADLOCK, STATUS_POSSIBLE_DEADLOCK. 
  VOID EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

  // Alternativement, si on veut juste essayer d'entrer 
  // Renvoie si on a pu acquérir le verrou 
  BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection); 

  // On oublie évidemment pas 
  VOID LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

  // On oublie pas non plus 
  VOID DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
```


## Exercice : création d'une SafeQueue  

Consignes : 
- Utiliser CriticalSection pour la synchronisation 
- Tenter d'utiliser cette queue avec au moins 2 threads clients (pop) et deux threads serveurs (push) en même temps, sans aucun Sleep. 
- La Queue doit être une classe (template) : de taille statique pour commencer (spécifiée en template en plus du type)

## Les condition variables  & Events

Les condition variables sont également des outils essentiels dans la gestion de la synchronisation multithreading, puisqu'elle permet simplement de mettre des threads en attente (Sleep, ne consomme plus de CPU) puis de les réveiller lorsque la condition variable est signalée. Ce n'est pas à proprement parler un outil de sécurité mémoire, mais cela permet d'économiser beaucoup de cycles CPU (0 polling).
On les utilise associées à des locks (Critical Section ou SRWLock). 

[Conditions Variables](https://learn.microsoft.com/en-us/windows/win32/sync/condition-variables)

``` cpp 
BOOL SleepConditionVariableCS(
  PCONDITION_VARIABLE ConditionVariable, 
  PCRITICAL_SECTION CriticalSection, 
  DWORD timeout
);

/* Cette fonction précédente va : 
  - Release la condition variable 
  - Dormir jusqu'à ce que l'une des fonctions suivantes soit appelée, ou que le timeout soit dépassé
*/

VOID WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable); // Réveille tous les threads qui attendent la condition
VOID WakeConditionVariable(PCONDITION_VARIABLE ConditionVariable); // Réveille 1 thread qui attend 
```

On peut accomplir des choses similaires avec les events : 
```cpp 
// Un thread attend un événement 
wait_res = WaitForSingleObject(event_handle, INFINITE); 

// Jusqu'à ce qu'un autre thread appelle 
SetEvent(event_handle);
// L'event est alors "true" (signalé) jusqu'à ce qu'un thread appelle 
ResetEvent(event_handle); // Uniquement si l'argument ManualReset est True quand l'event a été créé

// Si l'event n'est pas en ManualReset (donc reset automatique) : un seul thread est libéré lorsqu'on appelle SetEvent

```