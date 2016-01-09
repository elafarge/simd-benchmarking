/*
 * ============================================================================
 *
 *       Filename:  utilities.h
 *
 *    Description:  A couple of utility functions for our project
 *
 *        Version:  1.0
 *        Created:  01/08/2015 20:27:09
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

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

// Let's choose the POSIX definitions we want
#define _XOPEN_SOURCE 600

#include <time.h>

// Our finely crafted max macro
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/**
 * A function generating an n-size array of random integers between a and b
 */
int* generate_array(int n, int a, int b);

void print_array(int* U, int n);

long tdiff_micros(struct timespec t0, struct timespec t1);

int get_number_of_cores();

#endif
