#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
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


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 va;           // va保存待检查的page数组的虚拟地址
  int page_num;        // 要核验的page数
  uint64 abits_addr;   // abits从低到高记录page是否被访问过，abits_addr保存abits的地址
  if (argaddr(0, &va) < 0 || argint(1, &page_num) < 0 || argaddr(2, &abits_addr) < 0)
    return -1;

  if(va >= MAXVA)
    panic("walk");

  pagetable_t pagetable = myproc() -> pagetable;
  uint mask = 0;  // 内核数据
  for (int i = 0; i < page_num; ++i, va += PGSIZE) {
    pte_t *pte = 0;
    pagetable_t p = pagetable;
    for (int level = 2; level >= 0; --level) {
      // PX根据level提取va对应的9bit的L0/L1/L2，即64bits只有最右9bit起作用，可以索引pagetable不越界
      pte = &p[PX(level, va)];  // 根据L0/L1/L2获取页表中对应的pte，并取其地址
      if(*pte & PTE_V)  // 如果该PTE有效
        p = (pagetable_t)PTE2PA(*pte);  // 找到次一级页表
    }
    if (*pte&PTE_V && *pte&PTE_A) {
      mask = mask | (1L << i); 
      *pte &= ~PTE_A;  // 重置pte的PTE_A
    }    
  }
  if (copyout(pagetable, abits_addr, (char*)&mask, sizeof(uint64)) < 0) return -1;
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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
