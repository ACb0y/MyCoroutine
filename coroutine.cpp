//
// Created by ACb0y on 2022/2/15.
//

#include "coroutine.h"

#include <assert.h>

#include <iostream>

namespace MyCoroutine {

static void CoroutineStackClean(Coroutine* routine) {
  if (routine->stackCap <= 0) {
    return;
  }

  routine->stackSize = 0;
  routine->stackCap = 0;
  free(routine->stack);
  routine->stack = NULL;
}

static void CoroutineRun(Schedule* schedule) {
  schedule->isMasterCoroutine = false;
  int id = schedule->runningCoroutineId;
  assert(id >= 0 && id < schedule->coroutineCnt);

  Coroutine* routine = schedule->coroutines[id];
  // 执行entry函数
  routine->entry(routine->arg);
  // entry函数执行完之后，才能把协程状态更新为idle，并标记runningCoroutineId为无效的id
  routine->state = Idle;
  // 及时释放不使用的协程动态栈空间
  CoroutineStackClean(routine);
  schedule->runningCoroutineId = INVALID_ROUTINE_ID;
  // 这个函数执行完，调用栈会回到主协程中，执行routine->ctx.uc_link指向的上下文的下一条指令
}

static void CoroutineInit(Schedule& schedule, Coroutine* routine, Entry entry, void* arg, uint32_t priority) {
  routine->arg = arg;
  routine->entry = entry;
  routine->state = Run;
  routine->priority = priority;
  routine->stack = NULL;
  routine->stackCap = 0;
  routine->stackSize = 0;
  getcontext(&(routine->ctx));
  routine->ctx.uc_stack.ss_flags = 0;
  routine->ctx.uc_stack.ss_sp = schedule.stack;
  routine->ctx.uc_stack.ss_size = SHARE_STACK_SIZE;
  routine->ctx.uc_link = &(schedule.main);
  // 设置routine->ctx上下文要执行的函数和对应的参数，
  // 这里没有直接使用entry和arg设置，而是多包了一层CoroutineRun函数的调用，
  // 是为了在CoroutineRun中entry函数执行完之后，从协程的状态更新Idle，并更新当前处于运行中的从协程id为无效id，
  // 这样这些逻辑就可以对上层调用透明。
  makecontext(&(routine->ctx), (void (*)(void))(CoroutineRun), 1, &schedule);
}

static void SaveCoroutineStack(Coroutine* routine, uint8_t* stack_top) {
  uint8_t stack_bottom;
  // 从协程要保存的调用栈大小要小于等于SHARE_STACK_SIZE
  assert(stack_top - &stack_bottom <= SHARE_STACK_SIZE);

  // 计算获取当前从协程的调用栈大小
  routine->stackSize = stack_top - &stack_bottom;
  // 从协程的栈缓存空间不足时，需要重新分配
  if (routine->stackCap < routine->stackSize) {
    if (routine->stack) {
      free(routine->stack);
    }
    routine->stackCap = routine->stackSize;
    routine->stack = (uint8_t*)malloc(routine->stackCap);
  }
  // 把当前从协程的调用栈内容保存到routine->stack中。
  memcpy(routine->stack, &stack_bottom, routine->stackSize);
}

static void RestoreCoroutineStack(Schedule& schedule, Coroutine* routine) {
  // 从栈顶的起始地址，还原协程栈的内容
  memcpy(schedule.stack + SHARE_STACK_SIZE - routine->stackSize, routine->stack, routine->stackSizes);
}

int CoroutineCreate(Schedule& schedule, Entry entry, void* arg, uint32_t priority) {
  assert(schedule.isMasterCoroutine);
  int id = 0;
  for (id = 0; id < schedule.coroutineCnt; id++) {
    if (schedule.coroutines[id]->state == Idle) {
      break;
    }
  }
  if (id >= schedule.coroutineCnt) {
    return INVALID_ROUTINE_ID;
  }

  schedule.runningCoroutineId = id;
  Coroutine* routine = schedule.coroutines[id];
  CoroutineInit(schedule, routine, entry, arg, priority);
  // 切换到刚创建的协程中运行，并把当前执行上下文保存到schedule.main中，
  // 当从协程执行结束或者从协程主动yield时，swapcontext才会返回。
  swapcontext(&(schedule.main), &(routine->ctx));
  schedule.isMasterCoroutine = true;
  return id;
}

void CoroutineYield(Schedule& schedule) {
  assert(schedule.isMasterCoroutine == false);
  int id = schedule.runningCoroutineId;
  assert(id >= 0 && id < schedule.coroutineCnt);

  Coroutine* routine = schedule.coroutines[schedule.runningCoroutineId];
  // 保存当前从协程的栈空间
  SaveCoroutineStack(routine, schedule.stack + SHARE_STACK_SIZE);
  // 更新当前的从协程状态为挂起
  routine->state = Suspend;
  // 当前的从协程让出执行权，并把当前的从协程的执行上下文保存到routine->ctx中，
  // 执行权回到主协程中，主协程再做调度，当从协程被主协程resume时，swapcontext才会返回。
  swapcontext(&routine->ctx, &(schedule.main));
  schedule.isMasterCoroutine = false;
}

int CoroutineResume(Schedule& schedule) {
  assert(schedule.isMaterCoroutine);
  int coroutineId = INVALID_ROUTINE_ID;
  uint32_t priority = UINT32_MAX;
  // 按优先级调度，选择优先级最高的状态为挂起的从协程来运行
  for (int i = 0; i < schedule.coroutineCnt; i++) {
    if (schedule.coroutines[i]->state == Suspend && schedule.coroutines[i]->priority < priority) {
      coroutineId = i;
      priority = schedule.coroutines[i]->priority;
    }
  }

  if (coroutineId == INVALID_ROUTINE_ID) {
    return InvalidId;
  }

  Coroutine* routine = schedule.coroutines[coroutineId];
  routine->state = Run;
  schedule.runningCoroutineId = coroutineId;
  RestoreCoroutineStack(schedule, routine);
  // 从主协程切换到协程编号为id的协程中执行，并把当前执行上下文保存到schedule.main中，
  // 当从协程执行结束或者从协程主动yield时，swapcontext才会返回。
  swapcontext(&schedule.main, &routine->ctx);
  schedule.isMasterCoroutine = true;
  return Success;
}

int CoroutineResumeById(Schedule& schedule, int id) {
  assert(schedule.isMaterCoroutine);
  assert(id >= 0 && id < schedule.coroutineCnt);

  Coroutine* routine = schedule.coroutines[coroutineId];
  // 挂起状态的协程调用才生效
  if (routine->state != Suspend) {
    return NotSuspend;
  }
  routine->state = Run;
  schedule.runningCoroutineId = coroutineId;
  RestoreCoroutineStack(schedule, routine);
  // 从主协程切换到协程编号为id的协程中执行，并把当前执行上下文保存到schedule.main中，
  // 当从协程执行结束或者从协程主动yield时，swapcontext才会返回。
  swapcontext(&schedule.main, &routine->ctx);
  schedule.isMasterCoroutine = true;
  return Success;
}

int ScheduleInit(Schedule& schedule, int coroutineCnt) {
  // 最多创建MAX_COROUTINE_SIZE个协程
  assert(coroutineCnt > 0 && coroutineCnt <= MAX_COROUTINE_SIZE);
  schedule.isMasterCoroutine = true;
  schedule.coroutineCnt = coroutineCnt;
  schedule.runningCoroutineId = INVALID_ROUTINE_ID;
  for (int i = 0; i < coroutineCnt; i++) {
    schedule.coroutines[i] = new Coroutine;
    schedule.coroutines[i]->state = Idle;
  }
  return 0;
}

bool ScheduleRunning(Schedule& schedule) {
  assert(schedule.isMasterCoroutine);
  if (schedule.runningCoroutineId != INVALID_ROUTINE_ID) {
    return true;
  }
  for (int i = 0; i < schedule.coroutineCnt; i++) {
    if (schedule.coroutines[i]->state != Idle) {
      return true;
    }
  }
  return false;
}

}  // namespace MyCoroutine
