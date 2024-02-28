#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "strace.h"
#include "syscall.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  STRACE_ARGS("exit status: %d", n);
  STRACE_RETURN(SYS_exit);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  STRACE();
  STRACE_RETURN(SYS_getpid);
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  STRACE();
  STRACE_RETURN(SYS_fork);
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  STRACE_ARGS("time to wait: %d", p);
  STRACE_RETURN(SYS_wait);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  STRACE_ARGS("Address: %p, n: %d", &addr, n);
  if(growproc(n) < 0)
    return -1;

  STRACE_RETURN(SYS_sbrk);
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  STRACE_ARGS("Time: %d, Ticks: %d", n, ticks0);
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  STRACE_RETURN(SYS_sleep);
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  STRACE_ARGS("PID: %d", pid);
  STRACE_RETURN(SYS_kill);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  STRACE();

  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  STRACE_RETURN(SYS_uptime);
  return xticks;
}

// System call to get current timestamp
uint64
sys_time(void)
{
  STRACE();

  volatile uint64 *timestamp = (uint64 *) GOLDFISH_RTC;

  STRACE_RETURN(SYS_time);
  return *timestamp / 1000000000;
}

uint64
sys_strace(void)
{
  STRACE();

  myproc()->strace = 1;

  STRACE_RETURN(SYS_strace);
  return 0;
}
