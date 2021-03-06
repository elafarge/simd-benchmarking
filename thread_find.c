/*
 * ============================================================================
 *
 *       Filename:  thread_find.c
 *
 *    Description:  Implementation of our multithreaded versions of find:
 *                  thread_find.
 *
 *        Version:  1.0
 *        Created:  09/01/2016 20:05:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *        Authors:  Etienne LAFARGE (etienne.lafarge@mines-paristech.fr),
 *                  Vincent Villet (vincent.villet@mines-paristech.fr)
 *
 *   Organization:  École Nationale Supérieure des Mines de Paris
 *
 * ============================================================================
 */
#include "thread_find.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <stdio.h>

#include "find.h"
#include "utilities.h"

// Note that we perform the mutex unlocking operation ASAP here in order to
// minimize the waiting time for potentially blocked threads.
#define test_U_j_with_gc(j) \
    if(U[j] == val){ \
        pthread_mutex_lock(&gc_lock); \
        if(*gc >= mgc){ \
            pthread_mutex_unlock(&gc_lock); \
            pthread_exit((void*) c); \
        } \
        (*gc)++; \
        pthread_mutex_unlock(&gc_lock); \
        (*ind_val) = realloc((*ind_val), (*c + 1)*sizeof(int)); \
        (*ind_val)[*c] = j; \
        (*c)++; \
     }

// Global count and max global count
int *gc = NULL;
int mgc;

// A mutex for *gc access
pthread_mutex_t gc_lock;

struct thread_data{
    int *U;
    int i_start;
    int i_end;
    int i_step;
    int val;
    int **ind_val;
};

void* find_threadable(void* args){
    // Arguments passing
    int *U;
    int i_start, i_end, i_step, val, i;
    int **ind_val;
    struct thread_data* targs;
    int *c;

    targs = (struct thread_data*) args;
    U = targs->U;
    i_start = targs->i_start;
    i_end = targs->i_end;
    i_step = targs->i_step;
    val = targs->val;
    ind_val = targs->ind_val;

    c = malloc(sizeof(int));

    if(gc == NULL)
        *c = find(U, i_start, i_end, i_step, val, ind_val);
    else {
        // Let's take the k-factor into account
        *c = 0;

        (*ind_val) = NULL;

        for(i = i_start; i < i_end; i += i_step){
            test_U_j_with_gc(i);
        }
    }

    pthread_exit((void*) c);
}

void* vect_find_threadable(void* args){
    int *U;
    int i_start, i_end, i_step, val, i;
    int **ind_val;
    struct thread_data* targs;
    int *c;

    __m256i cmp_vect __attribute__((aligned (32))),
            cmp_res  __attribute__((aligned (32)));

    targs = (struct thread_data*) args;
    U = targs->U;
    i_start = targs->i_start;
    i_end = targs->i_end;
    i_step = targs->i_step;
    val = targs->val;
    ind_val = targs->ind_val;

    c = malloc(sizeof(int));

    if(gc == NULL)
        *c = vect_find(U, i_start, i_end, i_step, val, ind_val);
    else {
        *c = 0;
        cmp_vect = _mm256_set1_epi32(val);

        (*ind_val) = NULL;

        for(i = i_start; i < i_end - 6; i += i_step * 8){
            cmp_res = _mm256_cmpeq_epi32(cmp_vect, *((__m256i*)(U + i)));

            if(!_mm256_movemask_epi8(cmp_res))
                continue;

            test_U_j_with_gc(i);
            test_U_j_with_gc(i + 1);
            test_U_j_with_gc(i + 2);
            test_U_j_with_gc(i + 3);
            test_U_j_with_gc(i + 4);
            test_U_j_with_gc(i + 5);
            test_U_j_with_gc(i + 6);
            test_U_j_with_gc(i + 7);
        }

        for( ; i < i_end; i++){
            test_U_j_with_gc(i);
        }
    }

    pthread_exit((void*) c);
}

int thread_find(int *U, int i_start, int i_end, int i_step, int val,
                int **ind_val, int k, int ver){
    int n_threads, i, c, l, chunk_size;
    int *partial_count;
    int *s; // The number of matches returned by each thread
    int ***ind_vals;
    pthread_t *thread; // An array containing our threads
    struct thread_data *attr;

    // A pointer on the find function to use
    void* (*find_routine)(void*);

    // Let's use the common n_cores + 1 rule which is supposed to give the best
    // results. The " +1 " is simply the main thread which will check if the
    // number of occurences to find has been reached. (Also it's with the value
    // that we managed to reach the highest performance with the best
    // reproductability).
    n_threads = get_number_of_cores();

    if(ver == 0)
        find_routine = &find_threadable;
    else
        find_routine = &vect_find_threadable;

    // Let's make room in memory for our threads...
    thread = malloc(n_threads * sizeof(pthread_t));
    attr = malloc(n_threads * sizeof(struct thread_data));

    // ... and the result of their execution
    s = malloc(n_threads * sizeof(int));
    ind_vals = malloc(n_threads * sizeof(int**));


    if(k > 0){
        gc = malloc(sizeof(int));
        *gc = 0;
        mgc = k;
        pthread_mutex_init(&gc_lock, NULL);
    } else
        gc = NULL;


    for(i = 0; i < n_threads; i++){
        attr[i].U = U;
        // We have to round that up to make sure our subarrays are
        // aligned too (that's why we get a segfault when the number of threads
        // we launch is odd.
        chunk_size = (i_end - i_start)/n_threads;
        chunk_size -= (chunk_size % 8);
        attr[i].i_start = i_start + chunk_size * i;
        if(i < n_threads - 1)
            attr[i].i_end = i_start + chunk_size * (i + 1);
        else
            attr[i].i_end = i_end;
        attr[i].i_step = i_step;
        attr[i].val = val;
        ind_vals[i] = malloc(sizeof(int*));
        *ind_vals[i] = NULL;
        attr[i].ind_val = ind_vals[i];

        // Let's launch our individual threads
        pthread_create(&thread[i], NULL, find_routine,
                       (void *)((struct thread_data*) &attr[i]));
    }

    // Let's just wait for our threads to finish no matter the reason
    // And let's also initialize pointers for the single array creation
    for(i = 0; i < n_threads; i++){
        pthread_join(thread[i], (void **) &partial_count);
        s[i] = *partial_count;
    }
    free(partial_count);

    // Let's prepare the final data structures
    c = 0;
    for(i = 0; i < n_threads; i++){
        c += s[i];
    }

    (*ind_val) = malloc(sizeof(int) * c);

    // And now we just have to concatenate our arrays, so cool and fast
    // ... if we don't have a k-factor... otherwise we'll
    l = 0;
    for(i = 0; i < n_threads; i++){
        if(c - l > 0)
            memcpy(*ind_val + l, *ind_vals[i], min(s[i], c - l)*sizeof(int));
        l += min(s[i], c - l);
        free(*ind_vals[i]);
    }

    // Let's free our last resources
    if(gc != NULL){
        free(gc);
        pthread_mutex_destroy(&gc_lock);
    }
    free(ind_vals);

    return c;
}

