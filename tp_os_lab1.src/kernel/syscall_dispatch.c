#include "oslib.h"
#include "alloc.h"
#include "kernel.h"

/* sys_add
 *   test function
 */
int sys_add(int a, int b)
{
    return a + b;
}

/* syscall_dispatch
 *   dispatch syscalls
 *   n      : syscall number
 *   args[] : array of the parameters (4 max)
 */
int32 syscall_dispatch(uint32 n, uint32 args[])
{
    switch(n) {
      case 0:
          return sys_add((int)args[0], (int)args[1]);
      case 1:
          return sys_alloc((unsigned int)args[0]);
      case 2:
          sys_free((void *)args[0]);
          return 0;
      case 3:
          sys_os_start();
          return 0;
      case 4:
          return sys_task_new((TaskCode)args[0], (uint32)args[1]);
      case 5:
          return sys_task_id();
      case 6:
          sys_task_wait((uint32)args[0]);
          return 0;
      case 7:
          return (int32)sys_sem_new((int32)args[0]);
      case 8:
          sys_sem_p((Semaphore *)args[0]);
          return 0;
      case 9:
          sys_sem_v((Semaphore *)args[0]);
          return 0;
    }
    return -1;
}

