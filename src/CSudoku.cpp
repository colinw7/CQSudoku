#include <CSudoku.h>

#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iostream>

CSudoku::Values values1 = {{
  0, 2, 9,  0, 8, 5,  0, 0, 7,
  0, 8, 5,  0, 7, 0,  2, 3, 9,
  7, 0, 0,  9, 3, 0,  5, 0, 0,

  0, 3, 0,  0, 0, 9,  4, 0, 0,
  0, 0, 0,  0, 0, 0,  0, 0, 0,
  0, 0, 6,  4, 0, 0,  0, 1, 0,

  0, 0, 8,  0, 4, 7,  0, 0, 1,
  5, 6, 1,  0, 9, 0,  3, 7, 0,
  4, 0, 0,  5, 1, 0,  8, 9, 0,
}};

CSudoku::Values values2 = {{
  0, 6, 0,  0, 0, 9,  0, 8, 0,
  0, 0, 0,  0, 0, 8,  0, 6, 3,
  0, 4, 8,  3, 5, 0,  0, 0, 0,

  0, 9, 0,  0, 0, 0,  0, 0, 0,
  6, 0, 0,  8, 0, 3,  0, 0, 1,
  0, 0, 0,  0, 0, 0,  0, 7, 0,

  0, 0, 0,  0, 6, 2,  9, 1, 0,
  8, 2, 0,  9, 0, 0,  0, 0, 0,
  0, 7, 0,  4, 0, 0,  0, 3, 0,
}};

CSudoku::
CSudoku() :
 valid_(true), log_(false)
{
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    cell.setPos(pc1.i(), pc1.j());
  }
}

uint
CSudoku::
getValue(uint x, uint y) const
{
  return getCell(x, y).getValue();
}

void
CSudoku::
setValue(uint x, uint y, uint value)
{
  getCell(x, y).setValue(value);

  valid_ = checkValid();
}

bool
CSudoku::
isSolveValue(uint x, uint y, uint v) const
{
  return getCell(x, y).isSolveValue(v);
}

uint
CSudoku::
getSolvedValue(uint x, uint y) const
{
  return getCell(x, y).getSolvedValue();
}

uint
CSudoku::
getSolutionValue(uint x, uint y) const
{
  return getCell(x, y).getSolutionValue();
}

bool
CSudoku::
getIsInitUnknown(uint x, uint y) const
{
  return getCell(x, y).isInitUnknown();
}

void
CSudoku::
newGame()
{
  Values values;

  if (genValues(values))
    init(values);
  else
    init(values1);

  setInitValues();

  initSolution();
}

void
CSudoku::
loadGame(const char *str)
{
  Values values;

  uint len = strlen(str);

  for (uint i = 0; i < AREA; ++i) {
    if (i < len && str[i] >= '1' && str[i] <= '9')
      values.values[i] = str[i] - '0';
    else
      values.values[i] = 0;
  }

  init(values);

  setInitValues();

  initSolution();
}

void
CSudoku::
reset()
{
  Values values;

  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    const Cell &cell = *pc1;

    values.values[pc1.k()] = (! cell.isInitUnknown() ? cell.getInitValue() : 0);
  }

  init(values);

  initSolution();
}

void
CSudoku::
setInitValues()
{
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (! cell.isUnknown())
      cell.setInitValue(cell.getValue());
    else
      cell.resetInitValue();
  }
}

void
CSudoku::
print()
{
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    const Cell &cell = *pc1;

    uint value = cell.getValue();

    std::cout << char(value + '0');
  }

  std::cout << std::endl;
}

