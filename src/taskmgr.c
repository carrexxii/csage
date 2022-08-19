#include "taskmgr.h"

#define MAX_THREADS       4
#define MAX_TASKS         64
#define THREAD_STACK_SIZE 2*1024*1024

static bool get_task();
static int  thread_loop(void* data);

uint8 taskc;
struct Task {
	void (*fn)();
	bool active;
} tasks[MAX_TASKS];

static int8   threadc;
static Thread threads[MAX_THREADS];

static atomic_uint donec;
static Mutex       mtxGetTask;

void init_taskmgr()
{
	mtx_init(&mtxGetTask, mtx_plain);
	atomic_init(&donec, 0);
	for (uint i = 1; i < MAX_THREADS; i++)
		if (thrd_create(&threads[threadc++], thread_loop, NULL))
			ERROR("[THR] Failed to create thread %u", i);

	DEBUG(2, "[THR] Initialized %d threads", threadc);
}

void add_taskmgr_task(void (*fn)())
{
	if (taskc < MAX_TASKS) {
		tasks[taskc++].fn = fn;
		DEBUG(3, "[THR] Added new task (%u)", taskc);
	} else {
		ERROR("[THR] Task list is already full");
	}
}

bool reset_taskmgr()
{
	if (donec >= taskc) {
		atomic_store(&donec, 0);
		for (uint i = 0; i < taskc; i++)
			tasks[i].active = false;

		return false;
	}

	return true;
}

void free_taskmgr()
{
	for (int i = 0; i < threadc; i++)
		thrd_detach(threads[i]);
	mtx_destroy(&mtxGetTask);
}

static bool get_task()
{
	struct Task* task;
	mtx_lock(&mtxGetTask);
	for (int i = 0; i < taskc; i++) {
		task = tasks + i;
		if (!task->active) {
			task->active = true;
			mtx_unlock(&mtxGetTask);
			task->fn();

			atomic_fetch_add(&donec, 1);

			return false;
		}
	}

	mtx_unlock(&mtxGetTask);
	return true;
}

static int thread_loop(void* data)
{
	while (1)
		if (get_task())
			thrd_sleep(&(struct timespec){ .tv_nsec=10000 }, NULL);

	ERROR("[THR] Thread completed");
	return 1;
}

