/*
 * =====================================================================================
 *
 *       Filename:  cli_arguments.c
 *
 *    Description:  CLI argument parsing implementation
 *
 *        Version:  1.0
 *        Created:  09/01/2016 19:13:39
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
#include "cli_arguments.h"

#include <stdlib.h>

// These constants aren't needed in the header file so let's put them here to
// prevent name conflicts

const char *argp_program_version = "SIMDerizing benchmark";
const char *argp_program_bug_address = "<etienne.lafarge@mines-paristech.fr>, "
                                       "<vincent.villet@mines-paristech.fr>";
static char doc[] = "A simple C program to benchmark the performance gain "
                    "obtained using SIMD instructions and parrallel computing "
                    "in memory movement algorithms.";
static char args_doc[] = "";
static struct argp_option options[] = {
    { "size", 'n', "COUNT", OPTION_ARG_OPTIONAL, "The size of the array of "
        "generated random integers (default: 1,000,000)."},
    { "min", 'a', "COUNT", OPTION_ARG_OPTIONAL, "The smallest int of the "
        "randomly generated range of ints (default: 0)"},
    { "max", 'b', "COUNT", OPTION_ARG_OPTIONAL, "The highest int of the "
        "randomly generated range of ints (default: 1000)"},
    { "limit-search", 'k', "COUNT", OPTION_ARG_OPTIONAL, "Limits the search "
        "to the first k occurences found (default: -1 i.e. no limit)"},
    { "lookup", 'f', "COUNT", OPTION_ARG_OPTIONAL, "The value to search for "
        "(must be between a and b, defaults to 12)."}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
        case 'n': arguments->n = arg ? atoi (arg) : 1000000; break;
        case 'a': arguments->a = arg ? atoi (arg) : 0; break;
        case 'b': arguments->b = arg ? atoi (arg) : 100; break;
        case 'k': arguments->k = arg ? atoi (arg) : -1; break;
        case 'f': arguments->f = arg ? atoi (arg) : 12; break;
        case ARGP_KEY_ARG: return 0;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

struct arguments* parse_cli_arguments(int argc, char** argv){
    struct arguments *arguments;
    arguments = malloc(sizeof(struct arguments));

    /* Default values. */
    arguments->n = 1000000;
    arguments->a = 0;
    arguments->b = 100;
    arguments->k = -1;
    arguments->f = 12;

    /* Parse our arguments; every option seen by parse_opt will be
       reflected in arguments. */
    argp_parse (&argp, argc, argv, 0, 0, arguments);

    return arguments;
}