bool
CSudoku::
genValues(Values &values)
{
  enum { NUM_ITERATIONS = 50   };
  enum { MAX_TRIES      = 1000 };

  // reset values
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    cell.resetValue();

    values.values[pc1.k()] = 0;
  }

  uint n = 0;
  uint i = 0;

  while (n < NUM_ITERATIONS) {
    // get random value and position
    uint value = (rand() % SIZE) + 1;
    uint pos   = (rand() % AREA);

    // ensure position is unknown
    while (values.values[pos] != 0)
      pos = (rand() % AREA);

    // set to value
    values.values[pos] = value;

    Cell &cell = getCell(pos);

    cell.setValue(value);

    // check if valid (if not reset and try again)
    if (! checkValid()) {
      values.values[pos] = 0;

      cell.resetValue();
    }
    else
      ++n;

    // ensure we don't loop forever
    ++i;

    if (i > MAX_TRIES)
      break;
  }

  //------

  // set grid to calculated values
  init(values);

  // remove cells until solvable
  i = 0;

  while (! solvable()) {
    // count number of solved cells
    uint count = 0;

    for (uint i = 0; i < AREA; ++i)
      if (values.values[i] != 0)
        ++count;

    // too few values so give up
    if (count < SIZE)
      return false;

    // reset random cell to unknown (0) and try again
    uint pos = (rand() % AREA);

    while (values.values[pos] == 0)
      pos = (rand() % AREA);

    values.values[pos] = 0;

    init(values);
  }

  return true;
}

void
CSudoku::
init(const Values &values)
{
  // set cells from array of values
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (values.values[pc1.k()])
      cell.setValue(values.values[pc1.k()]);
    else
      cell.resetValue();
  }

  valid_ = checkValid();
}

bool
CSudoku::
solvable()
{
  return initSolution();
}

bool
CSudoku::
initSolution()
{
  // save values
  saveState();

  // solve
  bool solved = true;

  while (! isSolved()) {
    if (! solve1()) {
      solved = false;

      break;
    }
  }

  // store solved values
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    uint value = cell.getValue();

    if (value)
      cell.setSolutionValue(value);
    else
      cell.resetSolutionValue();
  }

  // restore values
  restoreState();

  return solved;
}

bool
CSudoku::
solveStep()
{
  log_ = true;

  bool rc = solve1();

  log_ = false;

  return rc;
}

bool
CSudoku::
solve()
{
  log_ = true;

  while (! isSolved())
    if (! solve1()) {
      break;
  }

  log_ = false;

  return isSolved();
}

bool
CSudoku::
solve1()
{
  if (isSolved())
    return true;

  while (true) {
    // remove twin values
    bool rc1 = checkTwinValues();

    // remove triple values
    bool rc2 = checkTripleValues();

    if (! rc1 && ! rc2)
      break;
  }

  // check single solutions
  if (addSolvedValues()) {
    // update unknown cells solutions and ensure valid
    valid_ = checkValid();

    return valid_;
  }

  // check unique values
  if (checkUniqueValues()) {
    // update unknown cells solutions and ensure valid
    valid_ = checkValid();

    return valid_;
  }

  //if (log_) return false;

  // no single solution so try all values
  log("Iterate solution");

  if (iterSolve())
    return true;

  return false;
}

bool
CSudoku::
iterSolve()
{
  if (isSolved())
    return true;

  uint min_num = SIZE;
  uint min_k   = 0;

  // find cell with least num possible values
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (cell.isUnknown()) {
      uint num = cell.getNumSolvedValues();

      if (num < min_num) {
         min_num = num;
         min_k   = pc1.k();
      }
    }
  }

  // Try each value in turn
  bool solved = false;

  Cell &min_cell = getCell(min_k);

  std::vector<uint> solve_values = min_cell.getSolveValues();

  uint num_solve_values = solve_values.size();

  for (uint i = 0; i < num_solve_values; ++i) {
    uint value = solve_values[i];

    saveState();

    min_cell.setValue(value);

    bool valid = checkValid();

    if (valid)
      solved = iterSolve();
    else
      solved = false;

    restoreState();

    if (solved) {
      min_cell.setValue(value);

      valid_ = checkValid();

      break;
    }
  }

  return solved;
}

bool
CSudoku::
addSolvedValues()
{
  // check all unknown cells for single solution
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (cell.isUnknown()) {
      uint new_value = cell.getSolvedValue();

      if (new_value != 0) {
        // update value with solution
        cell.setValue(new_value);

        log("Single Value " + intToString(new_value) +
            " for Cell (" + intToString(pc1.i()) + "," + intToString(pc1.j()) + ") ");

        return true;
      }
    }
  }

  return false;
}

