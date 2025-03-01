/** @file
 * @brief Solver functions
 * @author 5cover, Matteo-K
 */

#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "grid.c"
#include "types.c"

/// @brief Performs the simple techniques on the grid.
/// @param grid in/out: the grid
/// @return Whether progress has been made.
bool perform_simpleTechniques(tGrid *grid);

/// @brief Performs the backtracking technique.
/// @param grid in/out: the grid
/// @param emptyCellPositions in/out: the empty cell positions
/// @param emptyCellCount in: the amount of empty cells (length of @p
/// emptyCellPositions)
/// @param iEmptyCellPositions in: index of the empty cell to start the
/// backtracking from
/// @return Whether progress has been made.
/// @remark This technique must be performed last, as it will always solve the
/// grid completely.
/// @remark After calling this function, it is possible that the candidates of
/// the grid have an inconsistent state. This choice was made because it offers
/// a performance gain and we no longer need the candidates once the grid is
/// solved.
bool technique_backtracking(tGrid *grid, tPosition *emptyCellPositions,
    tIntSize emptyCellCount,
    tIntSize iEmptyCellPosition);

/// @brief Swaps the cell at @p iHere with the cell after @p iHere having the
/// least possible values in @p emptyCellPositions.
/// @param grid in: the grid
/// @param emptyCellPositions in/out: the empty cell positions
/// @param emptyCellCount in: the amount of empty cells (length of @p
/// emptyCellPositions)
/// @param iHere in: the index of the cell to swap
/// @remark Used in the backtracking technique.
void technique_backtracking_swap_cells(tGrid const *grid,
    tPosition *emptyCellPositions,
    tIntSize emptyCellCount, tIntSize iHere);

/// @brief Performs the naked singleton technique.
/// @param grid in/out: the grid
/// @param row in: the row of the targeted cell
/// @param column in: the column of the targeted cell
/// @return Whether progress has been made.
bool technique_nakedSingleton(tGrid *grid, tIntSize row, tIntSize column);

/// @brief Performs the hidden singleton technique.
/// @param grid in/out: the grid
/// @param row in: the row of the targeted cell
/// @param column in: the column of the targeted cell
/// @return Whether progress has been made.
bool technique_hiddenSingleton(tGrid *grid, tIntSize row, tIntSize column);

/// @brief Finds the unique candidate in a group.
/// @param grid in: the grid
/// @param rStart in: search start row
/// @param rEnd in: search end row (excluded)
/// @param cStart in: search start column
/// @param cEnd in: search end column (excluded)
/// @param candidatePosition out: assigned to the position of the unique
/// candidate found
/// @return The unique candidate found, or 0 if none was found.
/// @remark Used in the hidden singleton technique.
int technique_hiddenSingleton_findUniqueCandidate(
    tGrid const *grid, tIntSize rStart, tIntSize rEnd, tIntSize cStart,
    tIntSize cEnd, tPosition *candidatePosition);

/// @brief Performs the naked pair technique.
/// @param grid in/out: the grid
/// @param row in: the row of the targeted cell
/// @param column in: the column of the targeted cell
/// @return Whether progress has been made.
bool technique_nakedPair(tGrid *grid, tIntSize row, tIntSize column);

/// @brief Checks if a cell is a naked pair cell (it only contains the
/// candidates of @p pair).
/// @param grid in: the grid
/// @param cell in: the cell to check
/// @return Whether the cell is a naked pair cell
/// @remark Used in the naked pair technique.
#define technique_nakedPair_isPairCell(cell, pair) \
    (cell_candidate_count(cell) == 2 && cell_hasCandidate(cell, pair.candidates[0]) && cell_hasCandidate(cell, pair.candidates[1]))

/// @brief Performs the hidden pair technique.
/// @param grid in/out: the grid
/// @param row in: the row of the targeted cell
/// @param column in: the column of the targeted cell
/// @return Whether progress has been made.
bool technique_hiddenPair(tGrid *grid, tIntSize row, tIntSize column);

