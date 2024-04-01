#ifdef TESTING_UTIL

#include "maths/maths.h"
#include "varray.h"
#include "htable.h"
#include "arena.h"
#include "queue.h"

static void test_maths(void);
static void test_arena(void);
static void test_string(void);
static void test_varray(void);
static void test_htable(void);
static void test_queue(void);

int main()
{
	DEBUG(1, "------- Starting Tests -------");
	DEBUG(1, "------------------------------");

	// test_maths();
	// test_arena();
	// test_string();
	// test_varray();
	// test_htable();
	test_queue();

	DEBUG(1, "------- Tests Complete -------");
}

static void test_maths()
{
	DEBUG(1, "\t--- Testing Maths ---");
	printf("e0:   "); pga_print(e0);
	printf("e1:   "); pga_print(e1);
	printf("e2:   "); pga_print(e2);
	printf("e3:   "); pga_print(e3);
	printf("e123: "); pga_print(e123);
	printf("\n");

	printf("reverse of 5.0:  "); pga_print(reverse(5.0f));
	printf("reverse of e0:   "); pga_print(reverse(e0));
	printf("reverse of e123: "); pga_print(reverse(e123));
	printf("reverse of e31:  "); pga_print(reverse(e31));
	printf("\n");

	printf("dual of e12: "); pga_print(dual(e12));
	printf("dual of e01: "); pga_print(dual(e01));
	printf("\n");

	printf("wedge of e1 and e2:  "); pga_print(wedge(e1, e2));
	printf("wedge of e0 and e3:  "); pga_print(wedge(e0, e3));
	printf("wedge of e3 and e3:  "); pga_print(wedge(e3, e3));
	printf("wedge of e01 and e3: "); pga_print(wedge(e01, e3));
	printf("wedge of e23 and e0: "); pga_print(wedge(e23, e0));
	printf("wedge of e31 and e1: "); pga_print(wedge(e31, e1));
	printf("\n");

	printf("inner of 2 and 3:       "); pga_print(inner(2.0f, 3.0f));
	printf("inner of e1 and e2:     "); pga_print(inner(e1, e2));
	printf("inner of e12 and e23:   "); pga_print(inner(e12, e23));
	printf("inner of e123 and e032: "); pga_print(inner(e123, e032));
	printf("inner of e123 and e1:   "); pga_print(inner(e123, e1));
	printf("\n");
}

static void test_arena()
{
	DEBUG(1, " --- Testing Arena ---");
	struct Arena* a1 = arena_new(128, 0);
	struct Arena* a2 = arena_new(299, ARENA_RESIZEABLE);
	struct Arena* a3 = arena_new(64, 0);
	assert(arena_alloc(a1, 64));
	assert(arena_alloc(a1, 30));
	assert(!arena_alloc(a1, 36));
	assert(!arena_alloc(a1, 64));
	arena_reset(a1);
	assert(arena_alloc(a1, 128));

	assert(arena_alloc(a2, 100));
	assert(arena_alloc(a2, 150));
	assert(arena_alloc(a2, 200));
}

static void test_string()
{
	DEBUG(1, " --- Testing String ---");
	char* str = "some/file/path.txt";
	DEBUG(1, "Splitting with `string_new_split()`: \"%s\"", str);
	DEBUG(1, "\t0 = %s", string_new_split(str, '/', 0, NULL).data);
	DEBUG(1, "\t1 = %s", string_new_split(str, '/', 1, NULL).data);
	DEBUG(1, "\t2 = %s", string_new_split(str, '/', 2, NULL).data);
	DEBUG(1, "\t100 = %s", string_new_split(str, '/', 100, NULL).data);
	DEBUG(1, "\t-123 = %s", string_new_split(str, '/', -123, NULL).data);
	DEBUG(1, "\t-1 = %s", string_new_split(str, '/', -1, NULL).data);

	str = "abc;123;xyz";
	DEBUG(1, "\nSplitting with `string_new_split()`: \"%s\"", str);
	DEBUG(1, "\t0 = %s", string_new_split(str, ';', 0, NULL).data);
	DEBUG(1, "\t1 = %s", string_new_split(str, ';', 1, NULL).data);
	DEBUG(1, "\t2 = %s", string_new_split(str, ';', 2, NULL).data);
	DEBUG(1, "\t3 = %s", string_new_split(str, ';', 3, NULL).data);
	DEBUG(1, "\t-1 = %s", string_new_split(str, ';', -1, NULL).data);

	str = ".b.c.d..f";
	DEBUG(1, "\nSplitting with `string_new_split()`: \"%s\"", str);
	DEBUG(1, "\t0 = %s", string_new_split(str, '.', 0, NULL).data);
	DEBUG(1, "\t1 = %s", string_new_split(str, '.', 1, NULL).data);
	DEBUG(1, "\t2 = %s", string_new_split(str, '.', 2, NULL).data);
	DEBUG(1, "\t3 = %s", string_new_split(str, '.', 3, NULL).data);
	DEBUG(1, "\t4 = %s", string_new_split(str, '.', 4, NULL).data);
	DEBUG(1, "\t5 = %s", string_new_split(str, '.', 5, NULL).data);
	DEBUG(1, "\t15 = %s", string_new_split(str, '.', 15, NULL).data);
	DEBUG(1, "\t-10 = %s", string_new_split(str, '.', -10, NULL).data);
	DEBUG(1, "\t-1 = %s", string_new_split(str, '.', -1, NULL).data);
}