bool
CSudoku::
checkSolvedValues()
{
  // check if any unknown cell has single solution
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (cell.isUnknown()) {
      uint new_value = cell.getSolvedValue();

      if (new_value != 0)
        return true;
    }
  }

  return false;
}

bool
CSudoku::
checkUniqueValues()
{
  // TODO: collect all values for each row, col, cell update cell by associated row/col/cell

  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (! cell.isUnknown()) continue;

    if (checkUniqueRow (cell, pc1.i(), pc1.j())) return true;
    if (checkUniqueCol (cell, pc1.i(), pc1.j())) return true;
    if (checkUniqueCell(cell, pc1.i(), pc1.j())) return true;
  }

  return false;
}

bool
CSudoku::
checkUniqueRow(Cell &cell, uint i, uint j)
{
  ValueSet values = cell.getSolveValueSet();

  RowIterator pr1, pr2;

  for (pr1 = beginRow(i), pr2 = endRow(); pr1 != pr2; ++pr1) {
    if (j == pr1.j()) continue;

    Cell &cell1 = *pr1;

    if (! cell1.isUnknown()) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    values.removeValues(values1);
  }

  uint new_value = values.getUniqueValue();

  if (new_value != 0) {
    cell.setValue(new_value);

    log("Unique Value " + intToString(new_value) +
        " for Cell (" + intToString(i) + "," + intToString(j) + ") ");

    return true;
  }

  return false;
}

bool
CSudoku::
checkUniqueCol(Cell &cell, uint i, uint j)
{
  ValueSet values = cell.getSolveValueSet();

  ColIterator pc1, pc2;

  for (pc1 = beginCol(j), pc2 = endCol(); pc1 != pc2; ++pc1) {
    if (i == pc1.i()) continue;

    Cell &cell1 = *pc1;

    if (! cell1.isUnknown()) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    values.removeValues(values1);
  }

  uint new_value = values.getUniqueValue();

  if (new_value != 0) {
    cell.setValue(new_value);

    log("Unique Value " + intToString(new_value) +
        " for Cell (" + intToString(i) + "," + intToString(j) + ") ");

    return true;
  }

  return false;
}

bool
CSudoku::
checkUniqueCell(Cell &cell, uint i, uint j)
{
  ValueSet values = cell.getSolveValueSet();

  BlockIterator pb1, pb2;

  for (pb1 = beginBlock(i, j), pb2 = endBlock(); pb1 != pb2; ++pb1) {
    if (pb1.i() == i && pb1.j() == j) continue;

    Cell &cell1 = *pb1;

    if (! cell1.isUnknown()) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    values.removeValues(values1);
  }

  uint new_value = values.getUniqueValue();

  if (new_value != 0) {
    cell.setValue(new_value);

    log("Unique Value " + intToString(new_value) +
        " for Cell (" + intToString(i) + "," + intToString(j) + ") ");

    return true;
  }

  return false;
}

bool
CSudoku::
checkTwinValues()
{
  bool changed = false;

  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (! cell.isUnknown()) continue;

    if (cell.getNumSolvedValues() != 2) continue;

    if (checkTwinRow (cell, pc1.i(), pc1.j())) changed = true;
    if (checkTwinCol (cell, pc1.i(), pc1.j())) changed = true;
    if (checkTwinCell(cell, pc1.i(), pc1.j())) changed = true;
  }

  return changed;
}

bool
CSudoku::
checkTwinRow(Cell &cell, uint i, uint j)
{
  uint num_twins   = 1;
  uint num_unknown = 1;

  int tj = -1;

  const ValueSet &values = cell.getSolveValueSet();

  RowIterator pr1, pr2;

  for (pr1 = beginRow(i), pr2 = endRow(); pr1 != pr2; ++pr1) {
    if (j == pr1.j()) continue;

    Cell &cell1 = *pr1;

    if (! cell1.isUnknown()) continue;

    ++num_unknown;

    if (cell1.getNumSolvedValues() != 2) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    if (values == values1) {
      ++num_twins; tj = pr1.j();
    }
  }

  bool changed = false;

  if (num_twins == 2 && num_unknown > 2) {
    for (pr1 = beginRow(i), pr2 = endRow(); pr1 != pr2; ++pr1) {
      if (int(pr1.j()) == j || int(pr1.j()) == tj) continue;

      Cell &cell1 = *pr1;

      if (! cell1.isUnknown()) continue;

      if (cell1.removeSolvedValues(values))
        changed = true;
    }
  }

  if (changed)
    log("Twin Value for Row : Cell (" + intToString(i) + "," + intToString(j) + ") ");

  return changed;
}

