#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>
#include <semaphore.h>

int ARR_SIZE;
int NUM_THREADS;
int *ARR;
int MIN = INT_MAX;
clock_t T;
// for parallel and parent check on children
int ZERO_SIGNAL = 0;
int ALL_COMPLETE = 0;
// for parallel and symaphore
sem_t SEM;
int SEMAPHORE_STATE;


void exitProgram(int arg);
void checkValidity();
void initializeArray(int index_of_zero);
int ** initializeIndicesArray();
void *sequentialSearch(void *param);
void restartData();
void *parallelSearch(void *param);
void *parentCheckOnChildren();
void *parallelSearchPCOC(void *param);
void *parallelSearchSYM(void *param);


int main(int argc,char ** argv){
    printf("welcome to multithreading search application \n");
    
    exitProgram(argc);
    ARR_SIZE = atoi(argv[1]);
    NUM_THREADS = atoi(argv[2]);
    int index_of_zero = atoi(argv[3]);
    // ARR_SIZE = 12;
    // NUM_THREADS = 4;
    // int index_of_zero = 3;
    checkValidity();
    initializeArray(index_of_zero);
    int ** indices_array = initializeIndicesArray();
    double time_taken;
    pthread_t workers[NUM_THREADS];
    sem_init(&SEM,0,0);

    

    // sequential
            pthread_t tid_seq;
            pthread_attr_t attr_seq;
            pthread_attr_init(&attr_seq);
            
            T = clock();
            pthread_create(&tid_seq,&attr_seq,sequentialSearch,&ARR_SIZE);
            pthread_join(tid_seq,NULL);
            T = clock() - T;
            time_taken = ((double)T)/CLOCKS_PER_SEC; // in seconds
            printf("For the sequential search : \n");
            printf("the smallest number is %i and time : %f \n",MIN,time_taken);

    //////////////////////////////////////////////////////////////////////////
    restartData();
    //////////////////////////////////////////////////////////////////////////
    // parallel, parent wait for all children
            
            T = clock();
            for(int i =0; i < NUM_THREADS;i++){
                pthread_t tid;
                workers[i] = tid;
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_create(&workers[i],&attr,parallelSearch,indices_array[i]);
            }

            for(int i =0; i < NUM_THREADS;i++){
                pthread_join(workers[i],NULL);
            }
            T = clock() - T;
            time_taken = ((double)T)/CLOCKS_PER_SEC; // in seconds
            printf("For the parallel search : \n");
            printf("the smallest number is %i and time : %f \n",MIN,time_taken);
    /////////////////////////////////////////////////////////////////////////////
    restartData();
    /////////////////////////////////////////////////////////////////////////////
    // parallel, parent check on children
    // parent thread
            pthread_t tid;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&tid,&attr,parentCheckOnChildren,NULL);
   
            
            T = clock();
    // children threads        
            for(int i =0; i < NUM_THREADS;i++){
                pthread_t tid;
                workers[i] = tid;
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_create(&workers[i],&attr,parallelSearchPCOC,indices_array[i]);
            }
            pthread_join(tid,NULL);
            for(int i =0; i < NUM_THREADS;i++){
                pthread_join(workers[i],NULL);
            }
            
            T = clock() - T;
            time_taken = ((double)T)/CLOCKS_PER_SEC; // in seconds
            printf("For the parallel search and parent check on children : \n");
            printf("the smallest number is %i and time : %f \n",MIN,time_taken);
    ///////////////////////////////////////////////////////////////////////////////
    restartData();
    ///////////////////////////////////////////////////////////////////////////////
    // parallel using symaphores
    
            for(int i =0; i < NUM_THREADS;i++){
                pthread_t tid;
                workers[i] = tid;
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_create(&workers[i],&attr,parallelSearchSYM,indices_array[i]);
            }
            
            // no join use symaphores
            sem_wait(&SEM);
            for(int i =0; i < NUM_THREADS;i++){
                pthread_cancel(workers[i]);
            }
            
            T = clock() - T;
            time_taken = ((double)T)/CLOCKS_PER_SEC; // in seconds
            printf("For the parallel search and using symaphores : \n");
            printf("the smallest number is %i and time : %f \n",MIN,time_taken);

}