/// @brief Finds a pair present exactly twice in a group.
/// @param grid in: the grid
/// @param rStart in: search start row
/// @param rEnd in: search end row (excluded)
/// @param cStart in: search start column
/// @param cEnd in: search end column (excluded)
/// @param pairCellPositions in/out: positions of the cells containing the found
/// pair. The first element must contain the position of the first cell
/// containing the pair.
/// @param candidates out: filled with the pair's candididates
/// @return Whether a pair has been found.
/// @remark Used in the hidden singleton technique.
bool technique_hiddenPair_findPair(tGrid const *grid, tIntSize rStart,
    tIntSize rEnd, tIntSize cStart,
    tIntSize cEnd,
    tPosition pairCellPositions[PAIR_SIZE],
    tIntSize candidates[PAIR_SIZE]);

/// @brief Removes the cells containing a pair of candidates.
/// @param grid in/out: the grid
/// @param pairCellPositions in: positions of the pair cells
/// @param candidates in: the pair's candidates
/// @return Whether progress has been made.
bool technique_hiddenPair_removePairCells(
    tGrid *grid, tPosition const pairCellPositions[PAIR_SIZE],
    tIntSize const candidates[PAIR_SIZE]);

/// @brief Finds the cells containing a pair of candidates.
/// @param grid in: the grid
/// @param candidates in: the pair's candidates
/// @param rStart in: search start row
/// @param rEnd in: search end row (excluded)
/// @param cStart in: search start column
/// @param cEnd in: search end column (excluded)
/// @param pairCellPositions out: assigned to the positions of the cells
/// containing the pair
/// @return Whether a pair has been found.
bool technique_hiddenPair_findPairCells(tGrid const *grid,
    tIntSize const candidates[PAIR_SIZE],
    tIntSize rStart, tIntSize rEnd,
    tIntSize cStart, tIntSize cEnd,
    tPosition pairCellPositions[PAIR_SIZE]);

/// @brief Performs the X-Wing technique
/// @param grid in/out: the grid
/// @return Whether progress has been made.
bool technique_x_wing(tGrid *grid);

/////////////////////////////////////////////////////////////////

bool perform_simpleTechniques(tGrid *grid) {
    bool progress = false;
    tCell *cell;

    for (tIntSize r = 0; r < grid_size(*grid); r++) {
        for (tIntSize c = 0; c < grid_size(*grid); c++) {
            // Executing the techniques in order of increasing complexity.
            // As soon as the value of the cell is defined, we move on to the next
            // one.

            // Save time by avoiding to recalculate the address of the cell each time.
            cell = &grid_cellAt(*grid, r, c);

            if (cell_hasValue(*cell))
                continue;

            progress |= technique_nakedSingleton(grid, r, c);
            if (cell_hasValue(*cell))
                continue;

            progress |= technique_hiddenSingleton(grid, r, c);
            if (cell_hasValue(*cell))
                continue;

            progress |= technique_nakedPair(grid, r, c);
            if (cell_hasValue(*cell))
                continue;

            progress |= technique_hiddenPair(grid, r, c);
        }
    }

    return progress;
}

bool technique_backtracking(tGrid *grid, tPosition *emptyCellPositions,
    tIntSize emptyCellCount, tIntSize iCellPosition) {
    // This technique does not use candidates but value presence arrays.
    // The reason is that synchronizing the candidates between recursive calls
    // requires loops. While for the value arrays it is a simple boolean that
    // indicates whether a value is present in a group (row, block or column).

    // we have processed all the cells, the grid is solved
    if (iCellPosition == emptyCellCount) {
        return true;
    }

    assert(iCellPosition < emptyCellCount);

    // Select the cell to solve
    technique_backtracking_swap_cells(grid, emptyCellPositions, emptyCellCount,
        iCellPosition);

    tPosition pos = emptyCellPositions[iCellPosition];

    for (tIntSize value = 1; value <= grid_size(*grid); value++) {
        if (grid_possible(*grid, pos.row, pos.column, value)) {
            // assuming that the cell contains this value,
            grid_markValueFree(false, *grid, pos.row, pos.column, value);

            // move on to the next cell: recursive call to see if the value is good
            // afterwards
            if (technique_backtracking(grid, emptyCellPositions, emptyCellCount,
                    iCellPosition + 1)) {
                // the value is good, put it and return.
                grid_cellAtPos(*grid, pos)._value = value;
                return true;
            }

            // Solving the following cells assuming this value has failed, so we don't
            // have the right value.
            grid_markValueFree(true, *grid, pos.row, pos.column, value);
        }
    }

    // We failed for all values
    return false;
}

