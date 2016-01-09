/*
 * ============================================================================
 *
 *       Filename:  projet.c
 *
 *    Description:  The source code for the personal project built as per the
 *                  introductory course about the hardware and software
 *                  architecture of computers.
 *
 *        Version:  1.0
 *        Created:  10/12/2015 13:55:24
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

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <immintrin.h>
#include <unistd.h>
#include <pthread.h>

#include "utilities.h"
#include "cli_arguments.h"

// Global count and max global count
int *gc = NULL;
int mgc;

/**
 * Looks for val in U between the indexes i_start and i_end and jumping by
 * i_step at a time. It will return the number of found occurences of val and
 * put their positions in the pointer **ind_val.
 *
 * Why is **ind_val a pointer on a pointer of ints ? Well that's pretty simple:
 * what we need to do is populate an array, which is represented by a pointer
 * (*int_val) and a size (called c and returned by the function find). At first
 * you'd think that just passing a pointer and make it change would be fine,
 * but the point is on a function call, the arguments are copied in memory.
 * Therefore, it's that copy of the pointer that will be modified by find, not
 * the pointer itself. That's why we need to get a pointer pointing on that
 * pointer: therefore the pointer gives us the address of the pointer pointing
 * at the beginning of the array and we we can use this "address" to actually
 * modify the array every time we call realloc.
 *
 * In short the answer is: "because arguments get copied when a function is
 * called so we need to use pointers, and since the argument itself is a
 * pointer, we need a pointer on a pointer".
 */
int find(int *U, int i_start, int i_end, int i_step, int val, int **ind_val){
    int i;
    int c = 0;

    // So we have no results so far, let's have int_val point to nothing in the
    // first place
    (*ind_val) = NULL;

    // Ok let's start looking for things
    for(i = i_start; i < i_end; i += i_step){
        if(U[i] == val){
            // Let's make the array one inch bigger (one time the size of an
            // int actually)
            (*ind_val) = realloc((*ind_val), (c + 1)*sizeof(int));

            // And put our current index into into that one
            (*ind_val)[c] = i;

            c++;
        }
    }

    return c;
}

int vect_find(int *U, int i_start, int i_end, int i_step, int val,
              int **ind_val){
    int i, j, s;
    int c = 0;

    __m256i cmp_vect, cmp_res;

    // Let's build our comparison vector
    cmp_vect = _mm256_set1_epi32(val);

    (*ind_val) = NULL;

    for(i = i_start; i < i_end - 6; i += i_step * 8){
        // Let's go from a cmp_vect to a 4 bits mask with a dirty (but
        // efficient since we don't duplicate variables in memory, we just
        // access U[i] like an array of four ints since we know we can do so
        // thanks to the condition on the for)

        // If the whole mask is null, no matching element: let's move forward
        cmp_res = _mm256_cmpeq_epi32(cmp_vect, *((__m256i*)(U + i)));

        if(!_mm256_movemask_epi8(cmp_res))
            continue;

        // Or else let's analyse things one piece at a time
        s = i + 8;
        for(j = i; j < s; j++){
            if(U[j] == val){
                // Let's put a chunk of 4 elements in a vector register
                (*ind_val) = realloc((*ind_val), (c + 1)*sizeof(int));
                (*ind_val)[c] = j;
                c++;
            }
        }
    }

    // Let's finish the job for the potentially remaining last few (3 at most)
    // elements
    for( ; i < i_end; i++){
        if(U[i] == val){
            // TODO: let's move this part in an inline function someday
            (*ind_val) = realloc((*ind_val), (c + 1)*sizeof(int));
            (*ind_val)[c] = i;
            c++;
        }
    }

    return c;
}

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
    int i_start, i_end, i_step, val, i, s;
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

    *c = vect_find(U, i_start, i_end, i_step, val, ind_val);

    return (void*) c;
}

