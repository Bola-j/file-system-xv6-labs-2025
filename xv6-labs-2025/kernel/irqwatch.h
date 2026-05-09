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

// logging helpers (implemented in irqwatch.c)
void irqwatch_note_write(struct inode *ip, int bytes, uint off, int is_append);
void irqwatch_note_truncate(struct inode *ip);
void irqwatch_note_create(struct inode *ip);
void irqwatch_note_unlink(struct inode *ip);
