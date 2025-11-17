# Thread Safety 

Le problème principal du multithreading est la gestion de la mémoire partagée.
En effet, on souhaite éviter des data racen (la corruption mémoire), qui peut se produire lorsqu'un thread écrit sur une zone mémoire lue par un autre thread simultanément (segmentation fault, ou corruption). 

Comme nous l'avons vu dans la partie théorique, il existe plusieurs mécanismes permettant de sécuriser l'accès à des ressources partagées. 
Ces mécanismes diffèrent de ceux étudiés par Dijkstra, bien qu'on utilise encore la Sémaphore. 

Liste des verrous connus : 
- Opérations atomiques 
  - Pour WIN32, les Interlocked correspondent à ces opérations 
- Mutex (mutual exclusive)
  - Algorithme de Dekker
  - Algorithme de Peterson 
  - Implémentation WIN32 fonction aussi entre processus 
- Critical Section 
  - Implémentation WIN32 : Similaire au Mutex, mais uniquement au sein d'un processus 
  - Très rapide et sûr 
  - Basé sur un spinlock
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

