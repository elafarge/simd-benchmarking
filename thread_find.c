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

#include "find.h"
#include "utilities.h"

// Global count and max global count
int *gc = NULL;
int mgc;

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
    int i_start, i_end, i_step, val ;
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

    *c = find(U, i_start, i_end, i_step, val, ind_val);

    return (void*) c;
}

void* vect_find_threadable(void* args){
    int *U;
    int i_start, i_end, i_step, val;
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

    *c = vect_find(U, i_start, i_end, i_step, val, ind_val);

    return (void*) c;
}

int thread_find(int *U, int i_start, int i_end, int i_step, int val,
                int **ind_val, int k, int ver){
    int n_threads, i, c, l;
    int *partial_count;
    int ***ind_vals;

    // A pointer on the find function to use
    void* (*find_routine)(void*);

    // Let's use the common n_cores + 1 rule which is supposed to give the best
    // results. The " +1 " is simply the main thread which will check if the
    // number of occurences to find has been reached.
    n_threads = get_number_of_cores();

    // It's all about splitting the problem into shifted chunks. For instance,
    // if we need to go through all the elements and have five threads let's
    // take care of the 5k ones with the first core, the 5k+1 with the second
    // one... etc
    if(ver == 0)
        find_routine = &find_threadable;
    else
        find_routine = &vect_find_threadable;

    // Let's construct an ind_val for each thread
    ind_vals = malloc(n_threads*sizeof(int**));

    // Let's make room in memory for our threads
    pthread_t thread[n_threads];
    struct thread_data attr[n_threads];

    // Pointers to put results into a single array in the right order
    int s[n_threads];

    if(k > 0){
        gc = malloc(sizeof(int));
        *gc = 0;
        mgc = k;
    } else
        gc = NULL;


    for(i = 0; i < n_threads; i++){
        attr[i].U = U;
        attr[i].i_start = i_start + (i_end - i_start)/n_threads * i;
        attr[i].i_end = min(i_start + (i_end - i_start)/n_threads * (i + 1),
                            i_end);
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
    if(k > 0 && c > k) // TODO truncate in the memcpy below if k is defined
        c = k;

    (*ind_val) = malloc(sizeof(int) * c);

    // And now we just have to concatenate our arrays, so cool and fast
    l = 0;
    for(i = 0; i < n_threads; i++){
        memcpy(*ind_val + l, *ind_vals[i], s[i]*sizeof(int));
        l += s[i];
    }

    // Let's free our last resources
    free(gc);
    free(ind_vals);

    return c;
}

