/*
 * =====================================================================================
 *
 *       Filename:  sandbox.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  10/12/2015 16:28:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Etienne LAFARGE (), etienne.lafarge@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <emmintrin.h>

int main(int argc, char ** argv){

    int o[4] = {1, 2, 3, 4};
    __m128i v = _mm_set_epi32(1, 2, 3, 4);
    __m128i c = _mm_set_epi32(2, 2, 2, 2);
    __m128i rr;
    int r[4];

    _mm_store_si128( (__m128i*)r, (*((__m128i*) o)) );

    printf("%d %d %d %d\n", r[0], r[1], r[2], r[3]);

    return 0;
}
