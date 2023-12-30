#include <stdio.h>
#include <pthread.h>

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t myCond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t myCond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t startCond = PTHREAD_COND_INITIALIZER;
int myCount = 0;
int consumer_started = 0;

void* producer(void* arg) {
    while(1) {
        pthread_mutex_lock(&myMutex);
        
        while (!consumer_started) {
            printf("PRODUCER: waiting for consumer to start\n");
            pthread_cond_wait(&startCond, &myMutex);
        }
        
        if(myCount < 10) {
            printf("PRODUCER: myMutex locked\n");
            printf("myCount: %d -> %d\n", myCount, myCount + 1);
            myCount++;
            printf("PRODUCER: myMutex unlocked\n");
        
            pthread_cond_signal(&myCond2);
            printf("PRODUCER: waiting on myCond1\n");
            pthread_cond_wait(&myCond1, &myMutex);
        }
        
        if(myCount >= 10) {
            pthread_cond_signal(&myCond2); // Signal to consumer that producer is done
            pthread_mutex_unlock(&myMutex);
            return NULL;
        }

        pthread_mutex_unlock(&myMutex);
    }
}

void* consumer(void* arg) {
    printf("CONSUMER THREAD CREATED\n");
    
    pthread_mutex_lock(&myMutex);
    consumer_started = 1;
    pthread_cond_signal(&startCond);
    pthread_mutex_unlock(&myMutex);
    
    while(1) {
        pthread_mutex_lock(&myMutex);
        
        if(myCount < 10) {
            printf("CONSUMER: myMutex locked\n");
            printf("CONSUMER: myMutex unlocked\n");
        
            pthread_cond_signal(&myCond1);
            printf("CONSUMER: waiting on myCond2\n");
            pthread_cond_wait(&myCond2, &myMutex);
        }
        
        if(myCount >= 10) {
            pthread_cond_signal(&myCond1); // Signal to producer that consumer is done
            pthread_mutex_unlock(&myMutex);
            return NULL;
        }

        pthread_mutex_unlock(&myMutex);
    }
}




int main() {
    pthread_t producer_thread, consumer_thread;
    
    printf("PROGRAM START\n");
    
    pthread_create(&producer_thread, NULL, &producer, NULL);
    pthread_create(&consumer_thread, NULL, &consumer, NULL);
    
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
    
    printf("PROGRAM END\n");
    
    return 0;
}


