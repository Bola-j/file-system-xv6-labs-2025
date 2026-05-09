#include "spinlock.h"
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "irqwatch.h"

struct irqwatch_state {
  struct spinlock lock;
  uint timer_count;
  uint uart_count;
  uint disk_count;
  struct irqwatch_entry log[IRQWATCH_LOG_MAX];
  uint log_head;
  uint log_tail;
  uint log_count;
};

static struct irqwatch_state irqwatch;

void
irqwatch_init(void)
{
  initlock(&irqwatch.lock, "irqwatch");
  irqwatch.timer_count = 0;
  irqwatch.uart_count = 0;
  irqwatch.disk_count = 0;
  irqwatch.log_head = 0;
  irqwatch.log_tail = 0;
  irqwatch.log_count = 0;
}

void
irqwatch_trap_timer(void)
{
  acquire(&irqwatch.lock);
  irqwatch.timer_count++;
  release(&irqwatch.lock);
}

void
irqwatch_trap_uart(void)
{
  acquire(&irqwatch.lock);
  irqwatch.uart_count++;
  release(&irqwatch.lock);
}

void
irqwatch_trap_disk(void)
{
  acquire(&irqwatch.lock);
  irqwatch.disk_count++;
  release(&irqwatch.lock);
}

void
irqwatch_note_write(struct inode *ip, int bytes, uint off, int is_append)
{
  struct irqwatch_entry entry;
  struct proc *p = myproc();

  if(ip == 0)
    return;

  // allow logging of zero-byte events (truncate/create/delete)
  if(bytes < 0)
    bytes = 0;

  entry.ticks = ticks;
  entry.pid = p ? p->pid : -1;
  entry.inum = ip->inum;
  entry.off = off;
  entry.bytes = bytes;
  entry.is_append = is_append;
  if(p)
    safestrcpy(entry.proc_name, p->name, sizeof(entry.proc_name));
  else
    safestrcpy(entry.proc_name, "?", sizeof(entry.proc_name));

  acquire(&irqwatch.lock);
  if(irqwatch.log_count < IRQWATCH_LOG_MAX){
    irqwatch.log[irqwatch.log_head] = entry;
    irqwatch.log_head = (irqwatch.log_head + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_count++;
  } else {
    irqwatch.log[irqwatch.log_head] = entry;
    irqwatch.log_head = (irqwatch.log_head + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_tail = (irqwatch.log_tail + 1) % IRQWATCH_LOG_MAX;
  }
  release(&irqwatch.lock);
}

void
irqwatch_note_truncate(struct inode *ip)
{
  struct irqwatch_entry entry;
  struct proc *p = myproc();

  if(ip == 0)
    return;

  entry.ticks = ticks;
  entry.pid = p ? p->pid : -1;
  entry.inum = ip->inum;
  entry.off = 0;
  entry.bytes = 0;
  entry.is_append = 0;
  if(p)
    safestrcpy(entry.proc_name, p->name, sizeof(entry.proc_name));
  else
    safestrcpy(entry.proc_name, "?", sizeof(entry.proc_name));

  acquire(&irqwatch.lock);
  if(irqwatch.log_count < IRQWATCH_LOG_MAX){
    irqwatch.log[irqwatch.log_head] = entry;
    irqwatch.log_head = (irqwatch.log_head + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_count++;
  } else {
    irqwatch.log[irqwatch.log_head] = entry;
    irqwatch.log_head = (irqwatch.log_head + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_tail = (irqwatch.log_tail + 1) % IRQWATCH_LOG_MAX;
  }
  release(&irqwatch.lock);
}

void
irqwatch_note_create(struct inode *ip)
{
  struct irqwatch_entry entry;
  struct proc *p = myproc();

  if(ip == 0)
    return;

  entry.ticks = ticks;
  entry.pid = p ? p->pid : -1;
  entry.inum = ip->inum;
  entry.off = 0;
  entry.bytes = 0; // creation event
  entry.is_append = 0;
  if(p)
    safestrcpy(entry.proc_name, p->name, sizeof(entry.proc_name));
  else
    safestrcpy(entry.proc_name, "?", sizeof(entry.proc_name));

  acquire(&irqwatch.lock);
  if(irqwatch.log_count < IRQWATCH_LOG_MAX){
    irqwatch.log[irqwatch.log_head] = entry;
    irqwatch.log_head = (irqwatch.log_head + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_count++;
  } else {
    irqwatch.log[irqwatch.log_head] = entry;
    irqwatch.log_head = (irqwatch.log_head + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_tail = (irqwatch.log_tail + 1) % IRQWATCH_LOG_MAX;
  }
  release(&irqwatch.lock);
}

void
irqwatch_note_unlink(struct inode *ip)
{
  struct irqwatch_entry entry;
  struct proc *p = myproc();

  if(ip == 0)
    return;

  entry.ticks = ticks;
  entry.pid = p ? p->pid : -1;
  entry.inum = ip->inum;
  entry.off = 0;
  entry.bytes = 0; // deletion event
  entry.is_append = 0;
  if(p)
    safestrcpy(entry.proc_name, p->name, sizeof(entry.proc_name));
  else
    safestrcpy(entry.proc_name, "?", sizeof(entry.proc_name));

  acquire(&irqwatch.lock);
  if(irqwatch.log_count < IRQWATCH_LOG_MAX){
    irqwatch.log[irqwatch.log_head] = entry;
    irqwatch.log_head = (irqwatch.log_head + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_count++;
  } else {
    irqwatch.log[irqwatch.log_head] = entry;
    irqwatch.log_head = (irqwatch.log_head + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_tail = (irqwatch.log_tail + 1) % IRQWATCH_LOG_MAX;
  }
  release(&irqwatch.lock);
}

int
irqwatch_read(uint64 dst, int max)
{
  struct proc *p = myproc();
  struct irqwatch_entry tmp[IRQWATCH_LOG_MAX];
  int i = 0;
  int n = max;

  if(n <= 0)
    return 0;
  if(n > IRQWATCH_LOG_MAX)
    n = IRQWATCH_LOG_MAX;

  acquire(&irqwatch.lock);
  while(i < n && irqwatch.log_count > 0){
    tmp[i] = irqwatch.log[irqwatch.log_tail];
    irqwatch.log_tail = (irqwatch.log_tail + 1) % IRQWATCH_LOG_MAX;
    irqwatch.log_count--;
    i++;
  }
  release(&irqwatch.lock);

  if(i == 0)
    return 0;
  if(copyout(p->pagetable, dst, (char *)tmp, i * sizeof(tmp[0])) < 0)
    return -1;
  return i;
}
