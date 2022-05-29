//
// Created by ACb0y on 2022/4/5.
//

#include "../coroutine.h"

#include <iostream>

#include "UTestCore.h"
using namespace std;

TEST_CASE(fun1Test) {
  MyCoroutine::Schedule schedule;
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 1), 0);
  ASSERT_EQ(MyCoroutine::ScheduleInit(schedule, 1024), 0);
}

RUN_ALL_TESTS();
