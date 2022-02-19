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
  MyCoroutine::CoroutineCreate(schedule, routine1, &a);
  MyCoroutine::CoroutineCreate(schedule, routine2, &b);
  return 0;
}