void technique_backtracking_swap_cells(tGrid const *grid,
    tPosition *emptyCellPositions,
    tIntSize emptyCellCount,
    tIntSize iHere) {
    assert(iHere < emptyCellCount);

    tPosition pos = emptyCellPositions[iHere];
    tIntSize iMin = iHere;
    grid_cellPossibleValuesCount(*grid, pos.row, pos.column, possibleValCountMin);

    // find the cell after which has the least possible values
    for (tIntSize i = iHere + 1; i < emptyCellCount; i++) {
        pos = emptyCellPositions[i];
        grid_cellPossibleValuesCount(*grid, pos.row, pos.column, possibleValCountI);

        if (possibleValCountI < possibleValCountMin) {
            iMin = i;
            possibleValCountMin = possibleValCountI;
        }
    }

    // swap the cells
    tPosition tmp = emptyCellPositions[iHere];
    emptyCellPositions[iHere] = emptyCellPositions[iMin];
    emptyCellPositions[iMin] = tmp;
}

bool technique_nakedSingleton(tGrid *grid, tIntSize row, tIntSize column) {
    bool progress = false;

    tCell *cell = &grid_cellAt(*grid, row, column);
    if (cell_candidate_count(*cell) == 1) {
        cell_get_first_candidate(*cell, candidate);
        // remove all corresponding candidates
        // there will be at least one removal, the unique candidate of the cell.
        progress |= grid_removeCandidateFromRow(grid, row, candidate);
        progress |= grid_removeCandidateFromColumn(grid, column, candidate);
        progress |= grid_removeCandidateFromBlock(grid, row, column, candidate);
    }

    return progress;
}

bool technique_hiddenSingleton(tGrid *grid, tIntSize row, tIntSize column) {
    tPosition candidatePos;
    int candidate;
    bool progress = false;

    int const blockRow = grid_blockIndex(*grid, row);
    int const blockCol = grid_blockIndex(*grid, column);

    // Block
    if ((candidate = technique_hiddenSingleton_findUniqueCandidate(
             grid, blockRow, blockRow + grid->N, blockCol, blockCol + grid->N,
             &candidatePos))
        != 0) {
        grid_cell_provideValue(grid, candidatePos.row, candidatePos.column,
            candidate);
        grid_removeCandidateFromRow(grid, candidatePos.row, candidate);
        grid_removeCandidateFromColumn(grid, candidatePos.column, candidate);
        progress = true;
    }
    // Row
    if ((candidate = technique_hiddenSingleton_findUniqueCandidate(
             grid, row, row + 1, 0, grid_size(*grid), &candidatePos))
        != 0) {
        grid_cell_provideValue(grid, candidatePos.row, candidatePos.column,
            candidate);
        grid_removeCandidateFromBlock(grid, candidatePos.row, candidatePos.column,
            candidate);
        grid_removeCandidateFromColumn(grid, candidatePos.column, candidate);
        progress = true;
    }
    // chaud: the two ones above fail
    // chaud: this one succeeds

    // Column
    if ((candidate = technique_hiddenSingleton_findUniqueCandidate(
             grid, 0, grid_size(*grid), column, column + 1, &candidatePos))
        != 0) {
        grid_cell_provideValue(grid, candidatePos.row, candidatePos.column,
            candidate);
        grid_removeCandidateFromBlock(grid, candidatePos.row, candidatePos.column,
            candidate);
        grid_removeCandidateFromRow(grid, candidatePos.row, candidate);
        progress = true;
    }

    return progress;
}

