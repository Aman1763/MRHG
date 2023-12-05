#include "generate_prompt.h"
#include <iostream>


std::string interpretEscapeSequences(const std::string& str) {
  std::string interpreted;
  size_t pos = 0;

  while (pos < str.size()) {
    if (str[pos] == '\\' && pos + 1 < str.size()) {
      switch (str[pos + 1]) {
        case 'n':
          interpreted.push_back('\n');
          pos += 2; // Skip the escape sequence
          break;
        // Add more cases for other escape sequences if needed
        default:
          interpreted.push_back(str[pos]);
          ++pos; // Keep the backslash and check the next character
          break;
      }
    } else {
      interpreted.push_back(str[pos]);
      ++pos;
    }
  }
  return interpreted;
}


int main() {
  BuildingMap ahg;
  ahg = init_map();
  // ahg.printMap();

  vector<string> prompt = generate_prompt("2.202", NORTH, ahg);
  string prompt1 = interpretEscapeSequences(prompt[0]);
  string prompt2 = interpretEscapeSequences(prompt[1]);
  cout << prompt1 << prompt2 << endl;

  return 0;
}