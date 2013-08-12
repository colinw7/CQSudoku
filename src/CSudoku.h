#ifndef CSUDOKU_H
#define CSUDOKU_H

#include <string>
#include <vector>
#include <set>

#include <cassert>
#include <sys/types.h>

class CSudoku {
 public:
  enum { SIZE = 9         };
  enum { CELL_SIZE = 3    };
  enum { AREA = SIZE*SIZE };

 private:
  // class representing the set of possible for a cell (1-9)
  class ValueSet {
   public:
    ValueSet() { reset(); }

    void set  (uint i) { values_[i] = true; }
    void clear(uint i) { values_[i] = false; }

    bool get(uint i) const { return values_[i]; }

    void reset() {
      values_[0] = false;

      for (uint i = 1; i <= SIZE; ++i)
        values_[i] = true;
    }

    uint getNumValues() const {
      uint count = 0;

      for (uint i = 1; i <= SIZE; ++i)
        if (values_[i])
          ++count;

      return count;
    }

    std::vector<uint> getValues() const {
      std::vector<uint> v;

      for (uint i = 1; i <= SIZE; ++i)
        if (values_[i])
          v.push_back(i);

      return v;
    }

    uint getUniqueValue() const {
      uint value = 0;

      for (uint i = 1; i <= SIZE; ++i) {
        if (! values_[i]) continue;

        if (value != 0)
          return 0;

        value = i;
      }

      return value;
    }

    bool removeValues(const ValueSet &values) {
      bool changed = false;

      for (uint i = 1; i <= SIZE; ++i) {
        if (! values.values_[i] || ! values_[i]) continue;

        values_[i] = false;

        changed = true;
      }

      return changed;
    }

    bool contains(const ValueSet &values) const {
      for (uint i = 1; i <= SIZE; ++i)
        if (! values.values_[i] && values.values_[i] != values_[i])
          return false;

      return true;
    }

    friend bool operator==(const ValueSet &values1, const ValueSet &values2) {
      for (uint i = 1; i <= SIZE; ++i)
        if (values1.values_[i] != values2.values_[i])
          return false;

      return true;
    }

   private:
    bool values_[SIZE + 1]; // indexed by number
  };

  //------

  // class representing a single cell on the board
  class Cell {
   public:
    Cell() :
     i_(0), j_(0), init_value_(0), solution_value_(0) {
      resetValue();
    }

    void setPos(uint i, uint j) { i_ = i; j_ = j; }

    uint getValue() const { return value_; }

    bool isUnknown() const { return (value_ == 0); }

    void setValue(uint value) {
      assert(value >= 1 && value <= SIZE);

      value_ = value;

      for (uint i = 1; i <= SIZE; ++i)
        clearSolveValue(i);

      setSolveValue(value_);
    }

    void resetValue() {
      value_ = 0;

      resetSolvedValues();
    }

    //------

    uint getInitValue() const { return init_value_; }

    void setInitValue(uint value) {
      assert(value >= 1 && value <= SIZE);

      init_value_ = value;
    }

    void resetInitValue() {
      init_value_ = 0;
    }

    bool isInitUnknown() const { return (init_value_ == 0); }

    //------

    void setSolveValue(uint value) {
      assert(value >= 1 && value <= SIZE);

      values_.set(value);
    }

    void clearSolveValue(uint value) {
      assert(value >= 1 && value <= SIZE);

      values_.clear(value);
    }

    bool isSolveValue(uint value) const {
      assert(value >= 1 && value <= SIZE);

      return values_.get(value);
    }

    void resetSolvedValues() { values_.reset(); }

    uint getSolvedValue() const { return values_.getUniqueValue(); }

    void initSolveValue() {
      if (! isUnknown()) {
        for (uint i = 1; i <= SIZE; ++i)
          clearSolveValue(i);

        setSolveValue(value_);
      }
      else
        resetSolvedValues();
    }

    std::vector<uint> getSolveValues() const { return values_.getValues(); }

    const ValueSet &getSolveValueSet() const { return values_; }

    uint getNumSolvedValues() const { return values_.getNumValues(); }

    bool removeSolvedValues(const ValueSet &values) { return values_.removeValues(values); }

    //------

    uint getSolutionValue() const { return solution_value_; }

    void setSolutionValue(uint value) {
      assert(value >= 1 && value <= SIZE);

      solution_value_ = value;
    }

