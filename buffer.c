#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define MAXITEMS 35		// maximo de items para o tapete circular
#define TOTALITEMS 20000	// numero total de elementos a processar pelas threads

int numItems;

//estrutura do tapete rolante

struct {
        pthread_mutex_t mutex; // mutex para acesso exclusivo aos 2 buffers
        int *tapetecircular_buffer; //ponteiro para o array do tapete circular
        int nextPut; // indice do proximo local para escrever valores
	int nextPutAux; // valor de auxilio para o 'while' da rotina de producao
        int nextCon; // indice do proximo local para ler valores
        int nextVal; // proximo valor a colocar no tapete circular
} shared = {PTHREAD_MUTEX_INITIALIZER};

//estrutura tapete de saida

struct {
        int *saida_buffer; // ponteiro para o array do tapete de saida
        int nextPut; // indice do proximo local para escrever no tapete de saida
        int nextVal; // proximo valor a colocar no tapete de saida
} consumerShared = {};

//estrutura de controlo de acoes (produzir/nao produzir; consumir/ nao consumir)

struct {
        pthread_mutex_t mutex; // mutex para acesso exclusivo aos recursos de controlo
        pthread_cond_t cond_prod; // variavel de condicao de producao
	pthread_cond_t cond_con; // variavel de condicao de consumo
        int numReady; // numero de pratos prontos
} actCtrl = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};

//rotina de producao

void *produce(void* arg){
	while(shared.nextPutAux < TOTALITEMS){ // iterar ate o indice do ultimo valor ser 20000
                pthread_mutex_lock(&shared.mutex); // bloquear acesso aos buffers
                shared.tapetecircular_buffer[shared.nextPut] = shared.nextVal; //realizar operação de escrits
                *((int *) arg) += 1; // contagem individual de operacoes realizadas
                if(shared.nextPut == MAXITEMS - 1){ // verificacao de indice de producao do array
                        shared.nextPut -= 34; // se chegou ao limite, volta ao indice 0
                }else if(shared.nextPut < MAXITEMS - 1){
                        shared.nextPut++; // senao incrementa em 1;
                }
		shared.nextPutAux++; // incremento da variavel de auxilio ao 'while'
                shared.nextVal++; // incremento do valor do proximo prato a colocar
		pthread_mutex_unlock(&shared.mutex); // desbloquear acesso aos buffers

                pthread_mutex_lock(&actCtrl.mutex); // bloquear acesso aos recursos de controlo
		while(actCtrl.numReady == MAXITEMS){ // enquanto o tapete circular estiver cheio
			pthread_cond_wait(&actCtrl.cond_prod, &actCtrl.mutex); // esperar condicao de producao
		}
		pthread_cond_signal(&actCtrl.cond_con); // sinalizar condicao de consumo
                actCtrl.numReady++; // incremento de numero de pratos prontos
                pthread_mutex_unlock(&actCtrl.mutex); // desbloquear acesso aos recursos de controlo
        }
	return(NULL);
}

//rotina de consumo

void *consume(void* arg){

        while(consumerShared.nextPut < TOTALITEMS){ // iterar ate o indice do ultimo valor ser 20000
                pthread_mutex_lock(&shared.mutex); // bloquear acesso aos buffers
		consumerShared.saida_buffer[consumerShared.nextPut] = shared.tapetecircular_buffer[shared.nextCon] - 20000; // realizar operacao de consumo e colocar no tapete de saida
		*((int *) arg) += 1; // contagem individual de operacoes realizadas
		consumerShared.nextVal = consumerShared.saida_buffer[consumerShared.nextPut]; // valor a imprimir no ecra
                printf("%d ", consumerShared.nextVal); // imprimir 'nextVal' no ecra
		if(shared.nextCon == MAXITEMS - 1){ // verificacao do indice de consumo do array
                        shared.nextCon -= 34; // se chegou ao limite, volta ao indice 0
                }else if(shared.nextCon < MAXITEMS - 1){
                        shared.nextCon++; // senao incrementa em 1
                }
                consumerShared.nextPut++; // incremento do indice do proximo valor para o tapete de saida

		pthread_mutex_unlock(&shared.mutex); // desbloquear acesso aos buffers

                pthread_mutex_lock(&actCtrl.mutex); // bloquear acesso os recursos de controlo
		while(actCtrl.numReady == 0) // enquanto o numero de pratos for 0
                        pthread_cond_wait(&actCtrl.cond_con, &actCtrl.mutex); // esprar pela condicao de consumo
		pthread_cond_signal(&actCtrl.cond_prod); // sinzalizar condicao de producao
		actCtrl.numReady--; // decrementar numero de pratos prontos
                pthread_mutex_unlock(&actCtrl.mutex); // desbloquear acesso aos recursos de controlo
        }
	return(NULL);
}

int main(){

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

	//atributos thread
	
	pthread_attr_t tattr;
	int ret = pthread_attr_init(&tattr);

        //declarar threads produtoras

        pthread_t PCOOK_1, PCOOK_2, PCOOK_3;

        //declarar threads consumidoras

        pthread_t COMILAO_1, COMILAO_2;

        //iniciar threads...

        //produtoras
        pthread_create(&PCOOK_1, &tattr, produce, &count[0]);
        pthread_create(&PCOOK_2, &tattr, produce, &count[1]);
        pthread_create(&PCOOK_3, &tattr, produce, &count[2]);
        
	//consumidoras
	pthread_create(&COMILAO_1, &tattr, consume, &count[3]);
	pthread_create(&COMILAO_2, &tattr, consume, &count[4]);

        //esperar termino de threads
        pthread_join(PCOOK_1, NULL);
        pthread_join(PCOOK_2, NULL);
        pthread_join(PCOOK_3, NULL);
        pthread_join(COMILAO_1, NULL);
        pthread_join(COMILAO_2, NULL);

	//escrever contagens de operacoes individuais no ecra
        printf("\nPCOOK_1 cozinhou %d pratos\n", count[0]);
        printf("PCOOK_2 cozinhou %d pratos\n", count[1]);
        printf("PCOOK_3 cozinhou %d pratos\n", count[2]);
        printf("COMILAO_1 comeu %d pratos\n", count[3]);
        printf("COMILAO_2 comeu %d pratos\n", count[4]);

	//libertar memoria alocada dinamicamente para os 2 buffers
        free(shared.tapetecircular_buffer);
        free(consumerShared.saida_buffer);

	//sair com sucesso
        exit(0);
}
