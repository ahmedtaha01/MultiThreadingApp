# MultiThreading App
## this app is for learning purposes, it shows the difference between 4 search techniques.
* linear technique.
* parallel, parent thread wait for children.
* parallel, parent thread check on children.
* parallel, parent wait for semaphore.

### the app takes 3 arguments to run:
1. array size
2. number of threads
3. index of zero (will know why soon)
```console
./main 12 4 3
```
note that i am using linux.
## lets talk about technicall overview of this app:
Here, we are initializing the array that is searchable, initializing it with positive numbers only
```C
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
```
and we are initializing the indices array here :
```C
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
```
so what is the purpose of indices array, we will see this by a simple image:
![indices array](https://github.com/ahmedtaha01/MultiThreadingApp/assets/98897680/f4ba123d-cd0e-44e9-b614-75ec7fb15f3c)

we will send each row to the corresponding thread.
# linear technique:
this code is responsible for searching accross the array
```C
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
```
and here we are making a thread for the linear search to run 
```C
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
```
* we are estimating the time of the searching using the ``` clock()```
* ``` pthread_join() ``` makes the parent thread waits for his children -linear search- untill they finish.

# parallel, parent waits for children
here is the code for searching, normal one
``` C
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
```
here we are creating # threads -sent in arguments- and passing to the i thread the i indices row, corresponding one
``` C
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
```
# parallel, parent check on children:
here the parent will check on the children if they finished or not using some flags.
the first function is the parent that check on the children using 2 flags ``` ZERO_SIGNAL```, ``` ALL_COMPLETE```
* ``` ZERO_SIGNAL```: if the zero number is found, the thread should exit as there is no smaller number.
* ``` ALL_COMPLETE ```: if all the threads completed,then the thread should exit.
```C
void *parentCheckOnChildren(){
    while (1)
    {
        if(ZERO_SIGNAL == 1){
            printf("Zero found \n");
            pthread_exit(0);
        } else if(ALL_COMPLETE == NUM_THREADS){
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

```
and the code for making threads
```C
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
                pthread_cancel(workers[i]);
            }
            
            T = clock() - T;
            time_taken = ((double)T)/CLOCKS_PER_SEC; // in seconds
            printf("For the parallel search and parent check on children : \n");
            printf("the smallest number is %i and time : %f \n",MIN,time_taken);
```
# parallel, parent wait for symaphore
```C
void *parallelSearchSYM(void *param){
    int *ptr = (int *) param;
    
    for(int i =ptr[1]; i < ptr[2];i++){
        if(SEMAPHORE_STATE == 1){
            pthread_exit(0);
        }
        if(ARR[i] == 0){

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
    if(ALL_COMPLETE == NUM_THREADS){
        SEMAPHORE_STATE = 1;
        sem_post(&SEM);
    }
    pthread_exit(0);
}
```
thread creation
```C
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
```
the semaphore takes a long time, i will be optimizing it soon
## some of the outputs
```console
./main 120000 4 90001
```
```
welcome to multithreading search application 
Initializing the array with random numbers... 
Array initialized successfully. 
For the sequential search : 
the smallest number is 0 and time : 0.000655 
For the parallel search : 
the smallest number is 0 and time : 0.000788 
Zero found 
For the parallel search and parent check on children : 
the smallest number is 0 and time : 0.000719 
For the parallel search and using symaphores : 
the smallest number is 0 and time : 0.225493 
```
### the output may vary from machine to another
### i am using 4 threads as maximum as i have 4 cores, if i used more, it would be concurrent rather than parallel
### there are also some other overheads like context switching and other threads taking the core for itself etc...
### the use of third argument is to show the speed of parallel programming as if the zero is at 90001, the linear search will take 90000 iteration to find it
### but with multi threading, the third thread will find it after 1 or 2 iterations
