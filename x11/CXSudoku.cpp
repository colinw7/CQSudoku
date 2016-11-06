#include "std_c++.h"
#include "CSudoku/CSudoku.h"
#include "CXWidget/CXWidgetLib.h"
#include "CXLib/CXMachine.h"
#include "CXLibPixelRenderer/CXLibPixelRenderer.h"

class CXSudokuShell : public CXShellWidget {
 public:
  CXSudokuShell() :
   CXShellWidget("sudoku") {
  }
};

class CXSudokuCanvas : public CXCanvasWidget {
 private:
  CSudoku sudoku_;
  bool    show_possible_;

 public:
  CXSudokuCanvas(CXSudokuShell *shell);

  void expose();

  void keyPress(const CKeyEvent &event);
};

int
main(int, char **)
{
  CXSudokuShell shell;

  CXSudokuCanvas canvas(&shell);

  canvas.setSize(400, 400);

  shell.mapChildren();

  CXMachine::getInstance()->mainLoop();

  return 0;
}

CXSudokuCanvas::
CXSudokuCanvas(CXSudokuShell *shell) :
 CXCanvasWidget((CXWidget *) shell, "canvas")
{
  sudoku_.newGame();

  show_possible_ = false;
}

void
CXSudokuCanvas::
expose()
{
  CXLibPixelRenderer *renderer = getPixelRenderer();

  int width  = getWidth ();
  int height = getHeight();

  int size = min(width, height);

  int border = int(0.1*size);

  int delta = (size - 2*border)/9;

  int x1 = border;
  int x2 = border + 9*delta;
  int y1 = border;
  int y2 = border + 9*delta;

  renderer->setForeground(CRGB(1.0,1.0,1.0));

  renderer->fillRectangle(CIBBox2D(x1, y1, x2, y2));

  renderer->setForeground(CRGB(0.0,0.0,0.0));

  //------

  CFontPtr big_font =
    CFontMgrInst->lookupFont("helvetica", CFONT_STYLE_BOLD, delta/2);

  //------

  for (int i = 0, x = x1; i <= 9; ++i, x += delta) {
    renderer->drawLine(CIPoint2D(x, y1), CIPoint2D(x, y2));

    if (i % 3 == 0)
      renderer->drawLine(CIPoint2D(x + 1, y1), CIPoint2D(x + 1, y2 + 1));
  }

  for (int i = 0, y = y1; i <= 9; ++i, y += delta) {
    renderer->drawLine(CIPoint2D(x1, y), CIPoint2D(x2, y));

    if (i % 3 == 0)
      renderer->drawLine(CIPoint2D(x1, y + 1), CIPoint2D(x2 + 1, y + 1));
  }

  //------

  renderer->setFont(big_font);

  char str[2];

  str[1] = '\0';

  int k = 0;

  for (int j = 0, y = y1; j < 9; ++j, y += delta) {
    for (int i = 0, x = x1; i < 9; ++i, x += delta, ++k) {
      int value = sudoku_.getValue(i, j);

      if (value == 0) continue;

      str[0] = '0' + value;

      renderer->drawString(CIPoint2D(x + 3*delta/8, y + delta/4), str);
    }
  }

  //------

  if (show_possible_) {
    CFontPtr small_font =
      CFontMgrInst->lookupFont("helvetica", CFONT_STYLE_BOLD, delta/6);

    renderer->setFont(small_font);

    str[1] = '\0';

    k = 0;

    for (int j = 0, y = y1; j < 9; ++j, y += delta) {
      for (int i = 0, x = x1; i < 9; ++i, x += delta, ++k) {
        int value = sudoku_.getSolvedValue(i, j);

        if (value != 0) continue;

        for (int v = 0, v1 = 0; v1 < 3; ++v1) {
          for (int v2 = 0; v2 < 3; ++v2, ++v) {
            if (! sudoku_.isSolveValue(i, j, v + 1)) continue;

            str[0] = '1' + v;

            renderer->drawString(CIPoint2D(x + (v1 + 1)*delta/4, y + (v2 + 1)*delta/4), str);
          }
        }
      }
    }
  }
}

void
CXSudokuCanvas::
keyPress(const CKeyEvent &event)
{
  if (event.getType() == CKEY_TYPE_Space) {
    show_possible_ = ! show_possible_;

    expose();
  }
}