    void resetSolutionValue() {
      solution_value_ = 0;
    }

   private:
    uint     i_, j_;
    uint     value_;
    uint     init_value_;
    uint     solution_value_;
    ValueSet values_;
  };

 public:
  struct Values {
    uint values[AREA];
  };

 public:
  class CellIterator {
   public:
    CellIterator(CSudoku *sudoku=NULL) :
     sudoku_(sudoku), i_(0), j_(0), k_(0), end_(sudoku_==NULL) {
    }

    CellIterator &operator++() {
      ++k_;
      ++j_;

      if (j_ >= SIZE) {
        j_ = 0;

        ++i_;

        if (i_ >= SIZE)
          end_ = true;
      }

      return *this;
    }

    Cell &operator*() {
      assert(! end_);

      return sudoku_->getCell(i_, j_);
    }

    const Cell &operator*() const {
      assert(! end_);

      return sudoku_->getCell(i_, j_);
    }

    friend bool operator==(const CellIterator &i1, const CellIterator &i2) {
      if (i1.end_ != i2.end_) return false;

      if (i1.end_) return true;

      return (i1.i_ == i2.i_ && i1.j_ == i2.j_);
    }

    friend bool operator!=(const CellIterator &i1, const CellIterator &i2) {
      return ! (i1 == i2);
    }

    uint i() const { return i_; }
    uint j() const { return j_; }
    uint k() const { return k_; }

   private:
    CSudoku *sudoku_;
    uint     i_, j_, k_;
    bool     end_;
  };

  class RowIterator {
   public:
    RowIterator(CSudoku *sudoku=NULL, uint i=0) :
     sudoku_(sudoku), i_(i), j_(0), end_(sudoku_==NULL) {
    }

    RowIterator &operator++() {
      ++j_;

      if (j_ >= SIZE)
        end_ = true;

      return *this;
    }

    Cell &operator*() {
      assert(! end_);

      return sudoku_->getCell(i_, j_);
    }

    const Cell &operator*() const {
      assert(! end_);

      return sudoku_->getCell(i_, j_);
    }

    friend bool operator==(const RowIterator &i1, const RowIterator &i2) {
      if (i1.end_ != i2.end_) return false;

      if (i1.end_) return true;

      return (i1.i_ == i2.i_ && i1.j_ == i2.j_);
    }

    friend bool operator!=(const RowIterator &i1, const RowIterator &i2) {
      return ! (i1 == i2);
    }

    uint j() const { return j_; }

   private:
    CSudoku *sudoku_;
    uint     i_, j_;
    bool     end_;
  };

  class ColIterator {
   public:
    ColIterator(CSudoku *sudoku=NULL, uint j=0) :
     sudoku_(sudoku), i_(0), j_(j), end_(sudoku_==NULL) {
    }

    ColIterator &operator++() {
      ++i_;

      if (i_ >= SIZE)
        end_ = true;

      return *this;
    }

    Cell &operator*() {
      assert(! end_);

      return sudoku_->getCell(i_, j_);
    }

    const Cell &operator*() const {
      assert(! end_);

      return sudoku_->getCell(i_, j_);
    }

    friend bool operator==(const ColIterator &i1, const ColIterator &i2) {
      if (i1.end_ != i2.end_) return false;

      if (i1.end_) return true;

      return (i1.i_ == i2.i_ && i1.j_ == i2.j_);
    }

    friend bool operator!=(const ColIterator &i1, const ColIterator &i2) {
      return ! (i1 == i2);
    }

    uint i() const { return i_; }

   private:
    CSudoku *sudoku_;
    uint     i_, j_;
    bool     end_;
  };

  class BlockIterator {
   public:
    BlockIterator(CSudoku *sudoku=NULL, uint k=0) :
     sudoku_(sudoku), end_(sudoku_==NULL) {
      i1_ = CELL_SIZE*(k % CELL_SIZE);
      i2_ = i1_ + CELL_SIZE - 1;
      j1_ = CELL_SIZE*(k / CELL_SIZE);
      j2_ = j1_ + CELL_SIZE - 1;

      i_ = i1_;
      j_ = j1_;
    }

