#include "generate_prompt.h"
#include <iostream>

int main() {
  BuildingMap ahg;
  ahg = init_map();
  // ahg.printMap();

  vector<string> prompt = generate_prompt("2.202", NORTH, ahg);
  cout << prompt[0] << prompt[1] << endl;

  return 0;
}