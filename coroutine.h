//
// Created by ACb0y on 2022/2/15.
//

#pragma once

#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#include <cstdint>

namespace MyCoroutine {

#define INVALID_ROUTINE_ID -1
#define MAX_COROUTINE_SIZE 1024  // 最多创建1024个协程
#define STACK_SIZE 64 * 1024     // 64k的协程栈

/* 协程的状态，协程的状态转移如下：
 * idle->ready
 * ready->run
 * run->suspend
 * suspend->run
 * run->ide
 */
enum State {
  Idle = 1,     // 空闲
  Ready = 2,    // 就绪
  Run = 3,      // 运行
  Suspend = 4,  // 挂起
};

enum ResumeResult {
  NotRunnable = 1,  // 无可运行的协程
  Success = 2,      // 成功唤醒一个挂起状态的协程
};

// 协程入口函数
typedef void (*Entry)(void* arg);

// 协程结构体
typedef struct Coroutine {
  State state;                // 协程当前的状态
  uint32_t priority;          // 协程优先级，值越小，优先级越高
  void* arg;                  // 协程入口函数的参数
  Entry entry;                // 协程入口函数
  ucontext_t ctx;             // 协程执行上下文
  uint8_t stack[STACK_SIZE];  // 每个协程独占的协程栈
} Coroutine;

// 协程调度器
typedef struct Schedule {
  ucontext_t main;                            // 用于保存主协程的上下文
  int32_t runningCoroutineId;                 // 运行中（Run + Suspend）的从协程的id
  int32_t coroutineCnt;                       // 协程个数
  bool isMasterCoroutine;                     // 当前协程是否为主协程
  Coroutine* coroutines[MAX_COROUTINE_SIZE];  // 从协程数组池
} Schedule;

// 创建协程，只能在主协程中调用
int CoroutineCreate(Schedule& schedule, Entry entry, void* arg, uint32_t priority = 0);
// 让出执行权，只能在从协程中调用
void CoroutineYield(Schedule& schedule);
// 恢复从协程的调用，只能在主协程中调用
int CoroutineResume(Schedule& schedule);
// 恢复指定从协程的调用，只能在主协程中调用
int CoroutineResumeById(Schedule& schedule, int id);

// 协程调度结构体初始化
int ScheduleInit(Schedule& schedule, int coroutineCnt);
// 判断是否还有协程在运行
bool ScheduleRunning(Schedule& schedule);
// 释放调度器
void ScheduleClean(Schedule& schedule);

}  // namespace MyCoroutine
