# Quelques généralités sur le multithreading hors API Windows

Il s'agit là d'éléments de culture générale. 

## Système temps-réel 

Dans un système d'exploitation temps-réel, il est possible de programmer en étant sûr de l'exécution  de ses différents threads & tâches, car absence de scheduler système (ou minimal) : le programmeur est le scheduler. 

### Exemple de FreeRTOS

[FreeRTOS](https://fr.wikipedia.org/wiki/FreeRTOS)

Dans FreeRTOS 

``` cpp
  xCreateTaskPinnedToCore(
    TaskFunction_t pxTaskCode, 
    const char * name,
    const uint32_t stack_size, 
    void * const params, 
    UBaseType_t priority, 
    StackType_t * const stack_buffer, 
    StaticTask_t * const task_buffer, 
    const BaseType_t code_id
  );
```

Quand on créé une tâche, on lui assigne un coeur et une priorité. Lorsque plusieurs tâches s'exécutent sur un même coeur, et dans le cas où elles auraient besoin de temps de calcul en même temps, la priorité s'applique strictement (nécessaire en temps-réel). Aucune autre tâche que celles programmées ne s'exécutent (OS léger), on peut ainsi garantir l'exécution en temps-réel, et opérer des vraies programmes time critical. 

## Le threading & la STL 

Pour la culture générale, car std::thread n'est pas autorisé pour ce projet.

[Concurrence in STL](https://en.cppreference.com/w/cpp/atomic.html)
[Extensions parallèles](https://en.cppreference.com/w/cpp/experimental/parallelism.html)

``` cpp
#include<thread>

void thread_task(int arg_a, double arg_2)
{

}

void thread_task2(Object &o)
{

}

// la fonction, puis les arguments, passbyvalue
std::thread t(thread_task, 12, 1.53);
// Same, pass by ref 
Object o;
std::thread t(thread_task2, std::ref(o));


// On lui passe une lambda 
std::thread t([&](){
    while(true)
    {
        // do something 
    }
});

// Avec une méthode
Object o;
std::thread t(&Object::thread_task, &o);

//On pour ensuite joindre le thread (c'est comme WaitForSingleObject)
t.join();

// Ou le détacher (on perd sa trace, il n'est plus lié à l'objet t)
t.detach();
```

Il existe aussi depuis C++ 20 la classe `std::jthread`, qui propose une fonctionnalité de `request_stop` pour automatiquement rejoindre un thread et demander son arrêt *safe*. 


### Les verrous 

``` cpp
// Le mutex (différent du mutex windows)
std::mutex mtx; 
mtx.lock();
mtx.try_lock();
mtx.unlock(); 

// Les verrous uniques 
std::unique_lock lock(mtx); // prend possession du mutex, la relâche lorsque l'objet est détruit 

// Conditions variables 
std::condition_variable cv; 
cv.wait(lock, []{return 0;});

// Do something 

lock.unlock();
cv.notify_one(); // Réveille une cv en attente
cv.notify_all();
```

#### Les atomiques 

``` cpp
#include<atomics>

// Atomic n'est pas lock free
std::atomic<bool> keep_running = true; 

// Any thread can do 
keep_running = false; 
// Ou 
keep_running.store(false); 

// Et on peu lire atomiquement : 
bool should_i_keep_running = keep_running.load();

// Dispose aussi de fonctionnalités wait, notify_one, notify_all.


// atomic_flag est lock_free
std::atomic_flag f{};
// Tente d'obtenir le verrou, renvoie la valeur précédente 
bool prev_val = f.test_and_set(std::memory_order_acquire);
bool prev = f.test();

f.wait();
f.notify_one();
f.notify_all();
```