//
// Created by ACb0y on 2022/2/15.
//

#pragma once

#include <ucontext.h>
#include <stdint.h>
#include <vector>
#include <iostream>

namespace MyCoroutine {

#define INVALID_ROUTINE_ID    -1
#define MAX_COROUTINE_SIZE    2048      // 最大创建2048个协程
#define DEFAULT_STACK_SIZE    12 * 1024 // 12K的调用栈

enum State {
  Idle = 1,
  Running = 2,
  Suspend = 3,
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
  int32_t runningCoroutineId;
  Coroutine coroutines[MAX_COROUTINE_SIZE];

  Schedule() {
    std::cout << "test" << std::endl;
    runningCoroutineId = INVALID_ROUTINE_ID;
    std::cout << "test" << std::endl;
    for (int i = 0; i < MAX_COROUTINE_SIZE; i++) {
      std::cout << "a" << std::endl;
      coroutines[i].state = Idle;
    }
  }
}Schedule;

int CoroutineCreate(Schedule & schedule, Entry entry, void * arg);
void CoroutineYield(Schedule & schedule);
void CoroutineResume(Schedule & schedule, int id);
bool ScheduleRunning(Schedule & schedule);

}



