//
// Created by ACb0y on 2022/2/15.
//

#pragma once

#include <ucontext.h>
#include <stdint.h>
#include <vector>
#include <iostream>


namespace MyCoroutine {

#define INVALID_RUNNING_INDEX -1
#define MAX_COROUTINE_SIZE    2048       // 最大创建2048个协程
#define DEFAULT_STACK_SIZE    100 * 1024 // 100K的调用栈

enum State {
  Idle = 1,
  Ready = 2,
  Running = 3,
  Suspend = 4,
};

// 协程入口函数
typedef void(*Entry)(void * arg);

// 协程结构体
typedef struct Coroutine {
  ucontext_t ctx;
  Entry entry;
  void * arg;
  uint8_t stack[DEFAULT_STACK_SIZE];
  State state;
}Coroutine;

// 协程调度
typedef struct Schedule {
  ucontext_t main;
  int32_t runningIndex;
  std::vector<Coroutine *> coroutines;

  Schedule() {
    runningIndex = INVALID_RUNNING_INDEX;
    for (int i = 0; i < MAX_COROUTINE_SIZE; i++) {
      Coroutine * routine = new Coroutine;
      routine->state = Idle;
      coroutines.push_back(routine);
    }
  }
  ~Schedule() {
    for (int i = 0; i < MAX_COROUTINE_SIZE; i++) {
      delete coroutines[i];
    }
  }
}Schedule;

int CoroutineCreate(Schedule & schedule, Entry entry, void * arg);
void CoroutineYield(Schedule & schedule);
void CoroutineResume(Schedule & schedule, int id);
bool ScheduleRunning(Schedule & schedule);

}