bool
CSudoku::
checkTwinCol(Cell &cell, uint i, uint j)
{
  uint num_twins  = 1;
  uint num_unknown = 1;

  int ti = -1;

  const ValueSet &values = cell.getSolveValueSet();

  ColIterator pc1, pc2;

  for (pc1 = beginCol(j), pc2 = endCol(); pc1 != pc2; ++pc1) {
    if (i == pc1.i()) continue;

    Cell &cell1 = *pc1;

    if (! cell1.isUnknown()) continue;

    ++num_unknown;

    if (cell1.getNumSolvedValues() != 2) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    if (values == values1) {
      ++num_twins; ti = pc1.i();
    }
  }

  bool changed = false;

  if (num_twins == 2 && num_unknown > 2) {
    for (pc1 = beginCol(j), pc2 = endCol(); pc1 != pc2; ++pc1) {
      if (int(pc1.i()) == i || int(pc1.i()) == ti) continue;

      Cell &cell1 = *pc1;

      if (! cell1.isUnknown()) continue;

      if (cell1.removeSolvedValues(values))
        changed = true;
    }
  }

  if (changed)
    log("Twin Value for Col : Cell (" + intToString(i) + "," + intToString(j) + ") ");

  return changed;
}

bool
CSudoku::
checkTwinCell(Cell &cell, uint i, uint j)
{
  uint num_twins   = 1;
  uint num_unknown = 1;

  int ti = -1, tj = -1;

  const ValueSet &values = cell.getSolveValueSet();

  BlockIterator pb1, pb2;

  for (pb1 = beginBlock(i, j), pb2 = endBlock(); pb1 != pb2; ++pb1) {
    if (pb1.i() == i && pb1.j() == j) continue;

    Cell &cell1 = *pb1;

    if (! cell1.isUnknown()) continue;

    ++num_unknown;

    if (cell1.getNumSolvedValues() != 2) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    if (values == values1) {
      ++num_twins; ti = pb1.i(); tj = pb1.j();
    }
  }

  bool changed = false;

  if (num_twins == 2 && num_unknown > 2) {
    for (pb1 = beginBlock(i, j), pb2 = endBlock(); pb1 != pb2; ++pb1) {
      if ((int(pb1.i()) == i  && int(pb1.j()) == j ) ||
          (int(pb1.i()) == ti && int(pb1.j()) == tj)) continue;

      Cell &cell1 = *pb1;

      if (! cell1.isUnknown()) continue;

      if (cell1.removeSolvedValues(values))
        changed = true;
    }
  }

  if (changed)
    log("Twin Value for Cell : Cell (" + intToString(i) + "," + intToString(j) + ") ");

  return changed;
}

bool
CSudoku::
checkTripleValues()
{
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (! cell.isUnknown()) continue;

    if (cell.getNumSolvedValues() != 3) continue;

    if (checkTripleRow (cell, pc1.i(), pc1.j())) return true;
    if (checkTripleCol (cell, pc1.i(), pc1.j())) return true;
    if (checkTripleCell(cell, pc1.i(), pc1.j())) return true;
  }

  return false;
}

