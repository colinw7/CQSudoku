#ifndef CQSudoku_H
#define CQSudoku_H

#include <CSudoku.h>
#include <QWidget>
#include <QPixmap>
#include <QPushButton>
#include <QLineEdit>
#include <QStatusBar>

class CQSudokuApp;
class CQSudokuLineEdit;

class CQSudokuCanvas : public QWidget {
  Q_OBJECT

 public:
  CQSudokuCanvas(CQSudokuApp *app, QWidget *parent = NULL);

  virtual ~CQSudokuCanvas() { }

 protected:
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent (QMouseEvent *) override;
  void paintEvent     (QPaintEvent *) override;
  void resizeEvent    (QResizeEvent *) override;

  void forceDraw();
  void draw();

 private slots:
  void showValues(int value);
  void showSolution(int value);
  void step();
  void solve();
  void print();
  void reset();
  void newGame();
  void editCellChanged();

 public:
  bool xyToCell(int x, int y, int *i, int *j) const;
  bool cellToXY(int i, int j, int *x, int *y) const;

  bool getCellSize(int *size) const;

 private:
  CQSudokuApp *app_;
  QPixmap     pixmap_;
  bool        show_values_;
  bool        show_solution_;
  bool        force_redraw_;
};

class CQSudokuApp {
 public:
  CQSudokuApp();

  void setEditCell(int i, int j);
  void clearEditCell();

  void showMessage(const QString &msg);

  void step();
  void solve();
  void print();

  void reset();
  void newGame();
  void loadGame(const char *str);

  void editCellChanged();

  uint getCellValue(int x, int y) const {
    return sudoku_.getValue(x, y);
  }

  bool isSolveCellValue(int x, int y, int v) const {
    return sudoku_.isSolveValue(x, y, v);
  }

  uint getSolvedCellValue(int x, int y) const {
    return sudoku_.getSolvedValue(x, y);
  }

  uint getSolutionCellValue(int x, int y) const {
    return sudoku_.getSolutionValue(x, y);
  }

  bool getIsInitUnknown(int x, int y) const {
    return sudoku_.getIsInitUnknown(x, y);
  }

 private:
  CSudoku           sudoku_;
  CQSudokuCanvas   *canvas_;
  CQSudokuLineEdit *line_edit_;
  int               edit_x_, edit_y_;
  QStatusBar       *status_bar_;
};

class CQSudokuLineEdit : public QLineEdit {
 public:
  CQSudokuLineEdit(QWidget *parent);

  void setNumber(int n, int x, int y, int size);

 private:
  void keyPressedEvent(QKeyEvent *e);
};

#endif
