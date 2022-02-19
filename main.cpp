//
// Created by ACb0y on 2022/2/19.
//

#include <iostream>
#include "coroutine.h"
using namespace std;

void routine3(void * arg) {
  cout << "routine3 run begin" << endl;
  MyCoroutine::Schedule * schedule = (MyCoroutine::Schedule *)arg;
  MyCoroutine::CoroutineYield(*schedule);
  cout << "routine3 run end" << endl;
}

void routine1(void * arg) {
  cout << "routine1 run begin" << endl;
  MyCoroutine::Schedule * schedule = (MyCoroutine::Schedule *)arg;
  int id3 = MyCoroutine::CoroutineCreate(*schedule, routine3, schedule);
  cout << "id3 = " << id3 << endl;
  cout << "routine1 run end" << endl;
}

void routine2(void * arg) {
  cout << "routine2 run begin" << endl;
  MyCoroutine::Schedule * schedule = (MyCoroutine::Schedule *)arg;
  MyCoroutine::CoroutineYield(*schedule);
  cout << "routine2 run end" << endl;
}

int main() {
  MyCoroutine::Schedule schedule;
  int id1 = 0, id2 = 0;
  id1 = MyCoroutine::CoroutineCreate(schedule, routine1, &schedule);
  cout << "id1 = " << id1 << endl;
  id2 = MyCoroutine::CoroutineCreate(schedule, routine2, &schedule);
  cout << "id2 = " << id2 << endl;
  while (MyCoroutine::ScheduleRunning(schedule)) {
    MyCoroutine::CoroutineResume(schedule, id2);
    MyCoroutine::CoroutineResume(schedule, id1);
  }
  return 0;
}