    BlockIterator(CSudoku *sudoku, uint i, uint j) :
     sudoku_(sudoku), end_(sudoku_==NULL) {
      i1_ = CELL_SIZE*(i / CELL_SIZE);
      i2_ = i1_ + CELL_SIZE - 1;
      j1_ = CELL_SIZE*(j / CELL_SIZE);
      j2_ = j1_ + CELL_SIZE - 1;

      i_ = i1_;
      j_ = j1_;
    }

    BlockIterator &operator++() {
      ++i_;

      if (i_ > i2_) {
        i_ = i1_;

        ++j_;

        if (j_ > j2_)
          end_ = true;
      }

      return *this;
    }

    Cell &operator*() {
      assert(! end_);

      return sudoku_->getCell(i_, j_);
    }

    const Cell &operator*() const {
      assert(! end_);

      return sudoku_->getCell(i_, j_);
    }

    friend bool operator==(const BlockIterator &i1, const BlockIterator &i2) {
      if (i1.end_ != i2.end_) return false;

      if (i1.end_) return true;

      return (i1.i_ == i2.i_ && i1.j_ == i2.j_);
    }

    friend bool operator!=(const BlockIterator &i1, const BlockIterator &i2) {
      return ! (i1 == i2);
    }

    uint i() const { return i_; }
    uint j() const { return j_; }

   private:
    CSudoku *sudoku_;
    uint     i_, j_;
    uint     i1_, i2_, j1_, j2_;
    bool     end_;
  };

 public:
  CSudoku();
 ~CSudoku() { }

  //-------

  const Cell &getCell(uint i) const { return getCell(i / SIZE, i % SIZE); }

  Cell &getCell(uint i) { return getCell(i / SIZE, i % SIZE); }

  const Cell &getCell(uint x, uint y) const { return cells_[x][y]; }

  Cell &getCell(uint x, uint y) { return cells_[x][y]; }

  CellIterator beginCells() { return CellIterator(this); }
  CellIterator endCells  () { return CellIterator(NULL); }

  RowIterator beginRow(uint i) { return RowIterator(this, i); }
  RowIterator endRow  ()       { return RowIterator(NULL); }

  ColIterator beginCol(uint j) { return ColIterator(this, j); }
  ColIterator endCol  ()      { return ColIterator(NULL); }

  BlockIterator beginBlock(uint k        ) { return BlockIterator(this, k); }
  BlockIterator beginBlock(uint i, uint j) { return BlockIterator(this, i, j); }
  BlockIterator endBlock  ()             { return BlockIterator(NULL); }

  //-------

  uint getValue(uint x, uint y) const;
  void setValue(uint x, uint y, uint value);

  bool isSolveValue(uint x, uint y, uint v) const;

  uint getSolvedValue(uint x, uint y) const;

  uint getSolutionValue(uint x, uint y) const;

  bool getIsInitUnknown(uint x, uint y) const;

  bool getValid() const { return valid_; }

  bool solvable();

 public:
  bool solve();
  bool solveStep();
  bool iterSolve();
  bool addSolvedValues();
  bool checkSolvedValues();

  bool checkUniqueValues();
  bool checkUniqueRow(Cell &cell, uint i, uint j);
  bool checkUniqueCol(Cell &cell, uint i, uint j);
  bool checkUniqueCell(Cell &cell, uint i, uint j);

  bool checkTwinValues();
  bool checkTwinRow(Cell &cell, uint i, uint j);
  bool checkTwinCol(Cell &cell, uint i, uint j);
  bool checkTwinCell(Cell &cell, uint i, uint j);

  bool checkTripleValues();
  bool checkTripleRow(Cell &cell, uint i, uint j);
  bool checkTripleCol(Cell &cell, uint i, uint j);
  bool checkTripleCell(Cell &cell, uint i, uint j);

  bool checkValid();
  void newGame();
  void reset();
  void loadGame(const char *str);
  bool isSolved();

  void print();

 private:
  bool genValues(Values &values);

  void setInitValues();

  void init(const Values &values);

  bool initSolution();

  bool initSolve();
  void initSolveValues();
  bool solve1();
  bool solveCell(uint i);
  bool solveRow(uint i);
  bool solveCol(uint i);

  void saveState();
  void restoreState();

  void log(const std::string &msg) const;

  static std::string intToString(int i);

 private:
  Cell                cells_[SIZE][SIZE];
  bool                valid_;
  bool                log_;
  std::vector<Values> saved_state_;
};

#endif
