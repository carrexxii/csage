#ifndef TASKMGR_H
#define TASKMGR_H

void init_taskmgr();
void add_taskmgr_task(void (*fn)());
bool reset_taskmgr();
void free_taskmgr();

#endif
