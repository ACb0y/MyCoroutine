//
// Created by ACb0y on 2022/2/15.
//

#include <assert.h>
#include "coroutine.h"

namespace MyCoroutine {

static void CoroutineRun(Schedule * schedule) {
  int id = schedule->runningCoroutineId;
  assert(id >= 0 && id < MAX_COROUTINE_SIZE);

  Coroutine * routine = &schedule->coroutines[id];
  // 执行entry函数
  routine->entry(routine->arg);
  // entry函数执行完之后，才能把协程状态更新为idle，并标记
  routine->state = Idle;
  schedule->runningCoroutineId = INVALID_ROUTINE_ID;
  // 这个函数执行完，调用栈会回到主协程中，执行routine->ctx.uc_link指向的上下文的下一条指令
}

static void CoroutineInit(Schedule & schedule, Coroutine * routine, Entry entry, void * arg) {
  routine->arg = arg;
  routine->entry = entry;
  routine->state = Running;
  getcontext(&(routine->ctx));
  routine->ctx.uc_stack.ss_flags = 0;
  routine->ctx.uc_stack.ss_sp = routine->stack;
  routine->ctx.uc_stack.ss_size = DEFAULT_STACK_SIZE;
  routine->ctx.uc_link = &(schedule.main);
  // 设置routine->ctx上下文要执行的函数和对应的参数
  makecontext(&(routine->ctx), (void (*)(void))(CoroutineRun), 1, &schedule);
}

int CoroutineCreate(Schedule & schedule, Entry entry, void * arg) {
  int id = 0;
  for (id = 0; id < MAX_COROUTINE_SIZE; id++) {
    if (schedule.coroutines[id].state == Idle) {
      break;
    }
  }
  if (id >= MAX_COROUTINE_SIZE) {
    return INVALID_ROUTINE_ID;
  }
  schedule.runningCoroutineId = id;
  Coroutine * routine = &schedule.coroutines[id];
  CoroutineInit(schedule, routine, entry, arg);
  // 切换到刚创建的协程中运行，并把当前执行上下文保持到schedule.main中，
  // 当从协程执行结束或者从协程主动yield时，swapcontext才会返回。
  swapcontext(&(schedule.main), &(routine->ctx));
  return id;
}

void CoroutineYield(Schedule & schedule) {
  int id = schedule->runningCoroutineId;
  assert(id >= 0 && id < MAX_COROUTINE_SIZE);

  Coroutine * routine = &schedule.coroutines[schedule.runningCoroutineId];
  // 更新当前的从协程状态为挂起
  routine->state = Suspend;
  schedule.runningCoroutineId = INVALID_ROUTINE_ID;
  // 当前的从协程让出执行权，并把当前的从协程的执行上下文保存到routine->ctx中，
  // 执行权回到主协程中，主协程再做调度，当从协程被主协程resume时，swapcontext才会返回。
  swapcontext(&routine->ctx, &(schedule.main));
}

void CoroutineResume(Schedule & schedule, int id) {
  assert(id >= 0 && id < MAX_COROUTINE_SIZE);

  Coroutine * routine = &schedule.coroutines[id];
  // 挂起状态的协程调用才生效
  if (routine->state == Suspend) {
    schedule.runningCoroutineId = id;
    // 从主协程切换到协程编号为id的协程中执行，并把当前执行上下文保存到schedule.main中，
    // 当从协程执行结束或者从协程主动yield时，swapcontext才会返回。
    swapcontext(&schedule.main, &routine->ctx);
  }
}

bool ScheduleRunning(Schedule & schedule) {
  for (int i = 0; i < MAX_COROUTINE_SIZE; i++) {
    if (schedule.coroutines[i].state != Idle) {
      return true;
    }
  }
  return false;
}

}
