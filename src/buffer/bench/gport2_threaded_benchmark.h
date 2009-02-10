#ifndef __LIQUID_GPORT2_THREADED_BENCHMARK_H__
#define __LIQUID_GPORT2_THREADED_BENCHMARK_H__

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/resource.h>

#include "liquid.composite.h"

// prototype for thread routines
void gport2_producer_handler ( void *ptr );
void gport2_consumer_handler ( void *ptr );

typedef struct {
    gport2 p;
    unsigned int producer_size;
    unsigned int consumer_size;
    unsigned long int num_trials;
} gport2_threaded_bench_data_t;

#define GPORT2_THREADED_BENCH_API(N)    \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ gport2_threaded_bench(_start, _finish, _num_iterations, N); }

// Helper function to keep code base small
void gport2_threaded_bench(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations,
    unsigned int _n)
{
    // initialize port
    gport2_threaded_bench_data_t data;
    data.p = gport2_create(8*_n,sizeof(int));
    data.producer_size = _n;
    data.consumer_size = _n;
    data.num_trials = *_num_iterations;

    // threads
    pthread_t producer_thread;
    pthread_t consumer_thread;
    pthread_attr_t thread_attr;
    void * status;

    // set thread attributes
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_JOINABLE);

    // create threads
    pthread_create(&producer_thread, &thread_attr, (void*) &gport2_producer_handler, (void*) &data);
    pthread_create(&consumer_thread, &thread_attr, (void*) &gport2_consumer_handler, (void*) &data);

    // destroy attributes object (no longer needed)
    pthread_attr_destroy(&thread_attr);

    // start trials:
    getrusage(RUSAGE_SELF, _start);

    // join threads
    pthread_join(producer_thread, &status);
    pthread_join(consumer_thread, &status);

    getrusage(RUSAGE_SELF, _finish);
    //*_num_iterations *= _n;

    // clean up memory
    gport2_destroy(data.p);
}

void gport2_producer_handler(void * _data)
{
    gport2_threaded_bench_data_t * data = (gport2_threaded_bench_data_t*)_data;
    unsigned long int i;
    int w[data->producer_size];
    for (i=0; i<data->num_trials; i+=data->producer_size) {
        gport2_produce(data->p,(void*)w,data->producer_size);
    }
    pthread_exit(0);
}


void gport2_consumer_handler(void * _data)
{
    gport2_threaded_bench_data_t * data = (gport2_threaded_bench_data_t*)_data;
    unsigned long int i;
    int r[data->consumer_size];
    for (i=0; i<data->num_trials; i+=data->consumer_size) {
        gport2_consume(data->p,(void*)r,data->consumer_size);
    }
    pthread_exit(0);
}

// 
void benchmark_gport2_threaded_n1   GPORT2_THREADED_BENCH_API(1)
void benchmark_gport2_threaded_n4   GPORT2_THREADED_BENCH_API(4)
void benchmark_gport2_threaded_n16  GPORT2_THREADED_BENCH_API(16)
void benchmark_gport2_threaded_n64  GPORT2_THREADED_BENCH_API(64)

#endif // __LIQUID_GPORT2_THREADED_BENCHMARK_H__

