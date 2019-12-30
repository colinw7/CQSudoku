#include <CQSudoku.h>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QStatusBar>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>

#include <iostream>

int
main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  CQSudokuApp sudoku;

  if (argc > 1)
    sudoku.loadGame(argv[1]);
  else
    sudoku.newGame();

  return app.exec();
}

CQSudokuApp::
CQSudokuApp()
{
  QFrame *frame = new QFrame();

  QVBoxLayout *frame_layout   = new QVBoxLayout;
  QHBoxLayout *nframe_layout  = new QHBoxLayout;
  QVBoxLayout *neframe_layout = new QVBoxLayout;

  frame_layout  ->setMargin(0); frame_layout  ->setSpacing(0);
  nframe_layout ->setMargin(0); nframe_layout ->setSpacing(0);
  neframe_layout->setMargin(0); neframe_layout->setSpacing(0);

  frame->setLayout(frame_layout);

  //----

  status_bar_ = new QStatusBar();

  frame_layout->addLayout(nframe_layout, 1);
  frame_layout->addWidget(status_bar_);

  //----

  canvas_ = new CQSudokuCanvas(this);

  nframe_layout->addWidget(canvas_, 1);
  nframe_layout->addLayout(neframe_layout);

  //----

  QCheckBox *show_values = new QCheckBox("Show Values");

  canvas_->connect(show_values, SIGNAL(stateChanged(int)), SLOT(showValues(int)));

  QCheckBox *show_solution = new QCheckBox("Show Solution");

  canvas_->connect(show_solution, SIGNAL(stateChanged(int)), SLOT(showSolution(int)));

  QPushButton *step_button = new QPushButton("Step");

  canvas_->connect(step_button, SIGNAL(clicked()), SLOT(step()));

  QPushButton *solve_button = new QPushButton("Solve");

  canvas_->connect(solve_button, SIGNAL(clicked()), SLOT(solve()));

  QPushButton *print_button = new QPushButton("Print");

  canvas_->connect(print_button, SIGNAL(clicked()), SLOT(print()));

  QPushButton *reset_button = new QPushButton("Reset");

  canvas_->connect(reset_button, SIGNAL(clicked()), SLOT(reset()));

  QPushButton *new_game_button = new QPushButton("New Game");

  canvas_->connect(new_game_button, SIGNAL(clicked()), SLOT(newGame()));

  neframe_layout->addSpacing(8);
  neframe_layout->addWidget (show_values    );
  neframe_layout->addWidget (show_solution  );
  neframe_layout->addStretch(1);
  neframe_layout->addWidget (step_button    );
  neframe_layout->addSpacing(8);
  neframe_layout->addWidget (solve_button   );
  neframe_layout->addSpacing(8);
  neframe_layout->addWidget (print_button   );
  neframe_layout->addSpacing(8);
  neframe_layout->addWidget (reset_button   );
  neframe_layout->addSpacing(8);
  neframe_layout->addWidget (new_game_button);
  neframe_layout->addSpacing(8);

  //----

  line_edit_ = new CQSudokuLineEdit(canvas_);

  canvas_->connect(line_edit_, SIGNAL(returnPressed()), SLOT(editCellChanged()));

  line_edit_->hide();

  frame->resize(600, 500);

  frame->show();

  showMessage("Ready");
}

void
CQSudokuApp::
setEditCell(int i, int j)
{
  if (! sudoku_.getIsInitUnknown(i, j)) {
    clearEditCell();
    return;
  }

  edit_x_ = i;
  edit_y_ = j;

  QString message;

  message = "Cell:";
  message += (i + '1');
  message += " ";
  message += (j + '1');

  showMessage(message);

  //-----

  int x, y, size;

  canvas_->cellToXY(i, j, &x, &y);

  canvas_->getCellSize(&size);

  int n = sudoku_.getValue(i, j);

  line_edit_->setNumber(n, x, y, size);
}

void
CQSudokuApp::
clearEditCell()
{
  line_edit_->hide();
}

void
CQSudokuApp::
newGame()
{
  sudoku_.newGame();

  clearEditCell();

  showMessage("Ready");
}

void
CQSudokuApp::
reset()
{
  sudoku_.reset();

  clearEditCell();

  showMessage("Ready");
}

void
CQSudokuApp::
loadGame(const char *str)
{
  sudoku_.loadGame(str);

  clearEditCell();

  showMessage("Ready");
}

void
CQSudokuApp::
editCellChanged()
{
  QString str = line_edit_->text();

  int value = str[0].cell() - '0';

  sudoku_.setValue(edit_x_, edit_y_, value);

  clearEditCell();

  if (! sudoku_.getValid())
    showMessage("Invalid Solution");
}

void
CQSudokuApp::
showMessage(const QString &str)
{
  status_bar_->showMessage(str);
}

