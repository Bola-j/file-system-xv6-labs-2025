#include "types.h"
#include "spinlock.h"
#include "defs.h"

#define IRQWATCH_PRINT_EVERY 100

struct irqwatch_state {
  struct spinlock lock;
  uint timer_count;
  uint uart_count;
  uint disk_count;
  uint last_print_timer;
};

static struct irqwatch_state irqwatch;

void
irqwatch_init(void)
{
  initlock(&irqwatch.lock, "irqwatch");
  irqwatch.timer_count = 0;
  irqwatch.uart_count = 0;
  irqwatch.disk_count = 0;
  irqwatch.last_print_timer = 0;
}

void
irqwatch_trap_timer(void)
{
  uint t = 0;
  uint u = 0;
  uint d = 0;
  int do_print = 0;

  acquire(&irqwatch.lock);
  irqwatch.timer_count++;
  if(irqwatch.timer_count - irqwatch.last_print_timer >= IRQWATCH_PRINT_EVERY){
    irqwatch.last_print_timer = irqwatch.timer_count;
    t = irqwatch.timer_count;
    u = irqwatch.uart_count;
    d = irqwatch.disk_count;
    do_print = 1;
  }
  release(&irqwatch.lock);

  if(do_print){
    printf("irqwatch: timer=%d uart=%d disk=%d\n", t, u, d);
  }
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
