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
#define MAX_COROUTINE_SIZE 1024           // 最多创建1024个协程
#define SHARE_STACK_SIZE 4 * 1024 * 1024  // 4M的共享调用栈，linux进程默认的调用栈大小为8M

/* 协程的状态，协程的状态转移如下：
 * idle->run
 * run->suspend
 * suspend->run
 * run->ide
 */
enum State {
  Idle = 1,     // 空闲
  Run = 2,      // 运行
  Suspend = 3,  // 挂起
};

enum ResumeResult {
  InvalidId = 1,   // 无效的协程id
  NotSuspend = 2,  // 无挂起状态的协程
  Success = 3,     // 成功唤醒一个挂起状态的协程
};

// 协程入口函数
typedef void (*Entry)(void* arg);

// 协程结构体
typedef struct Coroutine {
  State state;        // 协程当前的状态
  uint32_t priority;  // 协程优先级，值越小，优先级越高
  void* arg;          // 协程入口函数的参数
  Entry entry;        // 协程入口函数
  ucontext_t ctx;     // 协程执行上下文
  /*
   * 运行时的动态协程栈，动态扩缩容；
   * 在协程被挂起时，用于保存协程的调用栈；
   * 在协程被唤醒时，内存会被拷贝到共享调用栈中。
   */
  uint8_t* stack;
  uint32_t stackSize;  // 动态协程栈使用的大小
  uint32_t stackCap;   // 动态协程栈的容量大小 stackCap >= stackSize
} Coroutine;

// 协程调度器
typedef struct Schedule {
  ucontext_t main;                            // 用于保存主协程的上下文
  int32_t runningCoroutineId;                 // 运行中（Run + Suspend）的从协程的id
  int32_t coroutineCnt;                       // 协程个数
  bool isMasterCoroutine;                     // 当前协程是否为主协程
  Coroutine* coroutines[MAX_COROUTINE_SIZE];  // 从协程数组池
  uint8_t stack[SHARE_STACK_SIZE];            // 从协程的共享栈
} Schedule;

// 创建协程并运行，只能在主协程中调用
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

}  // namespace MyCoroutine
