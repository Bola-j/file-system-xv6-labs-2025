#define IRQWATCH_LOG_MAX 64

struct irqwatch_entry {
  uint ticks;
  int pid;
  uint inum;
  uint off;
  int bytes;
  int is_append;
  char proc_name[16];
};