int technique_hiddenSingleton_findUniqueCandidate(
    tGrid const *grid, tIntSize rStart, tIntSize rEnd, tIntSize cStart,
    tIntSize cEnd, tPosition *candidatePosition) {
    tIntSize *candidateCounts = check_alloc(
        array_calloc(candidateCounts, grid_size(*grid) + 1), "candidate counts array");

    for (tIntSize r = rStart; r < rEnd; r++) {
        for (tIntSize c = cStart; c < cEnd; c++) {
            tCell cell = grid_cellAt(*grid, r, c);
            for (tIntSize candidate = 1; candidate <= grid_size(*grid); candidate++) {
                candidateCounts[candidate] += cell_hasCandidate(cell, candidate);
            }
        }
    }

    // Search for the unique candidate
    tIntSize candidate = 1;
    while (candidate <= grid_size(*grid) && candidateCounts[candidate] != 1) {
        candidate++;
    }

    free(candidateCounts);

    // If it hasn't been found
    if (candidate == grid_size(*grid) + 1) {
        return 0;
    }

    for (tIntSize r = rStart; r < rEnd; r++) {
        for (tIntSize c = cStart; c < cEnd; c++) {
            if (cell_hasCandidate(grid_cellAt(*grid, r, c), candidate)) {
                *candidatePosition = (tPosition) { .row = r, .column = c };
                return candidate;
            }
        }
    }

    dbg_fail("Unreachable code bug : unique candidate not found even though it "
             "was found earlier");
}

bool technique_nakedPair(tGrid *grid, tIntSize row, tIntSize column) {
    bool progress = false;

    tCell *cellRowColumn = &grid_cellAt(*grid, row, column);

    if (cell_candidate_count(*cellRowColumn) == 2) {
        tIntSize const blockRow = grid_blockIndex(*grid, row);
        tIntSize const blockCol = grid_blockIndex(*grid, column);

        tPair2 pair = (tPair2) {
            .candidates = { cell_candidateAt(cellRowColumn, 1),
                cell_candidateAt(cellRowColumn, 2) },
            .count = 1,
        };

        for (tIntSize r = blockRow; r < blockRow + grid->N && pair.count < 2; r++) {
            for (tIntSize c = blockCol; c < blockCol + grid->N && pair.count < 2;
                c++) {
                pair.count += (r != row || c != column) && technique_nakedPair_isPairCell(grid_cellAt(*grid, r, c), pair);
            }
        }

        if (pair.count == 2) {
            // Remove all candidates from the block except on the cells containing
            // only the candidates of the pair So we cannot use
            // grid_removeCandidateFromBlock
            for (tIntSize r = blockRow; r < blockRow + grid->N; r++) {
                for (tIntSize c = blockCol; c < blockCol + grid->N; c++) {
                    bool isNotPairCell = !technique_nakedPair_isPairCell(grid_cellAt(*grid, r, c), pair);
                    progress |= isNotPairCell && grid_cell_removeCandidate(grid, r, c, pair.candidates[0]);
                    progress |= isNotPairCell && grid_cell_removeCandidate(grid, r, c, pair.candidates[1]);
                }
            }
        }
    }

    return progress;
}

bool technique_hiddenPair(tGrid *grid, tIntSize row, tIntSize column) {
    tPosition pairCellPositions[PAIR_SIZE] = {
        (tPosition) { .row = row, .column = column }
    };
    tIntSize candidates[PAIR_SIZE];
    bool progress = false;

    tIntSize const blockRow = grid_blockIndex(*grid, row);
    tIntSize const blockCol = grid_blockIndex(*grid, column);

    tCell *cellRowColumn = &grid_cellAt(*grid, row, column);

    // Block
    progress |= (cell_candidate_count(*cellRowColumn) >= 2 && technique_hiddenPair_findPair(grid, blockRow, blockRow + grid->N, blockCol, blockCol + grid->N, pairCellPositions, candidates)) && technique_hiddenPair_removePairCells(grid, pairCellPositions, candidates);
    // Row
    progress |= (cell_candidate_count(*cellRowColumn) >= 2 && technique_hiddenPair_findPair(grid, row, row + 1, 0, grid_size(*grid), pairCellPositions, candidates)) && technique_hiddenPair_removePairCells(grid, pairCellPositions, candidates);
    // Column
    progress |= (cell_candidate_count(*cellRowColumn) >= 2 && technique_hiddenPair_findPair(grid, 0, grid_size(*grid), column, column + 1, pairCellPositions, candidates)) && technique_hiddenPair_removePairCells(grid, pairCellPositions, candidates);

    return progress;
}

