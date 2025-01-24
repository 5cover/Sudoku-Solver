/** @file
 * @brief Symolic constants
 * @author 5cover, Matteo-K
 */

#pragma once

#define PROGRAM_NAME "sudone"

/// @brief Integer: error code for invalid data in a file
#define ERROR_INVALID_DATA (-1)

/// @brief Integer: exit code for invalid arguments
#define EXIT_INVALID_ARG 1
/// @brief Integer: exit code for invalid data in a file
#define EXIT_INVALID_DATA 2

/// @brief Integer: size of a pair of candidates
#define PAIR_SIZE 2

/// @brief Defines that the memory debugger should give verbose output.
// #define MEMDBG_VERBOSE

/* Display character macros */

/// @brief Character: grid display space
#define DISPLAY_SPACE ' '
/// @brief Character: grid display empty value
#define DISPLAY_EMPTY_VALUE '.'
/// @brief Character: grid display intersection
#define DISPLAY_INTERSECTION '+'
/// @brief Character: grid display vertical line
#define DISPLAY_VERTICAL_LINE '|'
/// @brief Character: grid display horizontal line
#define DISPLAY_HORIZONTAL_LINE '-'
