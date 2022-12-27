#ifndef TASKMGR_H
#define TASKMGR_H

void taskmgr_init();
void taskmgr_add_task(void (*fn)());
bool taskmgr_reset();
void taskmgr_free();

#endif
