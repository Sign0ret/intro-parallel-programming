
// gcc pth3.c -o pth3 -g -lpthread -fsanitize=thread

// NOTE: ThreadSanitizer is currently broken on the lab machines.

#include <stdio.h>
#include <pthread.h>

int x;
pthread_mutex_t mut;

void * increment(void * n) {
    pthread_mutex_lock(&mut);
    x++;
    pthread_mutex_unlock(&mut);
    return NULL;
}

void * decrement(void * n) {
    pthread_mutex_lock(&mut);
    x--;
    pthread_mutex_unlock(&mut);
    return NULL;
}

int main (int argc, char ** argv) {		
    pthread_t thread1, thread2;

    x = 0;
    pthread_mutex_init(&mut, NULL);

    pthread_create(&thread1, NULL, increment, NULL);
    pthread_create(&thread2, NULL, decrement, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("x = %d\n", x);
    return 0;
}