bool
CSudoku::
checkTripleRow(Cell &cell, uint i, uint j)
{
  uint num_triples = 1;
  uint num_unknown = 1;

  int tj1 = -1, tj2 = -1;

  const ValueSet &values = cell.getSolveValueSet();

  RowIterator pr1, pr2;

  for (pr1 = beginRow(i), pr2 = endRow(); pr1 != pr2; ++pr1) {
    if (j == pr1.j()) continue;

    Cell &cell1 = *pr1;

    if (! cell1.isUnknown()) continue;

    ++num_unknown;

    uint num = cell1.getNumSolvedValues();

    if (num != 2 && num != 3) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    if ((num == 3 && values == values1) ||
        (num == 2 && values.contains(values1))) {
      ++num_triples;

      if      (tj1 < 0) tj1 = pr1.j();
      else if (tj2 < 0) tj2 = pr1.j();
    }
  }

  bool changed = false;

  if (num_triples == 3 && num_unknown > 3) {
    for (pr1 = beginRow(i), pr2 = endRow(); pr1 != pr2; ++pr1) {
      if (int(pr1.j()) == j || int(pr1.j()) == tj1 || int(pr1.j()) == tj2) continue;

      Cell &cell1 = *pr1;

      if (! cell1.isUnknown()) continue;

      if (cell1.removeSolvedValues(values))
        changed = true;
    }
  }

  if (changed)
    log("Triple Value for Row : Cell (" + intToString(i) + "," + intToString(j) + ") ");

  return changed;
}

bool
CSudoku::
checkTripleCol(Cell &cell, uint i, uint j)
{
  uint num_triples = 1;
  uint num_unknown = 1;

  int ti1 = -1, ti2 = -1;

  const ValueSet &values = cell.getSolveValueSet();

  ColIterator pc1, pc2;

  for (pc1 = beginCol(j), pc2 = endCol(); pc1 != pc2; ++pc1) {
    if (i == pc1.i()) continue;

    Cell &cell1 = *pc1;

    if (! cell1.isUnknown()) continue;

    ++num_unknown;

    uint num = cell1.getNumSolvedValues();

    if (num != 2 && num != 3) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    if ((num == 3 && values == values1) ||
        (num == 2 && values.contains(values1))) {
      ++num_triples;

      if      (ti1 < 0) ti1 = pc1.i();
      else if (ti2 < 0) ti2 = pc1.i();
    }
  }

  bool changed = false;

  if (num_triples == 3 && num_unknown > 3) {
    for (pc1 = beginCol(j), pc2 = endCol(); pc1 != pc2; ++pc1) {
      if (int(pc1.i()) == i || int(pc1.i()) == ti1 || int(pc1.i()) == ti2) continue;

      Cell &cell1 = *pc1;

      if (! cell1.isUnknown()) continue;

      if (cell1.removeSolvedValues(values))
        changed = true;
    }
  }

  if (changed)
   log("Triple Value for Col : Cell (" + intToString(i) + "," + intToString(j) + ") ");

  return changed;
}

bool
CSudoku::
checkTripleCell(Cell &cell, uint i, uint j)
{
  uint num_triples = 1;
  uint num_unknown = 1;

  int ti1 = -1, ti2 = -1, tj1 = -1, tj2 = -1;

  const ValueSet &values = cell.getSolveValueSet();

  BlockIterator pb1, pb2;

  for (pb1 = beginBlock(i, j), pb2 = endBlock(); pb1 != pb2; ++pb1) {
    if (pb1.i() == i && pb1.j() == j) continue;

    Cell &cell1 = *pb1;

    if (! cell1.isUnknown()) continue;

    ++num_unknown;

    uint num = cell1.getNumSolvedValues();

    if (num != 2 && num != 3) continue;

    const ValueSet &values1 = cell1.getSolveValueSet();

    if ((num == 3 && values == values1) ||
        (num == 2 && values.contains(values1))) {
      ++num_triples;

      if      (ti1 < 0) { ti1 = pb1.i(); tj1 = pb1.j(); }
      else if (ti2 < 0) { ti2 = pb1.i(); tj2 = pb1.j(); }
    }
  }

  bool changed = false;

  if (num_triples == 3 && num_unknown > 3) {
    for (pb1 = beginBlock(i, j), pb2 = endBlock(); pb1 != pb2; ++pb1) {
      if ((int(pb1.i()) == i   && int(pb1.j()) == j  ) ||
          (int(pb1.i()) == ti1 && int(pb1.j()) == tj1) ||
          (int(pb1.i()) == ti2 && int(pb1.j()) == tj2)) continue;

      Cell &cell1 = *pb1;

      if (! cell1.isUnknown()) continue;

      if (cell1.removeSolvedValues(values))
        changed = true;
    }
  }

  if (changed)
    log("Triple Value for Cell : Cell (" + intToString(i) + "," + intToString(j) + ") ");

  return changed;
}

