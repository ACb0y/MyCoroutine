//
// Created by ACb0y on 2022/2/19.
//

#include <iostream>

#include "coroutine.h"
using namespace std;

void routine1(void *arg) {
  cout << "routine1 run begin" << endl;
  MyCoroutine::Schedule *schedule = (MyCoroutine::Schedule *)arg;
  MyCoroutine::CoroutineYield(*schedule);
  cout << "routine1 running" << endl;
  MyCoroutine::CoroutineYield(*schedule);
  cout << "routine1 run end" << endl;
}

void routine2(void *arg) {
  cout << "routine2 run begin" << endl;
  MyCoroutine::Schedule *schedule = (MyCoroutine::Schedule *)arg;
  MyCoroutine::CoroutineYield(*schedule);
  cout << "routine2 running" << endl;
  MyCoroutine::CoroutineYield(*schedule);
  cout << "routine2 run end" << endl;
}

int main() {
  MyCoroutine::Schedule schedule;
  MyCoroutine::ScheduleInit(schedule, 100);
  int id1 = 0, id2 = 0;
  id1 = MyCoroutine::CoroutineCreate(schedule, routine1, &schedule, 2);
  cout << "id1 = " << id1 << endl;
  id2 = MyCoroutine::CoroutineCreate(schedule, routine2, &schedule, 1);
  cout << "id2 = " << id2 << endl;
  while (MyCoroutine::ScheduleRunning(schedule)) {
    MyCoroutine::CoroutineResume(schedule);
  }
  MyCoroutine::ScheduleClean(schedule);
  return 0;
}
