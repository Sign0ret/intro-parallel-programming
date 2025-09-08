// gcc pth1.c -o pth1 -lpthread

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex;

void * say_hello(void * n) {
	pthread_mutex_lock(&mutex);
	printf("Hello ");
	usleep(100);
	printf("world ");
	usleep(100);
	printf("from ");
	usleep(100);
	printf("thread ");
	usleep(100);
	printf("%d", *(int *) n);
	usleep(100);
	printf("!\n");
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main (int argc, char ** argv) {
	int nthreads = 16;
	
	if (argc > 1)
		nthreads = atoi(argv[1]);
		
	pthread_t * my_threads = malloc(nthreads * sizeof(pthread_t));

	pthread_mutex_init(&mutex, NULL);
	int thread_ids[nthreads];
	for (long i = 0; i < nthreads; i++) {
		thread_ids[i] = i;
		pthread_create(&my_threads[i], NULL, say_hello, (void *) &thread_ids[i]);
	}
		
	for (long i = 0; i < nthreads; i++) {
		pthread_join(my_threads[i], NULL);
	}

	return 0;
}
