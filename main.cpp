//
// Created by ACb0y on 2022/2/19.
//

#include <iostream>
#include "coroutine.h"
using namespace std;

void routine1(void * arg) {
  cout << "routine1 run" << endl;
}

void routine2(void * arg) {
  cout << "routine2 run" << endl;
}

int main() {
  MyCoroutine::Schedule schedule;
  cout << "test2" << endl;
  int a = 10;
  int b = 10;
  cout << schedule.coroutines.size() << endl;
  MyCoroutine::CoroutineCreate(&schedule, routine1, &a);
  cout << "test3" << endl;
  MyCoroutine::CoroutineCreate(&schedule, routine2, &b);
  return 0;
}
