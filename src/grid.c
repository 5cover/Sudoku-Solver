/** @file
 * @brief Sudoku grid functions
 * @author 5cover, Matteo-K
 */

#pragma once

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "const.c"
#include "memdbg.c"
#include "tCell.c"
#include "utils.c"

// Using macros to maximize the performance of these simple functions called very frequently in the program.

#define grid_size(grid) ((grid).N * (grid).N)

#define grid_cellAt(grid, row, column) \
    (grid).cells[at2d(grid_size(grid), (row), (column))]
#define grid_cellAtPos(grid, pos) grid_cellAt(grid, pos.row, pos.column)
/// @brief Defines whether a value is free or not at a position on the grid.
#define grid_markValueFree(isFree, grid, row, column, value)                       \
    do {                                                                           \
        assert((row) < grid_size(grid));                                               \
        assert((column) < grid_size(grid));                                            \
        assert((value) <= grid_size(grid));                                            \
        (grid)._isColumnFree[at2d(grid_size(grid) + 1, (column), (value))] = (isFree); \
        (grid)._isRowFree[at2d(grid_size(grid) + 1, (row), (value))] = (isFree);       \
        (grid)._isBlockFree[at3d((grid).N, grid_size(grid) + 1, (row) / (grid).N,      \
            (column) / (grid).N, (value))]                                         \
            = (isFree);                                                            \
    } while (0)

/// @brief Gets the axis index (a row or column number) of the start of the
/// block containing the given index.
/// @param index in: an axis index (row or column number)
/// @return The axis index of the start of the block containing the given index.
#define grid_blockIndex(grid, index) (index - (index % (grid).N))

/// @brief Check if a value can be added to the grid. Does not take candidates
/// into account.
/// @param grid in: the grid
/// @param row in: the cell's row
/// @param column in: the cell's column
/// @param value in: value to check
/// @return Whether the value can be added to the grid at the given position.
/// @remark According to the rules of Sudoku, a value can only be added to the
/// grid when it is not already present in the row, in the column and in the
/// block.
// After running tests, it seems that the fastest access order is column, row,
// block. This solutions is considerably faster than the naive alternative
// (iterating over cells) But it comes at a price : we must make sure that the
// state of candidates in the grid and the "_is*Free" arrays are synchronized
// from the start of the resolution to the backtracking call. For this we use
// the grid_markValueFree macro
#define grid_possible(grid, row, column, value) \
    ((grid)._isColumnFree[at2d(grid_size(grid) + 1, (column), (value))] && (grid)._isRowFree[at2d(grid_size(grid) + 1, (row), (value))] && (grid)._isBlockFree[at3d((grid).N, grid_size(grid) + 1, (row) / (grid).N, (column) / (grid).N, (value))])

/// @brief Counts the number of possible values for a cell.
/// @param grid in: the grid
/// @param row in: the cell's row
/// @param column in: the cell's column
/// @param outVarName Name of the variable to declare and assign the result to.
/// @return The amount of values for which @ref grid_possible returns @c true.
#define grid_cellPossibleValuesCount(grid, row, column, outVarName) \
    tIntSize outVarName = 0;                                        \
    for (tIntSize val = 1; val <= grid_size(grid); val++) {             \
        outVarName += grid_possible((grid), (row), (column), val);  \
    }

tGrid grid_create(tIntN const N);

/// @brief Loads a grid from a file in the Sud format.
/// @param inStream in: the file to read
/// @param N in: grid size factor
/// @param grid out: the loaded grid.
/// @return 0 if everything went well, or @ref ERROR_INVALID_DATA if the file
/// contains invalid data.
int grid_load(FILE *inStream, tGrid *g);

/// @brief Writes a grid to a file in the Sud format.
/// @param grid in: the grid to write
/// @param outStream in: the file to write to
void grid_write(tGrid const *grid, FILE *outStream);

/// @brief Frees a grid.
/// @param grid in/out: the grid to free
void grid_free(tGrid *grid);