void
CQSudokuApp::
step()
{
  sudoku_.solveStep();

  if (sudoku_.isSolved())
    showMessage("Solved");
  else {
    if (! sudoku_.solvable())
      showMessage("Unsolvable");
    else
      showMessage("Ready");
  }
}

void
CQSudokuApp::
solve()
{
  sudoku_.solve();

  if (sudoku_.isSolved())
    showMessage("Solved");
  else {
    if (! sudoku_.solvable())
      showMessage("Unsolvable");
    else
      showMessage("Ready");
  }
}

void
CQSudokuApp::
print()
{
  sudoku_.print();
}

//-----

CQSudokuCanvas::
CQSudokuCanvas(CQSudokuApp *app, QWidget *parent) :
 QWidget(parent), app_(app), show_values_(false), show_solution_(false),
 force_redraw_(false)
{
  setWindowTitle(tr("Sudoku"));
}

void
CQSudokuCanvas::
mousePressEvent(QMouseEvent *event)
{
  int x = event->x();
  int y = event->y();

  int i, j;

  if (xyToCell(x, y, &i, &j))
    app_->setEditCell(i, j);
  else
    app_->clearEditCell();
}

void
CQSudokuCanvas::
mouseMoveEvent(QMouseEvent *)
{
}

void
CQSudokuCanvas::
paintEvent(QPaintEvent *)
{
  if (force_redraw_) {
    draw();

    return;
  }

  QPainter painter;

  painter.begin(this);

  painter.drawPixmap(0, 0, pixmap_);

  painter.end();
}

void
CQSudokuCanvas::
resizeEvent(QResizeEvent *)
{
  app_->clearEditCell();

  forceDraw();
}

void
CQSudokuCanvas::
forceDraw()
{
  force_redraw_ = true;

  repaint(0, 0, -1, -1);
}

void
CQSudokuCanvas::
draw()
{
  force_redraw_ = false;

  int width  = this->width ();
  int height = this->height();

  pixmap_ = QPixmap(width, height);

  //pixmap_.fill(Qt::white);
  pixmap_.fill(0xFFAAAAFF);

  //-----

  QPainter painter;

  painter.begin(&pixmap_);

  QPen pen;

  //-----

  int size = int(0.9*std::min(width, height));

  int csize  = size/9;
  int rcsize = size - 9*csize;

  int dx = (width  - size)/2;
  int dy = (height - size)/2;

  int x1 = dx; int x2 = x1 + size;
  int y1 = dy; int y2 = y1 + size;

  painter.fillRect(QRect(dx, dy, size, size), QBrush(QColor(0xFFFFFFFF)));

  pen.setColor(Qt::black);

  painter.setPen(pen);

  int r;

  //------

  {
  int y1 = dy;
  int y2 = y1;
  int ry = rcsize;

  for (int i = 0; i < 9; ++i) {
    y2 = y1 + csize;

    if (ry > 0) {
      ++y2;
      --ry;
    }

    int x1 = dx;
    int x2 = x1;
    int rx = rcsize;

    for (int j = 0; j < 9; ++j) {
      x2 = x1 + csize;

      if (rx > 0) {
        ++x2;
        --rx;
      }

      if (app_->getIsInitUnknown(i, j))
        painter.fillRect(QRect(x1, y1, x2 - x1, y2 - y1), QBrush(QColor(0xFFDDDDDD)));

      x1 = x2;
    }

    y1 = y2;
  }
  }

  //------

  x1 = dx;
  y1 = dy;
  r  = rcsize;

  for (int i = 0; i <= 9; ++i) {
    painter.drawLine(x1, y1, x1, y2);

    if ((i % 3) == 0)
      painter.drawLine(x1 + 1, y1, x1 + 1, y2);

    x1 += csize;

    if (r > 0) {
      ++x1;
      --r;
    }
  }

  x1 = dx;
  y1 = dy;
  r  = rcsize;

  for (int i = 0; i <= 9; ++i) {
    painter.drawLine(x1, y1, x2, y1);

    if ((i % 3) == 0)
      painter.drawLine(x1, y1 + 1, x2, y1 + 1);

    y1 += csize;

    if (r > 0) {
      ++y1;
      --r;
    }
  }

  //-----

  int font_size1 = std::max(int(csize*0.45), 1);
  int font_size2 = std::max(int(csize*0.15), 1);

  QFont font1("helvetica", font_size1);
  QFont font2("helvetica", font_size2);

  QFontMetrics fm1(font1);
  QFontMetrics fm2(font2);

  int char_height1 = fm1.height();
  int char_height2 = fm2.height();

  int char_descent1 = fm1.descent();
  //int char_descent2 = fm2.descent();

  char str[2];

  str[1] = '\0';

  int y  = dy;
  int ry = rcsize;

  for (int i = 0; i < 9; ++i) {
    int x  = dx;
    int rx = rcsize;

    for (int j = 0; j < 9; ++j) {
       int value = app_->getCellValue(i, j);

       if (value != 0) {
         painter.setFont(font1);

         int svalue = app_->getSolutionCellValue(i, j);

         str[0] = value + '0';

         int char_width = fm1.width(str);

         if (value != svalue)
           pen.setColor(Qt::red);
         else
           pen.setColor(Qt::black);

         painter.setPen(pen);

         painter.drawText(x + csize/2 - char_width/2,
                          y + csize/2 + char_height1/2 - char_descent1,
                          str);
       }
       else {
         if      (show_values_) {
           painter.setFont(font2);

           int svalue = app_->getSolvedCellValue(i, j);

           for (int k = 1; k <= 9; ++k) {
             int ik = (k - 1) % 3 + 1;
             int jk = (k - 1) / 3 + 1;

             bool value = app_->isSolveCellValue(i, j, k);

             if (value) {
               str[0] = k + '0';

               int char_width = fm2.width(str);

               if (k == svalue)
                 pen.setColor(0xFFFF4444);
               else
                 pen.setColor(0xFF44CC44);

               painter.setPen(pen);

               painter.drawText(x + ik*csize/4 - char_width/2,
                                y + jk*csize/4 + char_height2/2,
                                str);
             }
           }
         }
         else if (show_solution_) {
           int svalue = app_->getSolutionCellValue(i, j);

           painter.setFont(font1);

           str[0] = svalue + '0';

           int char_width = fm1.width(str);

           pen.setColor(0xFF44CC44);

           painter.setPen(pen);

           painter.drawText(x + csize/2 - char_width/2,
                            y + csize/2 + char_height1/2 - char_descent1,
                            str);
         }
       }

       x += csize;

       if (rx > 0) {
         ++x;
         --rx;
       }
     }

     y += csize;

     if (ry > 0) {
       ++y;
       --ry;
     }
  }

  //-----

  painter.end();

  //-----

  painter.begin(this);

  painter.drawPixmap(0, 0, pixmap_);

  painter.end();
}

