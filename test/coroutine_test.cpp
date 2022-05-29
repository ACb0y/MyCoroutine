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
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 1024), 0);
}

TEST_CASE(ScheduleClean) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 100), 0);
  MyCoroutine::ScheduleClean(schedule);
}

void fun1(void *arg) {
  MyCoroutine::Schedule *schedule = (MyCoroutine::Schedule *)arg;
  MyCoroutine::CoroutineYield(schedule);
}

TEST_CASE(ScheduleRunning) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 100), 0);
  ASSERT_FALSE(MyCoroutine::ScheduleRunning(schedule));
  int id = MyCoroutine::CoroutineCreate(schedule, fun1, (void *)&schedule);
  ASSERT_EQ(id, 0);
  ASSERT_TRUE(MyCoroutine::ScheduleRunning(schedule));
  ASSERT_EQ(MyCoroutine::CoroutineResumeById(schedule, id), MyCoroutine::Success);
  ASSERT_FALSE(MyCoroutine::ScheduleRunning(schedule));
}
/*
 * / 创建协程并运行，只能在主协程中调用
int CoroutineCreate(Schedule& schedule, Entry entry, void* arg, uint32_t priority = 0);
// 让出执行权，只能在从协程中调用
void CoroutineYield(Schedule& schedule);
// 恢复从协程的调用，只能在主协程中调用
int CoroutineResume(Schedule& schedule);
// 恢复指定从协程的调用，只能在主协程中调用
int CoroutineResumeById(Schedule& schedule, int id);

// 判断是否还有协程在运行
bool ScheduleRunning(Schedule& schedule);
 */
RUN_ALL_TESTS();