/// @brief Removes a candidate from a cell of the grid and sets it as the cell's
/// value if it's the last one.
/// @param grid in/out: the grid
/// @param row in: the cell's row
/// @param column in: the cell's column
/// @param candidate in: the candidate to remove
/// @return Whether the candidate has been removed.
bool grid_cell_removeCandidate(tGrid *grid, tIntSize row, tIntSize column,
    tIntSize candidate);

/// @brief Defines the value of a cell and removes all its candidates.
/// @param grid in/out: the grid
/// @param row in: the cell's row
/// @param column in: the cell's column
/// @param candidate in: the value to provide to the cell
void grid_cell_provideValue(tGrid *grid, tIntSize row, tIntSize column,
    tIntSize value);

/// @brief Removes a candidate from all cells of a row.
/// @param grid in/out: the grid
/// @param row in: the row
/// @param candidate in: the candidate to remove
/// @return Whether progress has been made.
bool grid_removeCandidateFromRow(tGrid *grid, tIntSize row, tIntSize candidate);

/// @brief Removes a candidate from all cells of a column.
/// @param grid in/out: the grid
/// @param column in: the column
/// @param candidate in: the candidate to remove
/// @return Whether progress has been made.
bool grid_removeCandidateFromColumn(tGrid *grid, tIntSize column,
    tIntSize candidate);

/// @brief Removes a candidate from all cells of a block.
/// @param grid in/out: the grid
/// @param row in: the row of a cell in the block
/// @param column in: the column of a cell in the block
/// @param candidate in: the candidate to remove
/// @return Whether progress has been made.
bool grid_removeCandidateFromBlock(tGrid *grid, tIntSize row, tIntSize column,
    tIntSize candidate);

// Grid display functions

/// @brief Prints a grid.
/// @param grid in: the grid to print
/// @param outStream in: the file to write to
void grid_print(tGrid const *grid, FILE *outStream);

/// @brief Prints a block separation line.
/// @param grid in: the grid
/// @param padding in: value padding
/// @param outStream in: the file to write to
void printBlockSeparationLine(tGrid const *grid, int padding, FILE *outStream);

/// @brief Prints a grid row.
/// @param grid in: @ref tGrid : the grid
/// @param row in: index of the row to print (between 0 and @ref SIZE - 1)
/// @param padding in: value padding
/// @param outStream in: the file to write to
void grid_printRow(tGrid const *grid, tIntSize row, int padding,
    FILE *outStream);

/// @brief Prints a grid value as a number or @ref DISPLAY_EMPTY_VALUE for empty
/// values.
/// @param value in: the value to print
/// @param padding in: value padding
/// @param outStream in: the file to write to
void printValue(tIntSize value, int padding, FILE *outStream);

/// @brief Prints a character a specific amount of times.
/// @param character in: the character to print
/// @param times in: amount of times
/// @param outStream in: the file to write to
void printMultipleTimes(char character, unsigned times, FILE *outStream);

////////////////////////////////////////////////////////////////////////////

tGrid grid_create(tIntN const N) {
    return (tGrid) {
        .N = N,
        .cells = NULL,
        ._isBlockFree = NULL,
        ._isColumnFree = NULL,
        ._isRowFree = NULL,
    };
}

