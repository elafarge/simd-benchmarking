/*
 * =====================================================================================
 *
 *       Filename:  thread_find.h
 *
 *    Description:  A header file for our multithreaded variants of find.
 *
 *        Version:  1.0
 *        Created:  09/01/2016 20:07:38
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

#ifndef _THREAD_FIND_H_
#define _THREAD_FIND_H_

int thread_find(int *U, int i_start, int i_end, int i_step, int val,
                int **ind_val, int k, int ver);

#endif
