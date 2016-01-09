/*
 * =====================================================================================
 *
 *       Filename:  utilities.c
 *
 *    Description:  The implementation of our utility functions to create an
 *                  array containing random integers between a and b, printing
 *                  arrays and playing with high precision time measurements.
 *
 *        Version:  1.0
 *        Created:  08/01/2016 20:53:44
 *       Revision:  none
 *       Compiler:  gcc
 *
 *        Authors:  Etienne LAFARGE (etienne.lafarge@mines-paristech.fr),
 *                  Vincent Villet (vincent.villet@mines-paristech.fr)
 *
 *   Organization:  École Nationale Supérieure des Mines de Paris
 *
 * =====================================================================================
 */
#include "utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * A function generating an n-size array of random integers between a and b
 */
int* generate_array(int n, int a, int b){
    int i;

    // Let's create the array
    int *res;

    posix_memalign((void**) &res, 32, sizeof(int) * n);

    // Let's seed the random number generator using the current time
    srand(time(NULL));

    // And let's fill up the array in a vectorial way
    for(i = 0; i < n; i++)
        res[i] = rand() % (b - a + 1) + a;

    return res;
}

void print_array(int* U, int n){
    int i;
    printf("[");
    for(i = 0; i < n; i++){
        if(i == n - 1)
            printf("%d", U[i]);
        else
            printf("%d, ", U[i]);
    }
    printf("]\n");
}

long tdiff_micros(struct timespec t0, struct timespec t1){
    return (long)(t1.tv_sec - t0.tv_sec)*1000000 +
           (t1.tv_nsec - t0.tv_nsec)/1000;
}

int get_number_of_cores(){
    // Works only on Linux with GCC/glibc, relies on unistd.h
    return sysconf(_SC_NPROCESSORS_ONLN);
}