bool technique_hiddenPair_findPair(tGrid const *grid, tIntSize rStart,
    tIntSize rEnd, tIntSize cStart,
    tIntSize cEnd,
    tPosition pairCellPositions[PAIR_SIZE],
    tIntSize candidates[PAIR_SIZE]) {
    tCell firstPairCell = grid_cellAtPos(*grid, pairCellPositions[0]);

    assert(cell_candidate_count(firstPairCell) >= 2);

    for (candidates[0] = 1; candidates[0] <= grid_size(*grid); candidates[0]++) {
        if (!cell_hasCandidate(firstPairCell, candidates[0])) {
            continue;
        }

        for (candidates[1] = candidates[0] + 1; candidates[1] <= grid_size(*grid);
            candidates[1]++) {
            if (!cell_hasCandidate(firstPairCell, candidates[1])) {
                continue;
            }

            // Start the search for a pair with new candidates.
            if (technique_hiddenPair_findPairCells(grid, candidates, rStart, rEnd,
                    cStart, cEnd, pairCellPositions)) {
                return true;
            }
        }
    }

    return false;
}

bool technique_hiddenPair_findPairCells(
    tGrid const *grid, tIntSize const candidates[PAIR_SIZE], tIntSize rStart,
    tIntSize rEnd, tIntSize cStart, tIntSize cEnd,
    tPosition pairCellPositions[PAIR_SIZE]) {
    tIntSize nbPairCells = 1; // Count of cells found containing the pair (candidate1, candidate2).
    tIntSize nbPairCellsContainingOtherCandidates = 0;

    // Searching for hidden pairs
    // Add each cell that contain both candidates
    for (tIntSize r = rStart; r < rEnd && nbPairCells <= PAIR_SIZE + 1; r++) {
        for (tIntSize c = cStart; c < cEnd && nbPairCells <= PAIR_SIZE + 1; c++) {
            if (r == pairCellPositions[0].row && c == pairCellPositions[0].column) {
                continue;
            }

            tCell cell = grid_cellAt(*grid, r, c);

            if (cell_hasCandidate(cell, candidates[0]) && cell_hasCandidate(cell, candidates[1])) {
                nbPairCellsContainingOtherCandidates += cell_candidate_count(cell) > PAIR_SIZE;
                if (nbPairCells < PAIR_SIZE) {
                    pairCellPositions[nbPairCells] = (tPosition) { .row = r, .column = c };
                }
                nbPairCells++;
            }
            // search for single candidates that would invalidate the pair
            else if (cell_hasCandidate(cell, candidates[0]) || cell_hasCandidate(cell, candidates[1])) {
                return false;
            }
        }
    }

    // For the hidden pair to be useful, at least one cell of the pair must
    // contain other candidates. Otherwise, we won't be able to remove any
    // candidates.
    return nbPairCells == PAIR_SIZE && nbPairCellsContainingOtherCandidates > 0;
}

bool technique_hiddenPair_removePairCells(
    tGrid *grid, tPosition const pairCellPositions[PAIR_SIZE],
    tIntSize const candidates[PAIR_SIZE]) {
    bool progress = false;
    // For each cell containing the pair:
    for (tIntSize iPos = 0; iPos < PAIR_SIZE; ++iPos) {
        tPosition pos = pairCellPositions[iPos];
        // remove all its candidates
        for (tIntSize candidate = 1; candidate <= grid_size(*grid); ++candidate) {
            // except those forming the pair
            progress |= candidate != candidates[0] && candidate != candidates[1] && grid_cell_removeCandidate(grid, pos.row, pos.column, candidate);
        }
    }
    return progress;
}