static void test_varray()
{
	DEBUG(1, " --- Testing VArray ---");
	struct VArray arr = varray_new(10, 8);
	varray_print(&arr);
	varray_push(&arr, (int[]){ 1 });
	varray_push(&arr, (int[]){ 2 });
	varray_push(&arr, (int[]){ 3 });
	varray_push(&arr, (int[]){ 4 });
	varray_push(&arr, (int[]){ 5 });
	varray_print(&arr);

	DEBUG(1, "%d", *(int*)varray_get(&arr, 0));
	DEBUG(1, "%d", *(int*)varray_get(&arr, 1));
	DEBUG(1, "%d", *(int*)varray_get(&arr, 2));
	DEBUG(1, "%d", *(int*)varray_get(&arr, 3));
	DEBUG(1, "%d", *(int*)varray_get(&arr, 4));

	DEBUG(1, "Setting [0] = 99 and [4] = 100");
	varray_set(&arr, 0, (int[]){ 99 });
	varray_set(&arr, 4, (int[]){ 100 });
	varray_print(&arr);
}

static void test_htable()
{
	DEBUG(1, " --- Testing HTable ---");

	DEBUG(1, "\n - - - - - - - * htable_insert * - - - - - - - ");
	struct HTable* htable = htable_new(16);
	htable_insert(htable, string_new("x", -1, NULL), 1);
	htable_insert(htable, string_new("y", -1, NULL), 2);
	htable_insert(htable, string_new("table", -1, NULL), 3);
	htable_insert(htable, string_new("hash", -1, NULL), 4);
	htable_insert(htable, string_new("longer_value", -1, NULL), 5);
	htable_insert(htable, string_new("another_value", -1, NULL), 6);
	htable_insert(htable, string_new("hello", -1, NULL), 7);
	htable_insert(htable, string_new("world", -1, NULL), 8);
	htable_insert(htable, string_new("x2", -1, NULL), 9);
	htable_insert(htable, string_new("y2", -1, NULL), 10);

	DEBUG(1, "\n - - - - - - - * htable_print * - - - - - - - ");
	htable_print(htable);

	assert(1  == htable_get(htable, string_new("x", -1, NULL)));
	assert(2  == htable_get(htable, string_new("y", -1, NULL)));
	assert(3  == htable_get(htable, string_new("table", -1, NULL)));
	assert(4  == htable_get(htable, string_new("hash", -1, NULL)));
	assert(5  == htable_get(htable, string_new("longer_value", -1, NULL)));
	assert(6  == htable_get(htable, string_new("another_value", -1, NULL)));
	assert(7  == htable_get(htable, string_new("hello", -1, NULL)));
	assert(8  == htable_get(htable, string_new("world", -1, NULL)));
	assert(9  == htable_get(htable, string_new("x2", -1, NULL)));
	assert(10 == htable_get(htable, string_new("y2", -1, NULL)));

	DEBUG(1, "\n - - - - - - - * htable_set * - - - - - - - ");
	DEBUG(1, "Setting x and y to 50 and x2 and y2 to 100. Changing hash to 30 using htable_insert");
	htable_set(htable, string_new("x", -1, NULL), 50);
	htable_set(htable, string_new("y", -1, NULL), 50);
	htable_set(htable, string_new("x2", -1, NULL), 100);
	htable_set(htable, string_new("y2", -1, NULL), 100);
	htable_insert(htable, string_new("hash", -1, NULL), 30);

	assert(50  == htable_get(htable, string_new("x", -1, NULL)));
	assert(50  == htable_get(htable, string_new("y", -1, NULL)));
	assert(100 == htable_get(htable, string_new("x2", -1, NULL)));
	assert(100 == htable_get(htable, string_new("y2", -1, NULL)));
	assert(30  == htable_get(htable, string_new("hash", -1, NULL)));

	DEBUG(1, "\n - - - - - - - * htable_print * - - - - - - - ");
	htable_print(htable);

	htable_free(htable);
	DEBUG(1, " --- End of Testing HTable ---");
}

void test_queue()
{
	struct Queue* q = queue_new(4, sizeof(int64));
	queue_print(q);
	enqueue(q, (int64[]){ 2 });
	enqueue(q, (int64[]){ 4 });
	enqueue(q, (int64[]){ 6 });
	queue_print(q);

	DEBUG_VALUE(*(int64*)dequeue(q));
	DEBUG_VALUE(*(int64*)dequeue(q));
	DEBUG_VALUE(*(int64*)dequeue(q));
	queue_print(q);

	enqueue(q, (int64[]){ 7 });
	enqueue(q, (int64[]){ 8 });
	enqueue(q, (int64[]){ 9 });
	enqueue(q, (int64[]){ 10 });
	// queue_print(q);
	// enqueue(q, (int64[]){ 11 });
	queue_print(q);
	DEBUG_VALUE(*(int64*)dequeue(q));
	DEBUG_VALUE(*(int64*)dequeue(q));
	// DEBUG_VALUE(*(int64*)dequeue(q));
	queue_print(q);
}

#endif /* TESTING_UTIL */