int thread_find(int *U, int i_start, int i_end, int i_step, int val,
                int **ind_val, int k, int ver){
    int n_threads, i, j, c, min, l;
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
        attr[i].i_start = i;
        attr[i].i_end = i_end;
        attr[i].i_step = n_threads;
        attr[i].i_start = i_start + (i_end - i_start)/n_threads * i;
        attr[i].i_end = min(i_start + (i_end - i_start)/n_threads * (i + 1),
                            i_end);
        attr[i].i_step = 1;
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

int main(int argc, char **argv){
    struct timespec t0, t1, t2, t3, t4, t5, t6, t7;
    long d1, d2, d3, d4;
    int n, a, b, i, lookup_value, k, c1, c2, c3, c4, c5, c6, eq;
    float p_vect, p_parrallel, p_parrallel_vect, p_vect_bis;
    int *ind_val1, *ind_val2, *ind_val3, *ind_val4, *ind_val5, *ind_val6;
    int* test_array;
    struct arguments *arguments;

    //-------------------------------------------------------------------------
    // BEGINNING OF ARGUMENTS PARSING
    //-------------------------------------------------------------------------
    arguments = parse_cli_arguments(argc, argv);

    n = arguments->n;
    a = arguments->a;
    b = arguments->b;
    k = arguments->k;
    lookup_value = arguments->f;

    free(arguments);
    //-------------------------------------------------------------------------
    // END OF ARGUMENTS PARSING
    //-------------------------------------------------------------------------
    printf("\n-- Let's generate a test array of size %d --\n", n);
    test_array = generate_array(n, a, b);

    printf("\n-- Let's have a look at the generated array --\n");

    printf("-- Ok let's see where %d is in the array --\n", lookup_value);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    c1 = find(test_array, 0, n, 1, lookup_value, &ind_val1);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    d1 = tdiff_micros(t0, t1);
    printf("Time elapsed: %ld microseconds\n\n", d1);

    printf("-- Ok let's see where %d is in the array... vectorially --\n",
            lookup_value);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    c2 = vect_find(test_array, 0, n, 1, lookup_value, &ind_val2);
    clock_gettime(CLOCK_MONOTONIC, &t3);
    d2 = tdiff_micros(t2, t3);
    printf("Time elapsed: %ld microseconds\n\n", d2);

    printf("-- Ok let's see where %d is in the array... using a parrallelized "
           "version of the unthreaded approach --\n", lookup_value);
    clock_gettime(CLOCK_MONOTONIC, &t4);
    c3 = thread_find(test_array, 0, n, 1, lookup_value, &ind_val3, -1, 0);
    clock_gettime(CLOCK_MONOTONIC, &t5);
    d3 = tdiff_micros(t4, t5);
    printf("Time elapsed: %ld microseconds\n\n", d3);

    printf("-- Ok let's see where %d is in the array... using a parrallelized "
           "version of the vectorial unthreaded approach --\n", lookup_value);
    clock_gettime(CLOCK_MONOTONIC, &t6);
    c4 = thread_find(test_array, 0, n, 1, lookup_value, &ind_val4, -1, 1);
    clock_gettime(CLOCK_MONOTONIC, &t7);
    d4 = tdiff_micros(t6, t7);
    printf("Time elapsed: %ld microseconds\n\n", d4);

    if(k >= 0){
        printf("Ok, now let's just check that introducing a factor k limiting "
               "the output doesn't make our algorithms flaky !\n");
        c5 = thread_find(test_array, 0, n, 1, lookup_value, &ind_val5, k, 0);
        c6 = thread_find(test_array, 0, n, 1, lookup_value, &ind_val6, k, 1);

        free(ind_val5);
        free(ind_val6);

        if(k == c5 && k == c6)
            printf("The k-factor works as expected, careful though, we have "
                   "no reasons to get the first k occurences of the element "
                   "we're searching for!\n");
        else{
            printf("The k-factor doesn't behave as expected. \n "
                   "Debug info: k = %d, c5 = %d, c6 = %d\nExiting...\n", k, c5,
                   c6);

            free(ind_val1);
            free(ind_val2);
            free(ind_val3);
            free(ind_val4);

            return 14;
        }
    }

    //-------------------------------------------------------------------------
    // OK, all performance comparisons and computations are made but let's make
    // sure we actually get the appropriate results.
    //-------------------------------------------------------------------------
    printf("-- Test for equality of the arrays --\n");
    if(c1 == c2 && c1 == c3 && c1 == c4)
        printf("  - All have the same size :)\n");
    else {
        printf("  - The ind_val arrays don't have the same size (%d %d %d %d)!"
               " Stopping...\n", c1, c2, c3, c4);

        free(ind_val1);
        free(ind_val2);
        free(ind_val3);
        free(ind_val4);

        return 12;
    }

    // Let's see if values are the same
    eq = 1;

    for(i = 0; i < c1; i++){
        if(ind_val1[i] != ind_val2[i])
            printf("Erreur à l'indice %d , %d != %d !!\n", i, ind_val1[i], ind_val2[i]);
    }

    for(i = 0; i < c1; i++)
        eq = eq && (ind_val1[i] == ind_val2[i] && ind_val1[i] == ind_val3[i])
                && (ind_val1[i] == ind_val4[i]);

    if(eq)
        printf("  - All have the same values :D :D \n");
    else {
        printf("  - The ind_val arrays don't have the same values ! "
               "Stopping...\n");

        free(ind_val1);
        free(ind_val2);
        free(ind_val3);
        free(ind_val4);

        return 13;
    }

    printf("\n-- Performance factors --\n");

    p_vect = ((float)d1)/d2;
    p_parrallel = ((float)d1)/d3;
    p_parrallel_vect = ((float)d1)/d4;
    p_vect_bis = ((float)d3)/d4;

    printf("  - Vectorized find against our scalar find: %f \n",
           p_vect);
    printf("  - Vectorized find (multi-threaded) against our scalar find "
           "(in its multi-threaded variant too): %f \n", p_vect_bis);
    printf("  - Multi-threaded against mono-threaded (both scalar): %f \n",
            p_parrallel);
    printf("  - Multi-threaded and vectorized against mono-threaded scalar "
           "find: %f \n", p_parrallel_vect);


    //-------------------------------------------------------------------------
    // A simple output for scripts to run this program with different sets of
    // parameters, retrieve the performance (you can see this output as a line
    // in a csv file for instance) and draw nice performance graphs!
    //-------------------------------------------------------------------------
    printf("\n -- Simple output for benchmarking scripts -- \n\n");
    printf("T_NAIVE T_VECT T_MT_NAIVE T_MT_VECT PERF_VECT PERF_VECT_BIS "
           "PERF_MT PERF_MT_VECT \n");
    printf("%ld %ld %ld %ld %f %f %f %f\n", d1, d2, d3, d4, p_vect, p_vect_bis,
           p_parrallel, p_parrallel_vect);

    free(ind_val1);
    free(ind_val2);
    free(ind_val3);
    free(ind_val4);

    return 0;
}
