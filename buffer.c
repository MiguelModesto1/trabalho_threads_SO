#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define MAXITEMS 35
#define TOTALITEMS 20000

int numItems;

//tapete rolante

struct {
        pthread_mutex_t mutex;
        int *tapetecircular_buffer;
        int nextPut;
	int nextPutAux;
        int nextCon;
        int nextVal;
} shared = {PTHREAD_MUTEX_INITIALIZER};

//contador de colocados

//int counter = 0;

//tapete de saida

struct {
        pthread_mutex_t mutex;
        int *saida_buffer;
        int nextPut;
        int nextVal;
} consumerShared = {PTHREAD_MUTEX_INITIALIZER};

//estrutura de controlo de acoes (produzir/nao produzir; consumir/ nao consumir)

struct {
        pthread_mutex_t mutex;
        pthread_cond_t cond_prod;
	pthread_cond_t cond_con;
        int numReady;
} actCtrl = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};

//rotina de producao

void *produce(void* arg){

        printf("prod\n");

        while(shared.nextPutAux < TOTALITEMS){

                //printf("here1\n");

                pthread_mutex_lock(&shared.mutex);
                shared.tapetecircular_buffer[shared.nextPut] = shared.nextVal;
                printf("In %d: %d\n",shared.nextPut ,shared.tapetecircular_buffer[shared.nextPut]);
                *((int *) arg) += 1;
                if(shared.nextPut == MAXITEMS - 1){
                        shared.nextPut -= 34;
                }else if(shared.nextPut < MAXITEMS - 1){
                        shared.nextPut++;
                }
		shared.nextPutAux++;
                shared.nextVal++;

		/*if(shared.nextPutAux >= TOTALITEMS){
			for(int i = 0 ; i < 35; i++){
				shared.tapetecircular_buffer[i] = 20000;
			}
			pthread_mutex_unlock(&shared.mutex);
			return(NULL);
		}*/
                pthread_mutex_unlock(&shared.mutex);

                //printf("here2\n");

                pthread_mutex_lock(&actCtrl.mutex);
		if(actCtrl.numReady > 0){
			pthread_cond_signal(&actCtrl.cond_con);
		}
		while(actCtrl.numReady == MAXITEMS){
			pthread_cond_wait(&actCtrl.cond_prod, &actCtrl.mutex);
		}
                actCtrl.numReady++;
                //printf("numReady = %d\n", actCtrl.numReady);
                pthread_mutex_unlock(&actCtrl.mutex);

                //pthread_mutex_unlock(&shared.mutex);

                //printf("here3\n");
        }
	/*for(int i = 0 ; i < 35; i++){
		shared.tapetecircular_buffer[i] = 20000;
        }*/
	return(NULL);
}

//rotina de consumo

void *consume(void* arg){

        printf("con\n");

        while(consumerShared.nextPut < TOTALITEMS){
                //printf("here4\n");
                pthread_mutex_lock(&shared.mutex);
                consumerShared.saida_buffer[consumerShared.nextPut] = shared.tapetecircular_buffer[shared.nextCon] - 20000;               
		*((int *) arg) += 1;
		consumerShared.nextVal = consumerShared.saida_buffer[consumerShared.nextPut];
                printf("Out %d: %d\n", consumerShared.nextPut, consumerShared.nextVal);
		//pthread_mutex_unlock(&consumerShared.mutex);
                if(shared.nextCon == MAXITEMS - 1){
                        shared.nextCon -= 34;
                }else if(shared.nextCon < MAXITEMS - 1){
                        shared.nextCon++;
                }
                consumerShared.nextPut++;
                
		/*if(consumerShared.nextPut >= TOTALITEMS){
			pthread_mutex_unlock(&shared.mutex);
			return(NULL);
		}*/

		pthread_mutex_unlock(&shared.mutex);

                //printf("here5\n");

                pthread_mutex_lock(&actCtrl.mutex);
		//consumerShared.nextPut++;
		if(actCtrl.numReady < MAXITEMS){
			pthread_cond_signal(&actCtrl.cond_prod);
		}
                while(actCtrl.numReady == 0){
                        pthread_cond_wait(&actCtrl.cond_con, &actCtrl.mutex);
                }
                actCtrl.numReady--;
                //printf("numReady = %d\n", actCtrl.numReady);
                pthread_mutex_unlock(&actCtrl.mutex);

                //pthread_mutex_unlock(&shared.mutex);

                //printf("here6\n");
        }
	return(NULL);
}

int main(){

        //free(shared.tapetecircular_buffer);
        //free(consumerShared.saida_buffer);

        //declarar buffers de saida e de entrada dinamicamente

        shared.tapetecircular_buffer = calloc(MAXITEMS, sizeof(int));
        consumerShared.saida_buffer = calloc(TOTALITEMS, sizeof(int));

        //colocar standart output em estado unbuffered

        setbuf(stdout, NULL);

        //atribuir a 'shared.nextVal' 20001

        shared.nextVal = 20001;

	//iniciar 'numReady' a zero
	
	actCtrl.numReady = 0;

        //declarar iterador

        int i;

        //criar contadores de producoes e consumos

        int count[5] = {0,0,0,0,0};

        //declarar threads produtoras

        pthread_t PCOOK_1, PCOOK_2, PCOOK_3;

        //declarar threads consumidoras

        pthread_t COMILAO_1, COMILAO_2;

        //iniciar threads...

        //produtoras
        pthread_create(&PCOOK_1, NULL, produce, &count[0]);
        pthread_create(&PCOOK_2, NULL, produce, &count[1]);
        pthread_create(&PCOOK_3, NULL, produce, &count[2]);

        //consumidoras
        pthread_create(&COMILAO_1, NULL, consume, &count[3]);
        pthread_create(&COMILAO_2, NULL, consume, &count[4]);

        //esperar término de threads
        pthread_join(PCOOK_1, NULL);
        pthread_join(PCOOK_2, NULL);
        pthread_join(PCOOK_3, NULL);
        pthread_join(COMILAO_1, NULL);
        pthread_join(COMILAO_2, NULL);

        printf("PCOOK_1: %d\n", count[0]);
        printf("PCOOK_2: %d\n", count[1]);
        printf("PCOOK_3: %d\n", count[2]);
        printf("COMILAO_1: %d\n", count[3]);
        printf("COMILAO_2: %d\n", count[4]);

        free(shared.tapetecircular_buffer);
        free(consumerShared.saida_buffer);

        exit(0);
}




