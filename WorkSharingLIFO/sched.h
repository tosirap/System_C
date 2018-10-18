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
};


typedef struct Lifo Lifo;
struct Lifo{
	Tache* premier;
};


typedef struct scheduler scheduler;
struct scheduler{
	_Atomic int cpt; //compteur de thread actif
	int nbThread;
	int qlength;
	Lifo* pile;
};

void lifo_push( Lifo* p, Tache* tache);

Tache* lifo_pop(Lifo* p);

static inline int
sched_default_threads()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure);
int sched_spawn(taskfunc f, void *closure, struct scheduler *s);