bool technique_x_wing(tGrid *grid) {
    bool progress = false;

    // VERTICAL X-WING
    // identify 2 columns that each have, on the same rows, exactly two cells
    // where a candidate appears.
    /* We get a rectangle with these four cells as vertices:
    _   _
    A---B
    |   |
    C---D
    ‾   ‾*/
    for (tIntSize colAC = 0; colAC < grid_size(*grid); colAC++) {
        for (tIntSize colBD = colAC + 1; colBD < grid_size(*grid); colBD++) {
            for (tIntSize candidate = 1; candidate <= grid_size(*grid); candidate++) {
                tIntSize rows[2];
                tIntSize candidateInBothCount = 0;
                tIntSize candidateCounts[2] = { 0 };
                for (tIntSize row = 0; row < grid_size(*grid); row++) {
                    bool colACHasCandidate = cell_hasCandidate(grid_cellAt(*grid, row, colAC), candidate);
                    bool colBDHasCandidate = cell_hasCandidate(grid_cellAt(*grid, row, colBD), candidate);
                    candidateCounts[0] += colACHasCandidate;
                    candidateCounts[1] += colBDHasCandidate;
                    if (colACHasCandidate && colBDHasCandidate) {
                        if (candidateInBothCount < 2) {
                            rows[candidateInBothCount] = row;
                        }
                        candidateInBothCount++;
                    }
                }
                if (candidateInBothCount == 2 && candidateCounts[0] == 2 && candidateCounts[1] == 2) {
                    // eliminate the candidate from the rows (AB) and (CD).
                    for (tIntSize c = 0; c < grid_size(*grid); c++) {
                        // do not remove the candidate from the cells that form the X.
                        progress |= c != colAC && c != colBD && grid_cell_removeCandidate(grid, rows[0], c, candidate);
                        progress |= c != colAC && c != colBD && grid_cell_removeCandidate(grid, rows[1], c, candidate);
                    }
                }
            }
        }
    }

    // HORIZONTAL X-WING
    // identify 2 rows that each have, on the same columns, exactly two cells
    // where a candidate appears.
    /* We get a rectangle with these four cells as vertices:
    (A---B)
     |   |
    (C---D)*/
    for (tIntSize rowAB = 0; rowAB < grid_size(*grid); rowAB++) {
        for (tIntSize rowCD = rowAB + 1; rowCD < grid_size(*grid); rowCD++) {
            for (tIntSize candidate = 1; candidate <= grid_size(*grid); candidate++) {
                tIntSize columns[2];
                tIntSize candidateInBothCount = 0;
                tIntSize candidateCounts[2] = { 0 };
                for (tIntSize col = 0; col < grid_size(*grid); col++) {
                    bool rowABHasCandidate = cell_hasCandidate(grid_cellAt(*grid, rowAB, col), candidate);
                    bool rowCDHasCandidate = cell_hasCandidate(grid_cellAt(*grid, rowCD, col), candidate);
                    candidateCounts[0] += rowABHasCandidate;
                    candidateCounts[1] += rowCDHasCandidate;
                    if (rowABHasCandidate && rowCDHasCandidate) {
                        if (candidateInBothCount < 2) {
                            columns[candidateInBothCount] = col;
                        }
                        candidateInBothCount++;
                    }
                }
                if (candidateInBothCount == 2 && candidateCounts[0] == 2 && candidateCounts[1] == 2) {
                    // elmininate the candidate from the columns (AC) and (BD).
                    for (tIntSize r = 0; r < grid_size(*grid); r++) {
                        // do not remove the candidate from the cells that form the X.
                        progress |= r != rowAB && r != rowCD && grid_cell_removeCandidate(grid, r, columns[0], candidate);
                        progress |= r != rowAB && r != rowCD && grid_cell_removeCandidate(grid, r, columns[1], candidate);
                    }
                }
            }
        }
    }

    return progress;
}