bool
CSudoku::
checkValid()
{
  // reset unknown cells set of possible solutions
  initSolveValues();

  bool flag = true;

  // remove all values already used in cells, rows and cols
  // (fail if grid is invalid - no solution possible)
  for (uint i = 0; i < SIZE; ++i)
    if (! solveCell(i))
      flag = false;

  for (uint i = 0; i < SIZE; ++i)
    if (! solveRow(i))
      flag = false;

  for (uint i = 0; i < SIZE; ++i)
    if (! solveCol(i))
      flag = false;

  return flag;
}

void
CSudoku::
initSolveValues()
{
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    cell.initSolveValue();
  }
}

bool
CSudoku::
solveCell(uint k)
{
  ValueSet values;

  // get all used values for 3x3 grid
  BlockIterator pb1, pb2;

  for (pb1 = beginBlock(k), pb2 = endBlock(); pb1 != pb2; ++pb1) {
    Cell &cell = *pb1;

    if (cell.isUnknown()) continue;

    uint value = cell.getValue();

    if (! values.get(value))
      return false;

    values.clear(value);
  }

  // remove used values from grid cells
  for (pb1 = beginBlock(k), pb2 = endBlock(); pb1 != pb2; ++pb1) {
    Cell &cell = *pb1;

    if (! cell.isUnknown()) continue;

    for (uint k = 1; k <= SIZE; ++k) {
      if (! values.get(k))
        cell.clearSolveValue(k);
    }
  }

  return true;
}

bool
CSudoku::
solveRow(uint i)
{
  ValueSet values;

  // get all used values for row
  RowIterator pr1, pr2;

  for (pr1 = beginRow(i), pr2 = endRow(); pr1 != pr2; ++pr1) {
    Cell &cell = *pr1;

    if (cell.isUnknown()) continue;

    uint value = cell.getValue();

    if (! values.get(value))
      return false;

    values.clear(value);
  }

  // remove used values from row cells
  for (pr1 = beginRow(i), pr2 = endRow(); pr1 != pr2; ++pr1) {
    Cell &cell = *pr1;

    if (! cell.isUnknown()) continue;

    for (uint k = 1; k <= SIZE; ++k) {
      if (! values.get(k))
        cell.clearSolveValue(k);
    }
  }

  return true;
}

bool
CSudoku::
solveCol(uint j)
{
  // get all used values for col
  ValueSet values;

  ColIterator pc1, pc2;

  for (pc1 = beginCol(j), pc2 = endCol(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (cell.isUnknown()) continue;

    uint value = cell.getValue();

    if (! values.get(value))
      return false;

    values.clear(value);
  }

  // remove used values from col cells
  for (pc1 = beginCol(j), pc2 = endCol(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (! cell.isUnknown()) continue;

    for (uint k = 1; k <= SIZE; ++k) {
      if (! values.get(k))
        cell.clearSolveValue(k);
    }
  }

  return true;
}

bool
CSudoku::
isSolved()
{
  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (cell.isUnknown())
      return false;
  }

  return true;
}

void
CSudoku::
saveState()
{
  Values state;

  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    state.values[pc1.k()] = cell.getValue();
  }

  saved_state_.push_back(state);
}

void
CSudoku::
restoreState()
{
  assert(! saved_state_.empty());

  Values state = saved_state_.back();

  saved_state_.pop_back();

  CellIterator pc1, pc2;

  for (pc1 = beginCells(), pc2 = endCells(); pc1 != pc2; ++pc1) {
    Cell &cell = *pc1;

    if (state.values[pc1.k()])
      cell.setValue(state.values[pc1.k()]);
    else
      cell.resetValue();
  }

  valid_ = checkValid();
}

void
CSudoku::
log(const std::string &str) const
{
  if (log_)
    std::cerr << str << std::endl;
}

std::string
CSudoku::
intToString(int i)
{
  std::stringstream ss;

  ss << i;

  return ss.str();
}
