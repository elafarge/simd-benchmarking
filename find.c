/*
 * ============================================================================
 *
 *       Filename:  find.c
 *
 *    Description:  The implementation of our naïve version of find as well as
 *                  it's vectorial (SIMD) counterpart.
 *
 *        Version:  1.0
 *        Created:  09/01/2016 19:58:09
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

#include "find.h"

#include <immintrin.h>

#define test_U_j(j) \
    if(U[j] == val){ \
        (*ind_val) = realloc((*ind_val), (c + 1)*sizeof(int)); \
        (*ind_val)[c] = j; \
        c++; \
     }


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
        test_U_j(i)
    }

    return c;
}

int vect_find(int *U, int i_start, int i_end, int i_step, int val,
              int **ind_val){
    int i;
    int c = 0;

    __m256i cmp_vect __attribute__ ((aligned(32)));

    // Let's build our comparison vector
    cmp_vect = _mm256_set1_epi32(val);

    (*ind_val) = NULL;

    for(i = i_start; i < i_end - 6; i += i_step * 8){
        // Let's go from a cmp_vect to a 4 bits mask with a dirty (but
        // efficient since we don't duplicate variables in memory, we just
        // access U[i] like an array of four ints since we know we can do so
        // thanks to the condition on the for)

        // If the whole mask is null, no matching element: let's move forward
        if(!_mm256_movemask_epi8(_mm256_cmpeq_epi32(cmp_vect, *((__m256i*)(U + i)))))
            continue;

        // Or else let's analyse things one piece at a time
        test_U_j(i);
        test_U_j(i + 1);
        test_U_j(i + 2);
        test_U_j(i + 3);
        test_U_j(i + 4);
        test_U_j(i + 5);
        test_U_j(i + 6);
        test_U_j(i + 7);
    }

    // Let's finish the job for the potentially remaining last few (3 at most)
    // elements
    for( ; i < i_end; i++){
        test_U_j(i);
    }

    return c;
}
