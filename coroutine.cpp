//
// Created by ACb0y on 2022/2/15.
//

#include "coroutine.h"

namespace MyCoroutine {

static void CoroutineInit(Schedule & schedule, Coroutine * routine, Entry entry, void * arg) {
  routine->state = Ready;
  routine->entry = entry;
  routine->arg = arg;
  getcontext(&(routine->ctx));
  routine->ctx.uc_stack.ss_sp = routine->stack;
  routine->ctx.uc_stack.ss_size = DEFAULT_STACK_SIZE;
  routine->ctx.uc_stack.ss_flags = 0;
  routine->ctx.uc_link = &(schedule.main);
}

static void ScheduleCore(Schedule * schedule) {
  int id = schedule->runningIndex;
  assert(id >= 0 && id < MAX_COROUTINE_SIZE);
  Coroutine * routine = schedule->coroutines[id];
  assert(routine != NULL);
  routine->entry(routine->arg);
  routine->state = Idle;
  schedule->runningIndex = INVALID_RUNNING_INDEX;
  // 这个函数执行完，调用栈会回到主协程中
}

int CoroutineCreate(Schedule & schedule, Entry entry, void * arg) {
  int id = 0;
  for (id = 0; id < MAX_COROUTINE_SIZE; id++) {
    if (schedule.coroutines[id]->state == Idle) {
      break;
    }
  }
  if (id >= MAX_COROUTINE_SIZE) {
    return INVALID_RUNNING_INDEX;
  }
  schedule.runningIndex = id;
  Coroutine * routine = schedule.coroutines[id];
  CoroutineInit(schedule, routine, entry, arg);

  makecontext(&(routine->ctx), (void (*)(void))(ScheduleCore), 1, &schedule);
  // 切换到刚创建的协程中运行
  swapcontext(&(schedule.main), &(routine->ctx));
  return id;
}

void CoroutineYield(Schedule & schedule) {
  assert(schedule.runningIndex >= 0 && schedule.runningIndex < MAX_COROUTINE_SIZE);
  Coroutine * routine = schedule.coroutines[schedule.runningIndex];
  routine->state = Suspend;
  // 当前协程让出执行权，执行权回到主协程中，主协程再做调度
  schedule.runningIndex = INVALID_RUNNING_INDEX;
  // 切换到主协程的上下文中执行，swapcontext调用不会返回
  assert(swapcontext(&routine->ctx, &(schedule.main)) == 0);
}

void CoroutineResume(Schedule & schedule, int id) {
  assert(id >= 0 && id < MAX_COROUTINE_SIZE);
  Coroutine * routine = schedule.coroutines[id];
  assert(routine != NULL);
  // 挂起状态的协程调用才生效，执行栈切换到对应id的协程中，并把当前调用栈信息保存到schedule.main当中
  if (routine->state == Suspend) {
    schedule.runningIndex = id;
    // 从主协程切换到协程编号为id的协程中执行，swapcontext调用不会返回
    assert(swapcontext(&schedule.main, &routine->ctx) == 0);
  }
}

bool ScheduleRunning(Schedule & schedule) {
  for (int i = 0; i < MAX_COROUTINE_SIZE; i++) {
    if (schedule.coroutines[i]->state != Idle) {
      return true;
    }
  }
  return false;
}

}

/*
#ifndef MY_UTHREAD_CPP
#define MY_UTHREAD_CPP


#include "uthread.h"
//#include <stdio.h>



void uthread_body(schedule_t *ps)
{
    int id = ps->running_thread;

    if(id != -1){
        uthread_t *t = &(ps->threads[id]);

        t->func(t->arg);

        t->state = FREE;

        ps->running_thread = -1;
    }
}




#endif
 */