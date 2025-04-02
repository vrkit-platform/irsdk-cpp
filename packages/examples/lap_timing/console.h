#pragma once
#include <cstdint>

namespace Console {
  // true if any key was hit, leaves character in buffer
  bool isKeypress();

  // returns character that was hit, or 0 if no key
  char getKeypress();

  // halt execution of program till user presses and releases any key
  // returns pressed char
  char waitForKeypress();

  // get row/column count of console
  bool getDimensions(int &w, int &h);

  // get current location
  bool getCursorPosition(int &x, int &y);

  // move the cursor
  void setCursorPosition(int x, int y);

  // clear count characters at x,y
  void clearSpace(int x, int y, int count);

  // clear the indicated line
  void clearLine(int y);

  // clear the whole console
  void clearScreen();

  enum class Color : WORD {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    YELLOW = 6,
    WHITE = 7
  };


  void setStyle(Color bgColor = Color::BLACK, Color fgColor = Color::WHITE, bool bold = false);
}