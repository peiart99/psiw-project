#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <errno.h>
#include <time.h>

#define K 7
#define W 20

sem_t forks[5];
sem_t mutex;

struct Node {
    int dish_weight;
    struct Node* next;
};

struct Node* head = NULL;

void printDishes(struct Node* n) {
    printf("\n");
    while(n != NULL) {
        printf("%d, ", n->dish_weight);
        n = n->next;
    }
    printf("\n");
}

int countDishes(struct Node* n) {
    int count = 0;
    while(n != NULL) {
        count++;
        n = n->next;
    }

    return count;
}

int checkWeight(struct Node* n) {
    int weight = 0;
    while(n != NULL) {
        weight += n->dish_weight;
        n = n->next;
    }

    return W - weight;
}

void makeDish(struct Node** n) {

    struct Node* new_dish = (struct Node*)malloc(sizeof(struct Node));
    struct Node* last = *n;
    if(countDishes(last) == K) {
        printf("Cannot make a dish. The table is full!\n");
        return;
    }
    int max_weight = checkWeight(last);
    if(max_weight <= 0) {
        printf("Cannot make a dish. The table can't hold any more weight!\n");
        return;
    }

    int new_weight = rand() % max_weight + 1;
    new_dish->dish_weight = new_weight;
    new_dish->next = NULL;

    if(*n == NULL) {
        *n = new_dish;
        return;
    }

    while(last->next != NULL) {
        last = last->next;
    }

    last->next = new_dish;
    return;

}

void eatDish(struct Node** n) {
    struct Node *temp = *n, *prev;
    if(temp == NULL) {
        return;
    }
    if(temp->next == NULL) {
        *n = NULL;
        free(temp);
        return;
    }

    while(temp->next != NULL)
    {
        prev = temp;
        temp = temp->next;
    }

    prev->next = NULL;
    free(temp);
}



void* chef(void *arg) {
    long int index = (long int) arg;

    while(1){
        printf("Cook %ld is hungry!\n", index);

        sem_wait(&mutex);
        sem_wait(&forks[index]);

        sleep(1);
        sem_wait(&forks[(index + 1) % 5]);

        if(countDishes(head) < 3) {
            printf("Cook %ld is going to cook!\n", index);
            makeDish(&head);
        }else {
            printf("Cook %ld is going to eat!\n", index);
            eatDish(&head);
        }

        sleep(1);

        sem_post(&forks[(index + 1) % 5]);
        sem_post(&forks[index]);
        sem_post(&mutex);

        printf("Cook %ld is going to think!\n", index);
        sleep(2);
    }
    pthread_exit(NULL);
}


int main(void) {

    srand(time(NULL));
    makeDish(&head);
    makeDish(&head);
    makeDish(&head);
    makeDish(&head);
    makeDish(&head);
    makeDish(&head);
    makeDish(&head);



    pthread_t thread[5];

    for (long int i = 0; i < 5; i++) {
        if(sem_init(&forks[i], 0, 1) == -1) {
            perror("Failed to create a fork semaphore");
        }
    }
    if(sem_init(&mutex, 0, 1) == -1) {
        perror("Failed to create the mutex semaphore");
    }

    for(long int i = 0; i < 5; i++) {
        if(pthread_create(&thread[i], NULL, &chef, (void*)i) != 0) {
            perror("Failed to create a cook thread");
        }
    }

    for(long int i = 0; i < 5; i++) {
        if(pthread_join(thread[i], NULL) != 0) {
            perror("Failed to join a cook thread");
        }
    } 
    
    return 0;
}