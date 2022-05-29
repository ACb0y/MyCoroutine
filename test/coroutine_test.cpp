//
// Created by ACb0y on 2022/4/5.
//

#include "../coroutine.h"

#include <iostream>

#include "UTestCore.h"
using namespace std;

TEST_CASE(scheduleInit) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 1), 0);
  MyCoroutine::ScheduleClean(schedule);
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 1024), 0);
  MyCoroutine::ScheduleClean(schedule);
}

TEST_CASE(ScheduleClean) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 100), 0);
  MyCoroutine::ScheduleClean(schedule);
}

void fun1(void *arg) {
  MyCoroutine::Schedule *schedule = (MyCoroutine::Schedule *)arg;
  cout << "fun1 begin" << endl;
  MyCoroutine::CoroutineYield(*schedule);
  cout << "fun1 end" << endl;
}

TEST_CASE(ScheduleRunning) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 100), 0);
  ASSERT_FALSE(MyCoroutine::ScheduleRunning(schedule));
  int id = MyCoroutine::CoroutineCreate(schedule, fun1, (void *)&schedule);
  ASSERT_EQ(id, 0);
  ASSERT_TRUE(MyCoroutine::ScheduleRunning(schedule));
  ASSERT_EQ(MyCoroutine::CoroutineResumeById(schedule, id), MyCoroutine::Success);
  ASSERT_TRUE(MyCoroutine::ScheduleRunning(schedule));
  ASSERT_EQ(MyCoroutine::CoroutineResumeById(schedule, id), MyCoroutine::Success);
  MyCoroutine::ScheduleClean(schedule);
}

TEST_CASE(CoroutineCreate) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 1), 0);
  int id = MyCoroutine::CoroutineCreate(schedule, fun1, (void *)&schedule);
  ASSERT_EQ(id, 0);
  id = MyCoroutine::CoroutineCreate(schedule, fun1, (void *)&schedule);
  ASSERT_EQ(id, INVALID_ROUTINE_ID);
  while (MyCoroutine::ScheduleRunning(schedule)) {
    ASSERT_EQ(MyCoroutine::CoroutineResume(schedule), MyCoroutine::Success);
  }
  id = MyCoroutine::CoroutineCreate(schedule, fun1, (void *)&schedule);
  ASSERT_EQ(id, 0);
  MyCoroutine::ScheduleClean(schedule);
}

TEST_CASE(CoroutineResume_Success) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 100), 0);
  ASSERT_FALSE(MyCoroutine::ScheduleRunning(schedule));
  int id = MyCoroutine::CoroutineCreate(schedule, fun1, (void *)&schedule);
  ASSERT_EQ(id, 0);
  ASSERT_TRUE(MyCoroutine::ScheduleRunning(schedule));
  ASSERT_EQ(MyCoroutine::CoroutineResume(schedule), MyCoroutine::Success);
  ASSERT_TRUE(MyCoroutine::ScheduleRunning(schedule));
  ASSERT_EQ(MyCoroutine::CoroutineResumeById(schedule, id), MyCoroutine::Success);
  MyCoroutine::ScheduleClean(schedule);
}

TEST_CASE(CoroutineResume_InvalidId) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 100), 0);
  ASSERT_FALSE(MyCoroutine::ScheduleRunning(schedule));
  ASSERT_EQ(MyCoroutine::CoroutineResume(schedule), MyCoroutine::NotRunnable);
  MyCoroutine::ScheduleClean(schedule);
}

TEST_CASE(CoroutineResumeById_SuccessAndNotSuspend) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 100), 0);
  ASSERT_FALSE(MyCoroutine::ScheduleRunning(schedule));
  int id = MyCoroutine::CoroutineCreate(schedule, fun1, (void *)&schedule);
  ASSERT_EQ(id, 0);
  ASSERT_TRUE(MyCoroutine::ScheduleRunning(schedule));
  ASSERT_EQ(MyCoroutine::CoroutineResumeById(schedule, 0), MyCoroutine::Success);
  ASSERT_TRUE(MyCoroutine::ScheduleRunning(schedule));
  ASSERT_EQ(MyCoroutine::CoroutineResumeById(schedule, 0), MyCoroutine::Success);
  MyCoroutine::ScheduleClean(schedule);
}

void fun2(void *arg) {
  MyCoroutine::Schedule *schedule = (MyCoroutine::Schedule *)arg;
  cout << "fun2 begin" << endl;
  for (int i = 0; i < 5; i++) {
    MyCoroutine::CoroutineYield(*schedule);
    cout << "fun2 yield step " << i + 1 << endl;
  }
  cout << "fun2 end" << endl;
}

TEST_CASE(CoroutineYield) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 100), 0);
  ASSERT_FALSE(MyCoroutine::ScheduleRunning(schedule));
  int id = MyCoroutine::CoroutineCreate(schedule, fun2, (void *)&schedule);
  ASSERT_EQ(id, 0);

  while (MyCoroutine::ScheduleRunning(schedule)) {
    ASSERT_EQ(MyCoroutine::CoroutineResumeById(schedule, 0), MyCoroutine::Success);
    cout << "in master coroutine" << endl;
  }

  MyCoroutine::ScheduleClean(schedule);
}

RUN_ALL_TESTS();