int grid_load(FILE *inStream, tGrid *g) {
#define fail_invalid_data()        \
    do {                           \
        free(gridValues);          \
        return ERROR_INVALID_DATA; \
    } while (0);

    // As the .sud files only contain the grid values, we need a temporary integer
    // grid to store them.
    uint32_t *gridValues = check_alloc(array2d_malloc(gridValues, grid_size(*g), grid_size(*g)), "gridValues");

    if (fread(gridValues, sizeof *gridValues, grid_size(*g) * grid_size(*g), inStream) != grid_size(*g) * grid_size(*g)) fail_invalid_data();

    // Allocate and initialize all cells to 0 (candidates array is NULL, no value,
    // 0 candidates)
    g->cells = check_alloc(array2d_calloc(g->cells, grid_size(*g), grid_size(*g)),
        "grid cells array");

    // Allocate row, column and block arrays
    g->_isColumnFree = check_alloc(array2d_malloc(g->_isColumnFree, grid_size(*g), grid_size(*g) + 1),
        "grid _isColumnFree array");
    g->_isRowFree = check_alloc(array2d_malloc(g->_isRowFree, grid_size(*g), grid_size(*g) + 1),
        "grid _isRowFree array");
    g->_isBlockFree = check_alloc(array3d_malloc(g->_isBlockFree, g->N, g->N, grid_size(*g) + 1),
        "grid _isBlockFree array");

    // Initialize all rows, columns and blocks to free
    memset(g->_isRowFree, true, sizeof(bool) * grid_size(*g) * (grid_size(*g) + 1));
    memset(g->_isColumnFree, true, sizeof(bool) * grid_size(*g) * (grid_size(*g) + 1));
    memset(g->_isBlockFree, true, sizeof(bool) * g->N * g->N * (grid_size(*g) + 1));

    // Initialize cells and mark them as not free
    for (tIntSize r = 0; r < grid_size(*g); r++) {
        for (tIntSize c = 0; c < grid_size(*g); c++) {
            tIntSize value = gridValues[at2d(grid_size(*g), r, c)];
            tCell *cell = &grid_cellAt(*g, r, c);

            cell->hasCandidate = check_alloc(array_calloc(cell->hasCandidate, grid_size(*g) + 1),
                "grid cell %d,%d hasCandidate array", r, c);

            if (value != 0) {
                if (value > grid_size(*g)) fail_invalid_data();
                cell->_value = value;
                grid_markValueFree(false, *g, r, c, value);
            }
        }
    }

    // Add candidates
    for (tIntSize r = 0; r < grid_size(*g); r++) {
        for (tIntSize c = 0; c < grid_size(*g); c++) {
            tCell *cell = &grid_cellAt(*g, r, c);
            // No need to compute the candidates of a cell that already has a value.
            if (!cell_hasValue(*cell)) {
                // compute the cell's candidates
                for (tIntSize candidate = 1; candidate <= grid_size(*g); candidate++) {
                    bool possible = grid_possible(*g, r, c, candidate);
                    // add the candidate
                    cell->hasCandidate[candidate] = possible;
                    cell->_candidateCount += possible;
                }
            }
        }
    }

    free(gridValues);
    return 0;
}

void grid_free(tGrid *grid) {
    for (tIntSize r = 0; r < grid_size(*grid); r++) {
        for (tIntSize c = 0; c < grid_size(*grid); c++) {
            free(grid_cellAt(*grid, r, c).hasCandidate);
        }
    }
    free(grid->cells);
    free(grid->_isBlockFree);
    free(grid->_isColumnFree);
    free(grid->_isRowFree);
}

bool grid_cell_removeCandidate(tGrid *grid, tIntSize row, tIntSize column,
    tIntSize candidate) {
    tCell *cell = &grid_cellAt(*grid, row, column);

    assert(1 <= candidate && candidate <= grid_size(*grid));

    // If the cell has only one candidate remaining and it's the one we want to
    // remove, set it as the cell's value and remove it.
    if (cell_candidate_count(*cell) == 1) {
        cell_get_first_candidate(*cell, onlyCandidate);
        if (onlyCandidate == candidate) {
            cell->_value = onlyCandidate;
            cell->hasCandidate[candidate] = false;
            cell->_candidateCount = 0;
            grid_markValueFree(false, *grid, row, column, candidate);
            return true;
        }
    }

    // Otherwise proceed as usual
    bool possible = cell_hasCandidate(*cell, candidate);
    if (possible) {
        cell->hasCandidate[candidate] = false;
        cell->_candidateCount--;
    }

    return possible;
}

void grid_cell_provideValue(tGrid *grid, tIntSize row, tIntSize column,
    tIntSize value) {
    assert(grid_possible(*grid, row, column, value));

    tCell *cell = &grid_cellAt(*grid, row, column);

    assert(1 <= value && value <= grid_size(*grid));
    assert(!cell_hasValue(*cell));

    cell->_value = value;
    cell->_candidateCount = 0;
    memset(cell->hasCandidate, false, sizeof(bool) * grid_size(*grid) + 1);
    grid_markValueFree(false, *grid, row, column, value);
}

