#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct scheduler;
typedef void (*taskfunc)(void*, struct scheduler *);

typedef struct Tache Tache;
struct Tache{
	taskfunc tf;
	void* closure;
	Tache* suivant;
	Tache* precedent;
};

typedef struct Deque Deque;
struct Deque{
	Tache* premier;
	Tache* dernier;
	//pthread_mutex_t mutexBotDeque;
	//pthread_mutex_t mutexTopDeque;
	pthread_t numThread;
	pthread_mutex_t mutexDeque;
};

typedef struct scheduler scheduler;
struct scheduler{
	_Atomic int cpt; //compteur de thread actif
	_Atomic int tacheTotal;
	int nbThread;
	int qlength;
	Deque** deque;//liste de deque
	//pthread_t** tabThread; //tab d'id de thread
	//pthread_mutex_t []mutexBotDeque;
	//pthread_mutex_t []mutexTopDeque;

};

void deque_push(Deque* d, Tache* tache);
Tache* deque_pop_proprietaire(Deque* d);
Tache* deque_pop_voleur(Deque* d);
static inline int
sched_default_threads()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure);
int sched_spawn(taskfunc f, void *closure, struct scheduler *s);
