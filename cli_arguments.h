/*
 * =====================================================================================
 *
 *       Filename:  cli_arguments.h
 *
 *    Description:  All the CLI argument parsing related functions for simdbmk
 *
 *        Version:  1.0
 *        Created:  09/01/2016 17:21:23
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

#ifndef _CLI_ARGUMENTS_H_
#define _CLI_ARGUMENTS_H_

#include <argp.h>

struct arguments {
    int n;
    int a;
    int b;
    int k;
    int f;
};

struct arguments* parse_cli_arguments(int argc, char ** argv);

#endif