bool grid_removeCandidateFromRow(tGrid *grid, tIntSize row,
    tIntSize candidate) {
    bool progress = false;
    for (tIntSize c = 0; c < grid_size(*grid); c++) {
        progress |= grid_cell_removeCandidate(grid, row, c, candidate);
    }
    return progress;
}

bool grid_removeCandidateFromColumn(tGrid *grid, tIntSize column,
    tIntSize candidate) {
    bool progress = false;
    for (tIntSize r = 0; r < grid_size(*grid); r++) {
        progress |= grid_cell_removeCandidate(grid, r, column, candidate);
    }
    return progress;
}

bool grid_removeCandidateFromBlock(tGrid *grid, tIntSize row, tIntSize column,
    tIntSize candidate) {
    bool progress = false;

    tIntSize const blockRow = grid_blockIndex(*grid, row);
    tIntSize const blockCol = grid_blockIndex(*grid, column);

    for (tIntSize r = blockRow; r < blockRow + grid->N; r++) {
        for (tIntSize c = blockCol; c < blockCol + grid->N; c++) {
            progress |= grid_cell_removeCandidate(grid, r, c, candidate);
        }
    }

    return progress;
}

void grid_write(tGrid const *grid, FILE *outStream) {
    for (tIntSize r = 0; r < grid_size(*grid); r++) {
        for (tIntSize c = 0; c < grid_size(*grid); c++) {
            uint32_t value32 = grid_cellAt(*grid, r, c)._value;
            fwrite(&value32, sizeof(uint32_t), 1, outStream);
        }
    }
}

void grid_print(tGrid const *grid, FILE *outStream) {
    // Print grid body
    int padding = digitCount(grid_size(*grid), 10);

    for (tIntSize block = 0; block < grid->N; block++) {
        printBlockSeparationLine(grid, padding, outStream);

        // For eahc line in the block:
        for (tIntSize blockRow = 0; blockRow < grid->N; blockRow++) {
            // Find the actual line and print it
            grid_printRow(grid, block * grid->N + blockRow, padding, outStream);
        }
    }

    // Print the last separation line all the way down
    printBlockSeparationLine(grid, padding, outStream);
}

void printBlockSeparationLine(tGrid const *grid, int padding, FILE *outStream) {
    putc(DISPLAY_INTERSECTION, outStream);

    for (tIntSize block = 0; block < grid->N; block++) {
        // Add 2 to account for horizontal value margin (1 space left and right)
        printMultipleTimes(DISPLAY_HORIZONTAL_LINE, grid->N * (padding + 2),
            outStream);
        putc(DISPLAY_INTERSECTION, outStream);
    }

    putc('\n', outStream);
}

void grid_printRow(tGrid const *grid, tIntSize row, int padding,
    FILE *outStream) {
    putc(DISPLAY_VERTICAL_LINE, outStream);

    // Print line content
    for (tIntSize block = 0; block < grid->N; block++) {
        for (tIntSize blockCol = 0; blockCol < grid->N; blockCol++) {
            printValue(grid_cellAt(*grid, row, block * grid->N + blockCol)._value,
                padding, outStream);
        }
        putc(DISPLAY_VERTICAL_LINE, outStream);
    }

    putc('\n', outStream);
}

void printValue(tIntSize value, int padding, FILE *outStream) {
    putc(DISPLAY_SPACE, outStream);

    if (value == 0) {
        fprintf(outStream, "%*c", padding, DISPLAY_EMPTY_VALUE);
    } else {
        fprintf(outStream, "%*d", padding, value);
    }

    putc(DISPLAY_SPACE, outStream);
}

void printMultipleTimes(char character, unsigned times, FILE *outStream) {
    for (unsigned time = 0; time < times; time++) {
        putc(character, outStream);
    }
}
