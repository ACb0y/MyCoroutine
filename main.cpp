//
// Created by ACb0y on 2022/2/19.
//

#include <iostream>
#include "coroutine.h"
using namespace std;

void routine1(void * arg) {
  cout << "routine1 run, value = " << *(int *)(arg) << endl;
}

void routine2(void * arg) {
  cout << "routine2 run, value = " << *(int *)(arg) << endl;
}

int main() {
  MyCoroutine::Schedule schedule;
  int a = 10;
  int b = 100;
  MyCoroutine::CoroutineCreate(schedule, routine1, &a);
  MyCoroutine::CoroutineCreate(schedule, routine2, &b);
  return 0;
}
