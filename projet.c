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
 *   Organization:  École Nationale SUpérieure des Mines de Paris
 *
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emmintrin.h>

/**
 * A function generating an n-size array of random integers between a and b
 */
int* generate_array(int n, int a, int b){
    int i;

    // Let's create the array
    int *res = malloc(sizeof(int)*n);

    // Let's seed the random number generator using the current time
    srand(time(NULL));

    for(int i = 0; i < n; i++)
        res[i] = rand() % (b-a+1) + a;

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

int simple_realloc(int* U, int n){
    int i, j;
    int* Ubis = malloc(sizeof(int)*((n + 3)/4)*4);

    for(i = 0; i < n - 1; i += 1){
        Ubis[i] = U[i];
    }

    // Note: the line below is not so naïve and would be faster
    // memcpy(Ubis, U, n - 1);

    free(U);

    return Ubis;

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
        if(U[i] == val){
            // Let's make the array one inch bigger (one time the size of an
            // int actually)
            (*ind_val) = simple_realloc((*ind_val), c + 1);

            // And put our current index into into that one
            (*ind_val)[c] = i;

            // I love this line
            c++;
        }
    }

    return c;
}

int* vect_realloc(int *U, int n){
    int i, j;
    int* Ubis = malloc(sizeof(int)*((n + 3)/4)*4);

    for(i = 0; i < n - 1; i += 4){
        if(n - 1 - i < 4){
            for(j = 0; j < 4; j++){
                Ubis[i + j] = U[i + j];
            }
            break;
        }
        // Vectorial movement of variables
        _mm_store_si128((__m128i*) (Ubis + i),  *((__m128i*)(U + i)) );
    }

    free(U);

    return Ubis;
}

int vect_find(int *U, int i_start, int i_end, int i_step, int val, int **ind_val){
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
            (*ind_val) = vect_realloc((*ind_val), c + 1);

            // And put our current index into into that one
            (*ind_val)[c] = i;

            // I love this line
            c++;
        }
    }

    return c;
}

int main(int argc, char **argv){
    clock_t t0, t1, t2, t3;
    int n = 1000000;
    int a = -20;
    int b = 20;

    int i;
    int lookup_value = 12;
    int c1, c2;
    int *ind_val1, *ind_val2;

    int * test_array = generate_array(n, a, b);

    int eq = 1;

    printf("\n-- Let's have a look at the generated array --\n");
    //print_array(test_array, n);
    printf("Running time \n");

    printf("-- Ok let's see where %d is in the array --\n", lookup_value);
    t0 = clock();
    c1 = find(test_array, 0, n, 1, lookup_value, &ind_val1);
    t1 = clock();
    //print_array(ind_val, c);
    printf("Time elapsed: %ld microseconds\n\n",
            (long)(t1-t0)*1000000 / CLOCKS_PER_SEC);

    printf("-- Ok let's see where %d is in the array... vectorially --\n",
            lookup_value);
    t2 = clock();
    c2 = vect_find(test_array, 0, n, 1, lookup_value, &ind_val2);
    t3 = clock();
    //print_array(ind_val, c);
    printf("Time elapsed: %ld microseconds\n\n",
            (long)(t3-t2)*1000000 / CLOCKS_PER_SEC);

    printf("-- Test for equality of the arrays --\n");
    if(c1 == c2)
        printf("  - Both have the same size\n");
    else {
        printf("  - The ind_val arrays don't have the same size ! Stopping...\n");
        return 12;
    }

    for(i = 0; i < c1; i++)
        eq = eq && (ind_val1[i] == ind_val2[i]);

    if(eq)
        printf("  - Both have the same values\n");
    else {
        printf("  - The ind_val arrays don't have the same values ! Stopping...\n");
        return 13;
    }

    printf("\n-- Performance factors --\n");
    printf("  - Against our naive simple_realloc: %f \n", ((float)(t1-t0))/((float)(t3-t2)));


    return 0;
}

/**
 * Increases the size of an array of integer by 1.
 * Complexity O(n) where n designates the size of the array
 *
 * This function is commented out since we figured a similar one was already in
 * stdlib.h but the one in stdlib has to do something similar anyway. Unless it
 * is optimized for the linux kernel to actually take advantage on its virtual
 * memory and memory pages.
 *
 */
// int* realloc(int *U, int n){
//     int i;
//     int *Ubis = malloc(sizeof(int)*(n+1));
//
//     //
//     // Let's copy the elements of U (yeah I know the complexity sucks that's
//     // why it would be smarter to use Linked Lists
//     //
//     // Idea: Create and use a smart linked list with a counter on the number of
//     // elements, a pointer at the first, another one at n/4, another one at n/2
//     // and a last one at 3n/4
//     //
//     for(i = 0; i < n; i++)
//         Ubis[i] = U[i];
//
//     free(U);
//     return Ubis;
// }
