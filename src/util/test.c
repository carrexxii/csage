#ifdef TESTING_UTIL

#include "varray.h"
#include "htable.h"
#include "arena.h"

int main()
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

	DEBUG(1, " --- Testing String ---");
	char* str = "some/file/path.txt";
	DEBUG(1, "Splitting with `string_new_split()`: \"%s\"", str);
	DEBUG(1, "\t0 = %s", string_new_split(str, '/', 0).data);
	DEBUG(1, "\t1 = %s", string_new_split(str, '/', 1).data);
	DEBUG(1, "\t2 = %s", string_new_split(str, '/', 2).data);
	DEBUG(1, "\t100 = %s", string_new_split(str, '/', 100).data);
	DEBUG(1, "\t-123 = %s", string_new_split(str, '/', -123).data);
	DEBUG(1, "\t-1 = %s", string_new_split(str, '/', -1).data);

	str = "abc;123;xyz";
	DEBUG(1, "\nSplitting with `string_new_split()`: \"%s\"", str);
	DEBUG(1, "\t0 = %s", string_new_split(str, ';', 0).data);
	DEBUG(1, "\t1 = %s", string_new_split(str, ';', 1).data);
	DEBUG(1, "\t2 = %s", string_new_split(str, ';', 2).data);
	DEBUG(1, "\t3 = %s", string_new_split(str, ';', 3).data);
	DEBUG(1, "\t-1 = %s", string_new_split(str, ';', -1).data);

	str = ".b.c.d..f";
	DEBUG(1, "\nSplitting with `string_new_split()`: \"%s\"", str);
	DEBUG(1, "\t0 = %s", string_new_split(str, '.', 0).data);
	DEBUG(1, "\t1 = %s", string_new_split(str, '.', 1).data);
	DEBUG(1, "\t2 = %s", string_new_split(str, '.', 2).data);
	DEBUG(1, "\t3 = %s", string_new_split(str, '.', 3).data);
	DEBUG(1, "\t4 = %s", string_new_split(str, '.', 4).data);
	DEBUG(1, "\t5 = %s", string_new_split(str, '.', 5).data);
	DEBUG(1, "\t15 = %s", string_new_split(str, '.', 15).data);
	DEBUG(1, "\t-10 = %s", string_new_split(str, '.', -10).data);
	DEBUG(1, "\t-1 = %s", string_new_split(str, '.', -1).data);

	DEBUG(1, " --- Testing VArray ---");
	struct VArray* arr = varray_new(10, 8);
	varray_print(arr);
	varray_push(arr, (int[]){ 1 });
	varray_push(arr, (int[]){ 2 });
	varray_push(arr, (int[]){ 3 });
	varray_push(arr, (int[]){ 4 });
	varray_push(arr, (int[]){ 5 });
	varray_print(arr);

	DEBUG(1, "%d", *(int*)varray_get(arr, 0));
	DEBUG(1, "%d", *(int*)varray_get(arr, 1));
	DEBUG(1, "%d", *(int*)varray_get(arr, 2));
	DEBUG(1, "%d", *(int*)varray_get(arr, 3));
	DEBUG(1, "%d", *(int*)varray_get(arr, 4));

	DEBUG(1, "Setting [0] = 99 and [4] = 100");
	varray_set(arr, 0, (int[]){ 99 });
	varray_set(arr, 4, (int[]){ 100 });
	varray_print(arr);

	DEBUG(1, " --- Testing HTable ---");

	DEBUG(1, "\n - - - - - - - * htable_insert * - - - - - - - ");
	struct HTable* htable = htable_new(16);
	htable_insert(htable, string_new("x", -1), 1);
	htable_insert(htable, string_new("y", -1), 2);
	htable_insert(htable, string_new("table", -1), 3);
	htable_insert(htable, string_new("hash", -1), 4);
	htable_insert(htable, string_new("longer_value", -1), 5);
	htable_insert(htable, string_new("another_value", -1), 6);
	htable_insert(htable, string_new("hello", -1), 7);
	htable_insert(htable, string_new("world", -1), 8);
	htable_insert(htable, string_new("x2", -1), 9);
	htable_insert(htable, string_new("y2", -1), 10);

	DEBUG(1, "\n - - - - - - - * htable_print * - - - - - - - ");
	htable_print(htable);

	assert(1  == htable_get(htable, string_new("x", -1)));
	assert(2  == htable_get(htable, string_new("y", -1)));
	assert(3  == htable_get(htable, string_new("table", -1)));
	assert(4  == htable_get(htable, string_new("hash", -1)));
	assert(5  == htable_get(htable, string_new("longer_value", -1)));
	assert(6  == htable_get(htable, string_new("another_value", -1)));
	assert(7  == htable_get(htable, string_new("hello", -1)));
	assert(8  == htable_get(htable, string_new("world", -1)));
	assert(9  == htable_get(htable, string_new("x2", -1)));
	assert(10 == htable_get(htable, string_new("y2", -1)));

	DEBUG(1, "\n - - - - - - - * htable_set * - - - - - - - ");
	DEBUG(1, "Setting x and y to 50 and x2 and y2 to 100. Changing hash to 30 using htable_insert");
	htable_set(htable, string_new("x", -1), 50);
	htable_set(htable, string_new("y", -1), 50);
	htable_set(htable, string_new("x2", -1), 100);
	htable_set(htable, string_new("y2", -1), 100);
	htable_insert(htable, string_new("hash", -1), 30);

	assert(50  == htable_get(htable, string_new("x", -1)));
	assert(50  == htable_get(htable, string_new("y", -1)));
	assert(100 == htable_get(htable, string_new("x2", -1)));
	assert(100 == htable_get(htable, string_new("y2", -1)));
	assert(30  == htable_get(htable, string_new("hash", -1)));

	DEBUG(1, "\n - - - - - - - * htable_print * - - - - - - - ");
	htable_print(htable);

	htable_free(htable);
	DEBUG(1, " --- End of Testing HTable ---");
}

#endif /* TESTING_UTIL */
