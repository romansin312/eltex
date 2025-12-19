#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#define SHOPS_NUM 5
#define CONSUMERS_NUM 3
#define PROCUER_AMOUNT 5000
#define MIN_SHOP_VAL 9000
#define MAX_SHOP_VAL 11000
#define MIN_NEED 95000
#define MAX_NEED 105000

int shops[SHOPS_NUM];
int consumers[CONSUMERS_NUM];

pthread_mutex_t shop_mutexes[SHOPS_NUM];
int running = 1;

void *consumer_handler(void *consumer_index_ptr) {
    int consumer_index = *(int *)consumer_index_ptr;

    while (consumers[consumer_index] > 0 && running) {
        for (int i = 0; i < SHOPS_NUM; i++) {
            int need = consumers[consumer_index];
            if(shops[i] > 0 && pthread_mutex_trylock(&shop_mutexes[i]) == 0) {
                int shop_val = shops[i];
                int new_need, take_val;
                if (need >= shop_val) {
                    new_need = need - shop_val;
                    take_val = shop_val;
                } else {
                    new_need = 0;
                    take_val = need;
                }
                    
                printf("Consumer %d: need %d, shop #%d, take %d, new need %d\n", consumer_index, need, i, take_val, new_need);

                consumers[consumer_index] = new_need;
                shops[i] = shop_val - take_val;

                pthread_mutex_unlock(&shop_mutexes[i]);

                if (new_need == 0) {
                    printf("Need of consumer %d is completed\n", consumer_index);
                    return NULL;
                }
             
                sleep(2);
            }
        }
    }

    return NULL;
}

void* producer_handler(void* arg)
{
    int i = 0;
    while (running)
    {
        if (pthread_mutex_trylock(&shop_mutexes[i]) == 0)
        {
            shops[i] += PROCUER_AMOUNT;
            printf("Producer: %d is added to shop #%d, new value is %d\n",
                   PROCUER_AMOUNT, i, shops[i]);
            pthread_mutex_unlock(&shop_mutexes[i]);

            sleep(1);
        }

        i++;
        if (i >= SHOPS_NUM) {
            i = 0;
        }
    }

    return NULL;
}

int main() {
    pthread_t consumer_thread_ids[CONSUMERS_NUM];
    pthread_t procuder_thread_id;

    srand(time(NULL));

    for (int i = 0; i < SHOPS_NUM; i++) {
        shops[i] = (rand() % (MAX_SHOP_VAL - MIN_SHOP_VAL + 1)) + MIN_SHOP_VAL;
        pthread_mutex_init(&shop_mutexes[i], NULL);
    }

    for (int i = 0; i < CONSUMERS_NUM; i++) {
        consumers[i] = (rand() % (MAX_NEED - MIN_NEED + 1) + MIN_NEED);

        int *consumer_index_ptr = malloc(sizeof(int));
        *consumer_index_ptr = i;
        if (pthread_create(&consumer_thread_ids[i], NULL, consumer_handler, consumer_index_ptr) != 0) {
            printf("consumer thread error\n");
            return 1;
        }
    }

    if (pthread_create(&procuder_thread_id, NULL, producer_handler, NULL) != 0) {
        printf("producer thread error\n");
        return 1;
    }

    for (int i = 0; i < CONSUMERS_NUM; i++) {
        pthread_join(consumer_thread_ids[i], NULL);
    }

    running = 0;
    pthread_join(procuder_thread_id, NULL);

    for (int i = 0; i < SHOPS_NUM; i++) {
        pthread_mutex_destroy(&shop_mutexes[i]);
    }

    return 0;
}