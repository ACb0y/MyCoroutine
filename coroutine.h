//
// Created by ACb0y on 2022/2/15.
//

#pragma once

#include <stdint.h>
#include <ucontext.h>

namespace MyCoroutine {

#define INVALID_ROUTINE_ID    -1
#define MAX_COROUTINE_SIZE    1024      // 最大创建1024个协程
#define DEFAULT_STACK_SIZE    12 * 1024 // 12K的调用栈

// 协程的状态，协程的状态转移如下：
// idle->run
// run->suspend
// suspend->run
// run->ide
enum State {
  Idle = 1,     // 空闲
  Run = 2,      // 运行
  Suspend = 3,  // 挂起
};

// 协程入口函数
typedef void(*Entry)(void * arg);

// 协程结构体
typedef struct Coroutine {
  State state;                        // 协程当前的状态
  uint32_t priority;                  // 协程优先级，值越小，优先级越高
  void * arg;                         // 协程入口函数的参数
  Entry entry;                        // 协程入口函数
  ucontext_t ctx;                     // 协程执行上下文
  uint8_t stack[DEFAULT_STACK_SIZE];  // 协程栈
}Coroutine;

// 协程调度器
typedef struct Schedule {
  ucontext_t main;                            // 用于保存主协程的上下文
  int32_t runningCoroutineId;                 // 运行中（Run + Suspend）的从协程的id
  Coroutine * coroutines[MAX_COROUTINE_SIZE]; // 从协程数组池
  int32_t coroutineCnt;                       // 协程个数
}Schedule;

// 创建协程并运行，只能在主协程中调用
int CoroutineCreate(Schedule & schedule, Entry entry, void * arg, uint32_t priority = 0);
// 让出执行权，只能在从协程中调用
void CoroutineYield(Schedule & schedule);
// 恢复从协程的调用，只能在主协程中调用
void CoroutineResume(Schedule & schedule, int id = INVALID_ROUTINE_ID);
// 协程初始化
int ScheduleInit(Schedule & schedule, int coroutineCnt);
// 判断是否还有协程在运行
bool ScheduleRunning(Schedule & schedule);

}



