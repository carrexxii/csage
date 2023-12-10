#ifndef TASKMGR_H
#define TASKMGR_H

void taskmgr_init(void);
void taskmgr_add_task(void (*fn)(void));
bool taskmgr_reset(void);
void taskmgr_clear(void);
void taskmgr_free(void);

#endif