void restartData(){
    MIN = INT_MAX;
    ZERO_SIGNAL = 0;
    ALL_COMPLETE = 0;
    T = 0;
    sem_init(&SEM,0,0);
    SEMAPHORE_STATE = 0;
}

void exitProgram(int arg){
    if(arg != 4){
        printf("you should have provided 3 arguments \n");
        printf("1 - array size \n");

        printf("2 - number of threads \n");

        printf("3 - index of zero, -1 for none \n");

        exit(1);
    }
}

void checkValidity(){
    if(ARR_SIZE % NUM_THREADS != 0 || ARR_SIZE < NUM_THREADS){
        printf("Array size should be divisible by number of threads for initializing array correctly  \n");
        exit(1);
    }
}

void initializeArray(int index_of_zero){
    printf("Initializing the array with random numbers... \n");
    ARR = malloc(sizeof (int) * ARR_SIZE);
    for(int i =0; i < ARR_SIZE;i++){
        srand(time(NULL));
        ARR[i] = rand() % 50000;
    }
    if(index_of_zero >=0 && index_of_zero < ARR_SIZE){
        ARR[index_of_zero] = 0;
    } else {
        printf("Couldn't put zero as index is not valid \n");
    }
    printf("Array initialized successfully. \n");
    
}

int **initializeIndicesArray(){
    
    int **indices_array = (int **)malloc(NUM_THREADS * sizeof (int *));   //allocate memory block for the indices array
    
    for (int i = 0; i < NUM_THREADS; i++) {             //allocate memory for each row
        indices_array[i] = (int *)malloc(3 * sizeof(int));    // 3 -> thread_num , begin , final
        if (indices_array[i] == NULL) {
            perror("Failed to allocate memory for columns");
            exit(1);
        }
    }
    
    for(int i = 0; i < NUM_THREADS;i++){
        for(int j = 0; j < 3; j++){
            if(j == 0){
                indices_array[i][j] = i;
            }else if(j == 1){
                indices_array[i][j] = (ARR_SIZE / NUM_THREADS) * i;
            } else {
                indices_array[i][j] = (ARR_SIZE / NUM_THREADS) * (i + 1);
            }
        }
    }
    return indices_array;
}


void *sequentialSearch(void *param){
    int *ptr = (int *) param;
    int size = *ptr;
    for(int i =0; i < size; i++){
        if(ARR[i] == 0){
            MIN =0;
            pthread_exit(0);
        }
        if(ARR[i] < MIN){
            MIN = ARR[i];
        }
    }
}

void * parallelSearch(void *param){
    int *ptr = (int *) param;
    
    for(int i =ptr[1]; i < ptr[2];i++){
        if(ARR[i] == 0){
            MIN = 0;
            pthread_exit(0);
        }
        if(ARR[i] < MIN){
            MIN = ARR[i];
        }
    }
}

void *parentCheckOnChildren(){
    while (1)
    {
        if(ZERO_SIGNAL == 1){
            printf("Zero found \n");
            pthread_exit(0);
        } else if(ALL_COMPLETE == 4){
            printf("All Complete \n");
            pthread_exit(0);
        }
    }
    
}
void *parallelSearchPCOC(void *param){
    int *ptr = (int *) param;
    
    for(int i =ptr[1]; i < ptr[2];i++){
        if(ARR[i] == 0){
            MIN = 0;
            ZERO_SIGNAL = 1;
            pthread_exit(0);
        }
        if(ARR[i] < MIN){
            MIN = ARR[i];
        }
        if(ZERO_SIGNAL == 1){
            pthread_exit(0);
        }
    }
    ALL_COMPLETE += 1;
    
}

void *parallelSearchSYM(void *param){
    int *ptr = (int *) param;
    printf("%i",ptr[1]);
    for(int i =ptr[1]; i < ptr[2];i++){
        if(SEMAPHORE_STATE == 1){
            sem_post(&SEM);
        }
        if(ARR[i] == 0){
            if(i == 3){
                printf("here \n");
            }
            MIN = 0;
            SEMAPHORE_STATE = 1;
            sem_post(&SEM);
            pthread_exit(0);
            
        }
        if(ARR[i] < MIN){
            MIN = ARR[i];
        }
        
    }
    ALL_COMPLETE += 1;
    if(ALL_COMPLETE == 4){
        SEMAPHORE_STATE = 1;
        sem_post(&SEM);
    }
}
