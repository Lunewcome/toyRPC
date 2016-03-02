/**
 */

#include "src/event_loop.h"

#include <iostream>
using namespace std;

int main() {
  EventLoop el;
  el.Start();
  el.Stop();

  cout << "DONE!" << endl;

  return 0;
}
