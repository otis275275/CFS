#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_set_sched_policy(void)
{
  int policy;
  argint(0, &policy);
  return set_sched_policy(policy);
}

uint64
sys_set_priority(void)
{
  int pid, priority;
  struct proc *p;
  
  argint(0, &pid);
  argint(1, &priority);
    
  if(priority < -20) priority = -20;
  if(priority > 19) priority = 19;
  
  if(pid == 0) {
      myproc()->nice = priority;
      return 0;
  }
  
  // Find proc
  // Note: accessing global proc array requires iterating safely, 
  // but looking up by pid is done in kill/wait usually.
  // For simplicity here, we assume user passes 0 typically or we iterate.
  // Actually, let's just implement it for current process if pid=0.
  // If we want to change others, we need to iterate proc table.
  
  extern struct proc proc[];
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      p->nice = priority;
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  
  return -1;
}
