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

// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Unix-specific standard library
#include <unistd.h>

// Project
#include "colors.h"
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

    printf("\n" ANSI_COLOR_MAGENTA
" =======================================================================   \n"
ANSI_STYLE_BOLD
"                       SIMD BENCHMARKING PROGRAM                           \n"
ANSI_STYLE_NO_BOLD
"                   Étienne Lafarge  -  Vincent Villet                     \n"
ANSI_COLOR_BLUE
"             École Nationale Supérieure des Mines de Paris                 \n"
ANSI_COLOR_MAGENTA
" =======================================================================   \n"
    ANSI_COLOR_RESET "\n");

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

    printf(ANSI_STYLE_BOLD
"  [*] Generating the array on which tests will be performed with the  \n"
"      following characteristics: \n" ANSI_STYLE_NO_BOLD
"        * size: \t" ANSI_STYLE_BOLD ANSI_COLOR_BLUE "%d" ANSI_COLOR_RESET
                                                ANSI_STYLE_NO_BOLD " \n"
"        * lower bound:  " ANSI_STYLE_BOLD "%d" ANSI_STYLE_NO_BOLD " \n"
"        * higher bound: " ANSI_STYLE_BOLD "%d" ANSI_STYLE_NO_BOLD " \n", n, a,
    b);

    test_array = generate_array(n, a, b);

    printf(ANSI_COLOR_GREEN ANSI_STYLE_BOLD
"                            -- Done ! -- \n\n" ANSI_STYLE_NO_BOLD
    ANSI_COLOR_RESET);

    printf( ANSI_STYLE_BOLD
"  [*] Looking for element " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET
ANSI_STYLE_BOLD            " using different implementations of find: \n\n"
    ANSI_STYLE_NO_BOLD, lookup_value);
    printf(
"     *-------------------------*--------------*--------------------* \n"
"     |     IMPLEMENTATION      | RUNNING TIME | PERFORMANCE FACTOR | \n"
"     *-------------------------*--------------*--------------------* \n"
"     |                         |              |                    | \n");

    clock_gettime(CLOCK_MONOTONIC, &t0);
    c1 = find(test_array, 0, n, 1, lookup_value, &ind_val1);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    d1 = tdiff_micros(t0, t1);

    printf(
"     |     " ANSI_STYLE_BOLD
           "find() (scalar)" ANSI_STYLE_NO_BOLD
                          "     | "ANSI_STYLE_BOLD ANSI_COLOR_MAGENTA
                                 "%9ld ms" ANSI_STYLE_NO_BOLD ANSI_COLOR_RESET
                                             " |      "
                  ANSI_COLOR_MAGENTA ANSI_STYLE_BOLD "xxxxxxxx"
                         ANSI_STYLE_NO_BOLD ANSI_COLOR_RESET "      |\n"
"     |                         |              |                    | \n", d1);

    clock_gettime(CLOCK_MONOTONIC, &t2);
    c2 = vect_find(test_array, 0, n, 1, lookup_value, &ind_val2);
    clock_gettime(CLOCK_MONOTONIC, &t3);
    d2 = tdiff_micros(t2, t3);

    printf(
"     |       " ANSI_STYLE_BOLD
           "vect_find()    " ANSI_STYLE_NO_BOLD
                          "   | "ANSI_STYLE_BOLD ANSI_COLOR_BLUE
                                 "%9ld ms" ANSI_STYLE_NO_BOLD ANSI_COLOR_RESET
                                             " |       "
                      ANSI_STYLE_BOLD ANSI_COLOR_BLUE "x%5.2f"
                        ANSI_STYLE_NO_BOLD ANSI_COLOR_RESET "       |\n"
"     |                         |              |                    | \n", d2,
    ((float)d1)/d2);

    clock_gettime(CLOCK_MONOTONIC, &t4);
    c3 = thread_find(test_array, 0, n, 1, lookup_value, &ind_val3, -1, 0);
    clock_gettime(CLOCK_MONOTONIC, &t5);
    d3 = tdiff_micros(t4, t5);
    printf(
"     | " ANSI_STYLE_BOLD
           " thread_find() (scalar)" ANSI_STYLE_NO_BOLD
                          " | "ANSI_STYLE_BOLD ANSI_COLOR_CYAN
                                 "%9ld ms" ANSI_STYLE_NO_BOLD ANSI_COLOR_RESET
                                             " |       "
                      ANSI_STYLE_BOLD ANSI_COLOR_CYAN "x%5.2f"
                        ANSI_STYLE_NO_BOLD ANSI_COLOR_RESET "       |\n"
"     |                         |              |                    | \n", d3,
    ((float)d1)/d3);

    clock_gettime(CLOCK_MONOTONIC, &t6);
    c4 = thread_find(test_array, 0, n, 1, lookup_value, &ind_val4, -1, 1);
    clock_gettime(CLOCK_MONOTONIC, &t7);
    d4 = tdiff_micros(t6, t7);
    printf(
"     |" ANSI_STYLE_BOLD
           "  thread_find() (vect.)  " ANSI_STYLE_NO_BOLD
                          "| "ANSI_STYLE_BOLD ANSI_COLOR_GREEN
                                 "%9ld ms" ANSI_STYLE_NO_BOLD ANSI_COLOR_RESET
                                             " |       "
                     ANSI_STYLE_BOLD ANSI_COLOR_GREEN "x%5.2f"
                        ANSI_STYLE_NO_BOLD ANSI_COLOR_RESET "       |\n"
"     |                         |              |                    | \n"
"     *-------------------------*--------------*--------------------* \n\n",
    d4, ((float)d1)/d4);


    //-------------------------------------------------------------------------
    // OK, all performance comparisons and computations are made but let's make
    // sure we actually get the appropriate results.
    //-------------------------------------------------------------------------
    printf( ANSI_STYLE_BOLD
"  [*] Testing the correctness of all our implementations: \n"
    ANSI_STYLE_NO_BOLD );

    if(c1 == c2 && c1 == c3 && c1 == c4)
        printf("       - The ind_val arrays all have the " ANSI_COLOR_GREEN
                ANSI_STYLE_BOLD "same size" ANSI_COLOR_RESET
                ANSI_STYLE_NO_BOLD".\n");
    else {
        printf("       - The ind_val arrays" ANSI_COLOR_RED ANSI_STYLE_BOLD
               " don't have the same size" ANSI_COLOR_RESET ANSI_STYLE_NO_BOLD
               " (%d %d %d %d)! Stopping...\n", c1, c2, c3, c4);

        free(ind_val1);
        free(ind_val2);
        free(ind_val3);
        free(ind_val4);

        return 12;
    }

    // Let's see if values are the same
    eq = 1;

    for(i = 0; i < c1; i++)
        eq = eq && (ind_val1[i] == ind_val2[i] && ind_val1[i] == ind_val3[i])
                && (ind_val1[i] == ind_val4[i]);

    if(eq)
        printf("       - All have the " ANSI_COLOR_GREEN ANSI_STYLE_BOLD
               "same values" ANSI_COLOR_RESET ANSI_STYLE_NO_BOLD ".\n");
    else {
        printf("       - The ind_val arrays " ANSI_COLOR_RED ANSI_STYLE_BOLD
               "don't have the same values" ANSI_COLOR_RESET ANSI_STYLE_NO_BOLD
               " ! Stopping...\n");

        free(ind_val1);
        free(ind_val2);
        free(ind_val3);
        free(ind_val4);

        return 13;
    }


    // Let's make sure our k-factor works as expected
    if(k >= 0){
        c5 = thread_find(test_array, 0, n, 1, lookup_value, &ind_val5, k, 0);
        c6 = thread_find(test_array, 0, n, 1, lookup_value, &ind_val6, k, 1);

        free(ind_val5);
        free(ind_val6);

        if( (k == c5 || c5 == c1) && (k >= c6 || c6 == c1))
            printf("       - " ANSI_COLOR_GREEN ANSI_STYLE_BOLD "The "
                   "k-factor works as expected" ANSI_COLOR_RESET
                   ANSI_STYLE_NO_BOLD", careful though, we have \n         "
                   "no reasons to get the first k occurences of the element\n"
                   "         we're searching for!\n");
        else{
            printf("       - " ANSI_COLOR_RED ANSI_STYLE_BOLD "The k-factor "
                   "doesn't behave as expected" ANSI_COLOR_RESET
                   ANSI_STYLE_NO_BOLD". \n           Debug info: k = %d, c5 = "
                   "%d, c6 = %d\n           Exiting...\n", k, c5, c6);

            free(ind_val1);
            free(ind_val2);
            free(ind_val3);
            free(ind_val4);

            return 14;
        }
    }

    printf("\n" ANSI_COLOR_MAGENTA
" =======================================================================   \n"
"   The results will be reprinted below for an easier CSV-like parsing.    \n"
" =======================================================================   \n"
    ANSI_COLOR_RESET );

    p_vect = ((float)d1)/d2;
    p_parrallel = ((float)d1)/d3;
    p_parrallel_vect = ((float)d1)/d4;
    p_vect_bis = ((float)d3)/d4;

    //-------------------------------------------------------------------------
    // A simple output for scripts to run this program with different sets of
    // parameters, retrieve the performance (you can see this output as a line
    // in a csv file for instance) and draw nice performance graphs!
    //-------------------------------------------------------------------------
    printf("\n%ld %ld %ld %ld %f %f %f %f\n", d1, d2, d3, d4, p_vect,
           p_vect_bis, p_parrallel, p_parrallel_vect);


    free(ind_val1);
    free(ind_val2);
    free(ind_val3);
    free(ind_val4);

    return 0;
}
