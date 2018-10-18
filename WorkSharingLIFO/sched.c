#include "sched.h"
#include <pthread.h>

pthread_mutex_t mutexTopPile = PTHREAD_MUTEX_INITIALIZER;
int nbAction=0;

int isEmpty(Lifo* pile){
	if(pile->premier ==NULL)
		return 1;
	return 0;
}

Tache* lifo_pop(Lifo* pile){
	//pthread_mutex_lock(&mutexTopPile);
	if(pile->premier==NULL || pile==NULL){
		return NULL;
	}
	Tache* tacheTmp =pile->premier;
	pile->premier=pile->premier->suivant;
	return tacheTmp;
	//pthread_mutex_unlock (&mutexTopPile);
}

void lifo_push(Lifo* pile, Tache* tache){
	pthread_mutex_lock(&mutexTopPile);
	if(pile==NULL){
		exit(EXIT_FAILURE);
	}
	tache->suivant = pile->premier;
	pile->premier=tache;
	pthread_mutex_unlock (&mutexTopPile);
}

void toutDetruire(scheduler *sc, pthread_t* tabThread){
	int i;
	free(tabThread);
	free(sc->pile);
	free(sc);
}

void* maFonction(void * ptr){
	scheduler *sc = (scheduler*)ptr;
	//printf("start \n");
	while(1){
		pthread_mutex_lock(&mutexTopPile);
		Tache* tache =lifo_pop(sc->pile); //element top de la pile
		pthread_mutex_unlock(&mutexTopPile);

		if(tache != NULL){
			sc->cpt++;
			taskfunc f = tache->tf;
			(*f)(tache->closure,sc);
			free(tache);
			sc->cpt--;
		}

		if(sc->cpt == 0 && isEmpty(sc->pile)){ //si pile vide et tout les thread inactif => pu rien a faire
			break;
		}
	}
	//printf("end\n");

	//return;
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure){
	
	scheduler *sc = malloc(sizeof(*sc));
	sc->pile = malloc(sizeof(sc->pile));
	sc->cpt = 0;

	if(nthreads<=0){
		sc->nbThread=sched_default_threads();
		//printf("sched defaut val %d\n", sched_default_threads());
	}	
	else{
		sc->nbThread=nthreads;	
	} 
	sc->qlength=qlen;
	Tache* tache = malloc(sizeof(*tache));
	tache->tf=f;
	tache->closure=closure;
	tache->suivant=NULL;

	sc->pile->premier = tache;

	pthread_t* tabThread =malloc(sizeof(pthread_t)*sc->nbThread); //  [sc->nbThread];
	int i;
	for(i=0; i<sc->nbThread; i++){
		pthread_create(&tabThread[i],NULL,maFonction,sc );
		
	}
	int j;
	for(j=0;j<sc->nbThread;j++){
		pthread_join(tabThread[j],NULL);
	}
	toutDetruire(sc,tabThread);

	return 1;
}


int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
	//write(1,"testSpawn\n",10);
	Tache* tache = malloc(sizeof(*tache));
	tache->tf=f;
	tache->closure=closure;
	tache->suivant=NULL;
	//si nb tache >= capacite ordonnanceur, potentiellement erreur
	lifo_push(s->pile, tache);
	return 1;
}