void
CQSudokuCanvas::
showValues(int value)
{
  show_values_ = value;

  forceDraw();
}

void
CQSudokuCanvas::
showSolution(int value)
{
  show_solution_ = value;

  forceDraw();
}

void
CQSudokuCanvas::
step()
{
  app_->step();

  forceDraw();
}

void
CQSudokuCanvas::
solve()
{
  app_->solve();

  forceDraw();
}

void
CQSudokuCanvas::
print()
{
  app_->print();
}

void
CQSudokuCanvas::
reset()
{
  app_->reset();

  forceDraw();
}

void
CQSudokuCanvas::
newGame()
{
  app_->newGame();

  forceDraw();
}

void
CQSudokuCanvas::
editCellChanged()
{
  app_->editCellChanged();

  forceDraw();
}

bool
CQSudokuCanvas::
xyToCell(int x, int y, int *i, int *j) const
{
  int width  = this->width ();
  int height = this->height();

  int size = int(0.9*std::min(width, height));

  int dx = (width  - size)/2;
  int dy = (height - size)/2;

  int x1 = dx;
  int y1 = dy;

  if (x < x1 || x > x1 + size ||
      y < y1 || y > y1 + size)
    return false;

  *i = (9*(y - y1))/size;
  *j = (9*(x - x1))/size;

  if (*i < 0 || *i >= 9 || *j < 0 || *j >= 9)
    return false;

  return true;
}

bool
CQSudokuCanvas::
cellToXY(int i, int j, int *x, int *y) const
{
  int width  = this->width ();
  int height = this->height();

  int size = int(0.9*std::min(width, height));

  int dx = (width  - size)/2;
  int dy = (height - size)/2;

  int x1 = dx;
  int y1 = dy;

  *x = (j*size)/9 + x1;
  *y = (i*size)/9 + y1;

  return true;
}

bool
CQSudokuCanvas::
getCellSize(int *csize) const
{
  int width  = this->width ();
  int height = this->height();

  int size = int(0.9*std::min(width, height));

  *csize  = size/9;

  return true;
}

//-----

CQSudokuLineEdit::
CQSudokuLineEdit(QWidget *parent) :
 QLineEdit(parent)
{
  setInputMask("d");
  setMaxLength(1);
}

void
CQSudokuLineEdit::
setNumber(int n, int x, int y, int size)
{
  char str[2];

  str[0] = n + '0';
  str[1] = '\0';

  int font_size = std::max(int(size*0.45), 1);

  QFont font("helvetica", font_size);

  QFontMetrics fm(font);

  setFont(font);

  int char_width  = fm.width(str);
  int char_height = fm.height();

  int w = char_width  + 12;
  int h = char_height + 4;

  x += (size - w)/2 + 2;
  y += (size - h)/2 + 3;

  move(x, y);

  resize(w, h);

  if (str[0] == '0')
    str[0] = ' ';

  setText(str);

  show();

  setSelection(0, 1);

  setFocus();
}

void
CQSudokuLineEdit::
keyPressedEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Escape)
    hide();
}
