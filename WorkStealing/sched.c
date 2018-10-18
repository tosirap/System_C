#include "sched.h"
#include <pthread.h>


//static pthread_mutex_t mutexQuelDeque = PTHREAD_MUTEX_INITIALIZER;
static double timeSleep = 0.1;
static int nbTacheTotal = 1;
static _Atomic int nbThreadArete =0;

/*int isEmpty(Deque* deque){
	if(deque->premier ==NULL)
		return 1;
	return 0;
}*/

void deque_push(Deque* d, Tache* tache){//seems ok
	if(d!= NULL){
		tache->precedent = NULL;
		tache->suivant = d->premier;
		
		if(d->premier != NULL){
			d->premier->precedent =tache;	
		}
		if(d->dernier==NULL){
			d->dernier = tache;
		}
		d->premier = tache;
		//printf("tache->fonc: %p, tache closure : %p \n",tache->tf,tache->closure );
	}
}

Tache* deque_pop_proprietaire(Deque* deque){ //seems ok
	if(deque==NULL || deque->premier==NULL){
		return NULL;
	}
	Tache* tacheTmp =deque->premier;
	deque->premier=deque->premier->suivant;
	if(deque->premier!=NULL){
		deque->premier->precedent=NULL;
	}else{
		deque->dernier=NULL;
	}


	return tacheTmp;
}

Tache* deque_pop_voleur(Deque* d){
	if(d==NULL || d->dernier==NULL){
		return NULL;
	}
	Tache* tacheTmp = d->dernier;
	d->dernier = d->dernier->precedent;
	if(d->dernier!=NULL){
		d->dernier->suivant = NULL;
	}
	else{
		d->premier=NULL;
	}
	return tacheTmp;
}

int toutFini(scheduler *sc){
	int nbDequeVide=0;
	if(sc->cpt>0){
		return 0;
	}
	int i;
	if(sc->deque == NULL){
		return 0;
	}
	if(sc->deque!=NULL){
		for(i=0;i<(sc->nbThread);i++){
			pthread_mutex_lock(&sc->deque[i]->mutexDeque);
			if(sc->deque!=NULL && sc->deque[i]!=NULL){
				if(sc->deque[i]->premier ==NULL){
					nbDequeVide++;
				}
			}
		}
		int j;
		for(j=0;j<sc->nbThread;j++){
			pthread_mutex_unlock(&sc->deque[j]->mutexDeque);
		}
	}
	if(nbDequeVide==sc->nbThread)
		return 1;
	return 0;
}

void toutDetruire(scheduler *sc){
	int i;
	for(i=0;i<sc->nbThread;i++){
		//free(sc->deque[i]->numThread);
		free(sc->deque[i]);
	}
	free(sc->deque);
	free(sc);
}

void* maFonction(void * ptr){
	scheduler *sc = (scheduler*)ptr;
	Deque *mydeque= malloc(sizeof(Deque));
	//on cherche la bonne deque:  
	int k;
	for(k=0;k<sc->nbThread;k++){
		if(sc->deque[k]!=NULL){
			if(pthread_self()==sc->deque[k]->numThread){
				mydeque= sc->deque[k];
				break;
			}
		}
	}
	while(1){
		
		pthread_mutex_lock(&(mydeque->mutexDeque));//& mutexBotDeque[nb]);;
		Tache* tache = deque_pop_proprietaire(mydeque);
		pthread_mutex_unlock(&(mydeque->mutexDeque));
		if(tache !=NULL && tache->tf!=NULL && tache->closure !=NULL){
			sc->tacheTotal--;
			sc->cpt++;
			taskfunc f = tache->tf;
			(*f)(tache->closure,sc);
			sc->cpt--;
			free(tache);
				//printf("nb de thread actif %d, nb de tache dans pile: %d \n",sc->cpt, sc->tacheTotal);
		}
	
		else if(tache == NULL){//tester pour voler ailleur //parti stealing
			int j;
			//printf("test\n");
			for(j=0;j<sc->nbThread;j++){
				
					pthread_mutex_lock( &(sc->deque[j]->mutexDeque));
					Tache *tache = deque_pop_voleur(sc->deque[j]);
					pthread_mutex_unlock(&(sc->deque[j]->mutexDeque));
					//printf("tache : %p\n", tache);
					if(tache != NULL && tache->tf!=NULL && tache->closure !=NULL){
						sc->tacheTotal--;
						sc->cpt++;
						taskfunc f = tache->tf;
						(*f)(tache->closure,sc);
						sc->cpt--;
						free(tache);
						break;
					}
			}
			sleep(timeSleep);
			if(toutFini(sc)==1){
				nbThreadArete ++;
				break;
			}
			
		}	
	}
}



int sched_init(int nthreads, int qlen, taskfunc f, void *closure){
	scheduler *sc = malloc(sizeof(*sc));
	sc->qlength=qlen;
	sc->tacheTotal=1;
	if(nthreads<=0){
		sc->nbThread=sched_default_threads();
	}	
	else{
		sc->nbThread=nthreads;	
	}
	sc->deque = malloc(sizeof(Deque)*sc->nbThread);//taille de la liste de deque
	int k;
	for(k=0;k<sc->nbThread;k++){//on cré une deque par thread
		sc->deque[k] = malloc(sizeof(Deque));
		sc->deque[k]->premier = malloc(sizeof(Tache));
		sc->deque[k]->dernier = malloc(sizeof(Tache));
		pthread_mutex_init(&sc->deque[k]->mutexDeque, NULL);
	}
	Tache* tache = malloc(sizeof(*tache));
	tache->tf=f;
	tache->closure=closure;
	tache->suivant=NULL;
	tache->precedent=NULL;

	sc->deque[0]->premier = tache;//on met la tache dans la deque n°0
	int i;
	for(i=0; i<sc->nbThread; i++){
		pthread_create(&sc->deque[i]->numThread,NULL,maFonction,sc );
	}
	int j;
	for(j=0;j<sc->nbThread;j++){
		pthread_join((sc->deque[j]->numThread),NULL);
	}
	toutDetruire(sc);
	return 1;

}




int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
	Tache* tache = malloc(sizeof(*tache));
	tache->tf=f;
	tache->closure=closure;
	//tache->suivant=NULL;
	Deque *mydeque= malloc(sizeof(Deque));
	//on cherche la bonne deque:  
	int k;
	for(k=0;k<s->nbThread;k++){
		if(pthread_self()==s->deque[k]->numThread){
			mydeque= s->deque[k];
			break;
		}
	}
	s->tacheTotal++;
	pthread_mutex_lock(&(mydeque->mutexDeque));
	deque_push(mydeque, tache);
	pthread_mutex_unlock(&(mydeque->mutexDeque));
	return 1;
}