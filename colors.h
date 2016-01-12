/*
 * ============================================================================
 *
 *       Filename:  colors.h
 *
 *    Description:  Just a small header with preprocessor directives containing
 *                  ANSI color strings for the color capable Linux terminals
 *                  and terminal emulators. It also contains the bold string.
 *
 *        Version:  1.0
 *        Created:  10/01/2016 09:36:33
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

#ifndef _COLORS_H_
#define _COLORS_H_

// Colors
#define ANSI_COLOR_RED              "\x1b[31m"
#define ANSI_COLOR_GREEN            "\x1b[32m"
#define ANSI_COLOR_YELLOW           "\x1b[33m"
#define ANSI_COLOR_BLUE             "\x1b[34m"
#define ANSI_COLOR_MAGENTA          "\x1b[35m"
#define ANSI_COLOR_CYAN             "\x1b[36m"
#define ANSI_COLOR_RESET            "\x1b[0m"

// Styles
#define ANSI_STYLE_UNDERLINE        "\033[4m"
#define ANSI_STYLE_NO_UNDERLINE     "\033[24m"
#define ANSI_STYLE_BOLD             "\033[1m"
#define ANSI_STYLE_NO_BOLD          "\033[22m"

#endif
