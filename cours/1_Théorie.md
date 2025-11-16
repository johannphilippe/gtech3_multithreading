---
title: Théorie de la concurrence
author: Johann Philippe
---

# Théorie de la concurrence  

Le cerveau humain effectue plusieurs actions en même temps et toutefois, notre conscience / concentration saisit mieux les tâches séquentielles, c'est même une tendance naturelle : réduire les événements à une suite séquentielle. 
[60 years of mastering concurrent computing through sequential thinking](https://hal.science/hal-03162635v1/document)
Idem en informatique donc : plus simple de comprendre un programme purement séquentiel qu'un complexe parallèle ou concurrent. 

La concurrence naît d'un certain nombre de contraintes et nouveautés dans l'informatique à partir des années 60 et 70 : 
- Le dépassement du paradigme "single user interacting with a single sequential computer" 
- La nature blocante de certaines tâches 
- Le développement des communications réseau  
- Plus tard l'impossibilité de dépasser significativement les vitesses processeur, donnant naissance aux architectures multi-coeurs

Pionniers à avoir travaillé sur la concurrence :
- **Theodorus Dekker** - mathématicien hollandais, connu pour l'invention de l'algorithme de dekker
- **Edsger Dijkstra** - aussi connu pour son travail sur les algorithmes du plus court chemin *pathfinding* (i.e. [Algorithme de Dijkstra](https://fr.wikipedia.org/wiki/Algorithme_de_Dijkstra)).

Ils sont connus pour avoir travaillé sur le concept d'*exclusion mutuelle*, **mutual exclusion** (spoiler, on va en parler). 
[Dijkstra](https://lamport.azurewebsites.net/pubs/dijkstra.pdf)
Dans cet article, l'autrice décrit les problématiques de base de l'informatique conccurrente ainsi que les premières solutions apportées par les deux pionniers : un système de *mutual exclusion*. 
> For almost three years, this solution has been considered a "curiosity", until these issues at the beginning of 1962 suddenly became relevant for me again...
1959, Dekker

Le problème principal : le partage de la mémoire entre plusieurs processus (on les appelle comme ça à l'époque). 
C'est dans ces années qu'on va baptiser *critical section*, la partie du code de chaque processus qui travaille sur une mémoire partagée avec d'autres processus. 
Le problème principal est donc celui-ci : la synchronisation, ou comment s'assurer que les processus ne travaillent pas sur les mêmes ressources en même temps (ou du moins, que tous les processus sont hors de la *critical section* lorsqu'un processus souhaite y entrer). 
En effet, une lecture & écriture simultanée de deux processus entraîne des risques de corruption mémoire : impossible de s'assurer de l'état de la mémoire lorsque deux acteurs travaillent sur celle-ci. 
Les deux chercheurs développent donc un système que l'autrice du précédent article nomme *one-bit protocol*. Un système de *flag* (booléen) : 
- Tous les processus ont un flag qui est `true` par défaut
- Lorsqu'un processus souhaite entrer dans la critical section, il vérifie que les flags des autres processus sont `true`
- Si oui, il met son flag en `false`, puis `true` lorsqu'il sort de la critical section. 

Un problème survient rapidement : le *deadlock* (Article précédent sur Dijkstra page 5, *one bit algorithm*).
Le problème de l'exclusion mutuelle vu d'un point de vue théorique est complètement indépendant des systèmes : c'est un problème conceptuel de la concurrence.
En partant du principe que dans un contexte de concurrence : *on ne sait jamais dans quel état sont les autres processus, ni ce qu'ils font vraiment* :> Sont-ils en train de mettre leur flag en false en même temps qu'un autre processus ? 
Dans ce contexte, où deux processus verrouillent leur flag, ils vont tous deux attendre indéfiniment que l'autre se déverrouille : ils supposent chacun que l'autre est dans la critical section, alors qu'aucun des deux ne l'est. Dekker et Dijkstra proposent la solution : l'un d'entre eux doit déverrouiller, et attendre, libérant l'autre processus pour entrer dans la section critique. 
Cette solution résoud la situation du deadlock, mais ne prévient pas tous les risques du multiprocessing : il est théoriquement possible qu'un processus attende indéfiniment (car théoriquement possible qu'un autre processus entre dans la section critique pendant qu'il attend). 

L'algorithme suivant, de Dekker, page 8, propose une solution avec un système de priorité pour les systèmes à 2 processus, appelé *starvation freedom*, ou *liberté de famine* (?). 

Définition de Dijkstra de la preuve du *deadlock freedom* : 
> Suppose both processes are trying to enter the critical section. If
> neither succeeds, then the value of turn remains constant. It is
> then easy to see that process number *turn* must eventually wait
> with flag[turn] equal to `false` while process 1 − *turn* waits with
> *flag[1 − turn]* equal to `true`, allowing process *turn* to enter the
> critical section. If only a single process i is trying to enter, then
> process 1 − i must reach the noncritical section with *flag[1 − i]*
> equal to true, allowing process i to reach the critical section.

Dijkstra va plus loin pour un système permettant à un plus grand nombre de processus de *turn* : 
- Avec un booléen *idle* 
- Et avec un *turn* integer que chaque processus peut mettre sur son index si *turn* indique un numéro de process qui n'est pas en section critique. 

L'OS [THE operating system](https://en.wikipedia.org/wiki/THE_multiprogramming_system), prévu pour le [Electrologica X8](https://en.wikipedia.org/wiki/Electrologica_X8) voit donc l'introduction d'un paradigme de multiprogrammation : 
- *Sémaphores*
- 1 CPU + 1 CPU dédié périphériques 
- Un système par couches *Layer* où le Layer 0 assure l'ordonnancement des différents processus (scheduler). 

Important : ces considérations et algorithmes sont théoriques, et relativement simple à penser dans un système prédictible et linéaire. 
L'informatique contemporaine est à la fois bien plus complexe dans l'architecture hardware et système, mais aussi dotée de facilités de programmation.  

# L'informatique moderne

## Architecture processeur 

### CPU multi-coeurs

Depuis 2001 et l'IBM POWER4, les fabricants ont commencé à fabriquer des processeurs à plusieurs coeurs. 
Aujourd'hui, ce principe est si généralisé qu'on retrouve même des microcontrolleurs (type ESP32) disposant de plusieurs coeurs. 
Mais à l'époque, c'est nouveau, et cette nouveauté apparaît avec un certain nombre de challenges de programmation & construction :
- Le partage de cache 
- La gestion des interruptions  
- La nécessaire adaptation des OS pour prendre en charge ces multi coeurs 
- La réécriture des logiciels qui pourraient en tirer partie 

### Hardware thread 

Une confusion courante concerne le terme même de *thread*. On distingue : 
- *thread materiel* (hardware thread)
- *thread système* (system thread, ou software thread)

Et plein d'autres.
Le hardware thread est relatif aux architectures processeur.   

![intel_list](../assets/intel_list.png "Liste processeurs")

Il s'agit d'un contexte d'exécution physique au sein d'un coeur de processeur. 
Il permet l'exécution concurrente de plusieurs flux d'instructions. 
Sans rentrer dans le détail, il s'agit d'un mécanisme propre à chaque processeur (les threads hardware d'AMD sont différents de ceux d'Intel etc). Le point commun entre eux, c'est qu'ils apparaissent au système comme des coeurs à part entière. L'OS peut bien sûr aller chercher les informations de combien de coeurs physiques et combien de hardware thread. 
Si les mécanismes des hardware threads diffèrent selon les constructeurs, ils offrent l'avantage de pouvoir optimiser l'utilisation des coeurs en permettant une plus grande rentabilité de chaque coeur (en limitant les moments où le coeur est inoccupé). 

Ce que tout ça signifie, c'est que dans un système de multithreading de performance pure (par exemple un système où chaque thread exécute des instructions en permanence), il n'y a aucun intérêt à créer plus de threads logiciels que le nombre de threads matériels disponibles. 

Exemple connu : 
* La commande `make` (compilation makefile Linux) dispose de l'option `j` : `make -j8`. Elle permet de paralléliser la compilation des différents objets avant la phase de linkage final du compileur. Dans ce contexte, inutile de préciser un nombre de threads supérieur au nombre de threads matériels disponibles. 

#### Du côté OS 

Chaque système d'exploitation dispose d'un ordonnanceur (*scheduler*) qui gère le temps d'exécution accordé à chaque processus. Le *Moniteur de ressources* de Windows permet d'observer la charge de calcul de chaque thread matériel, le nombre de threads de chaque processus, et le nombre de threads matériels utilisés pour exécuter ces threads logiciels. 

Le Scheduler dispose d'une certain nombre d'informations pour chaque processus et chaque thread, notamment des polices de priorité, qui lui permettent d'évaluer quand lui donner du temps de calcul physique. 

Windows, MacOS et les grandes distributions Linux ne sont PAS des systèmes temps-réel : d'où la nécessité de ce scheduler. 
En effet, sur un microcontrolleur avec FreeRTOS, on peut prédire de manière sûre (en informatique) l'ordre d'exécution des tâches (threads), et avoir la certitude qu'elles seront exécutées dans un certain ordre. 
Dans un système *general purpose* comme les ordinateurs modernes, le système est prévu pour gérer un grand nombre de tâches simultanées : 
- Entrées sorties périphériques 
- Réseau & communication
- Un jeu vidéo 
- Le gourmand navigateur internet 
- Discord 
- etc etc 

Chacune de ces applications correspond à un certain nombre de threads logiciels. Le *scheduler* va affecter du temps de calcul à chaque thread en fonction de sa priorité, de ses droits, et des processeurs (thread matériel) disponibles. 
Il serait long de décrire dans le détail comment fonctionnent précisément ces ordonnanceurs, mais il est important de comprendre que même dans un contexte de programmation multithreading : on ne dispose jamais pleinement d'un coeur CPU ni même d'un thread matériel. 

## Quelques concepts clés 

- *Concurrence vs Parallélisme* 
  - La concurrence concerne le partage d'un temps de calcul **séquentiel** entre plusieurs tâches, moyennent un système d'interruption. Le concept de programmation le plus souvent utilisé pour la concurrence est la **coroutine**. On trouve aussi des systèmes concurrents avec les **light threads** utilisés par certains langages interprétés (Python, Ruby etc) en raison de la *Global Interpreter Lock* (un verrou qui assure la sécurité des threads en s'assurant qu'ils ne s'exécutent jamais en parallèle, permettant une sécurité mémoire notamment). On retrouve un fonctionnement similaire en JS et Dart avec les fonctionnalités *async* et *await*. 
  - Le parallélisme consiste à séparer des tâches sur différentes unités de calcul (threads) afin de permettre une exécution de plusieurs tâches **en même temps**. C'est du **multithreading**. Le thread logiciel est un concept à la définition assez largement partagée : tâche à laquelle le scheduler de l'OS va donner du temps de calcul sur un thread hardware qui est disponible. Par défaut, les threads partagent la mémoire. Certaines VM/compileurs implémentent des isolats ou *Isolates* qui sont de véritables threads parallèles mais qui ne partagent aucune mémoire, et peuvent communiquer entre-eux par messages (un message est une transmission de donnée qui implique une copie). C'est le cas de la VM d'Erlang ou du compileur Dart. 
- *Preemptive vs non-preemptive* threading / scheduling : 
  - Préemptive : l'ordonnancement préemptif signifie que le scheduler peut interrompre des tâches à tout moment pour en exécuter d'autres. C'est de cette manière que le scheduler système va pouvoir allouer du temps de calcul supplémentaire aux threads qui ont une haute priorité. 
  - Non-préemptif : c'est la tâche elle-même qui va *yield* (passer la main) le scheduleur pour permettre l'exécution d'une autre tâche. C'est le principe de fonctionnement classique des coroutines.

![concurrency_parallelism](../assets/concurrency_vs_parallelism.png)