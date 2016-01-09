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
#include <unistd.h>

#include "cli_arguments.h"
#include "find.h"
#include "thread_find.h"
#include "utilities.h"


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
