#ifdef TESTING_UTIL

#include "htable.h"

int main(int agrc, char** argv)
{
	DEBUG(1, " --- Testing HTable ---");

	DEBUG(1, "\n - - - - - - - * htable_insert * - - - - - - - ");
	struct HTable* htable = htable_new(16);
	htable_insert(htable, STRING32("x"), DATA(1));
	htable_insert(htable, STRING32("y"), DATA(2));
	htable_insert(htable, STRING32("table"), DATA(3));
	htable_insert(htable, STRING32("hash"), DATA(4));
	htable_insert(htable, STRING32("longer_value"), DATA(5));
	htable_insert(htable, STRING32("another_value"), DATA(6));
	htable_insert(htable, STRING32("hello"), DATA(7));
	htable_insert(htable, STRING32("world"), DATA(8));
	htable_insert(htable, STRING32("x2"), DATA(9));
	htable_insert(htable, STRING32("y2"), DATA(10));

	DEBUG(1, "\n - - - - - - - * htable_print * - - - - - - - ");
	htable_print(htable);

	DEBUG(1, "\n - - - - - - - * htable_get * - - - - - - - ");
	DEBUG(1, "x: %ld", htable_get(htable, STRING32("x")).s64);
	DEBUG(1, "y: %ld", htable_get(htable, STRING32("y")).s64);
	DEBUG(1, "table: %ld", htable_get(htable, STRING32("table")).s64);
	DEBUG(1, "hash: %ld", htable_get(htable, STRING32("hash")).s64);
	DEBUG(1, "longer_value: %ld", htable_get(htable, STRING32("longer_value")).s64);
	DEBUG(1, "another_value: %ld", htable_get(htable, STRING32("another_value")).s64);
	DEBUG(1, "hello: %ld", htable_get(htable, STRING32("hello")).s64);
	DEBUG(1, "world: %ld", htable_get(htable, STRING32("world")).s64);
	DEBUG(1, "x2: %ld", htable_get(htable, STRING32("x2")).s64);
	DEBUG(1, "y2: %ld", htable_get(htable, STRING32("y2")).s64);

	DEBUG(1, "\n - - - - - - - * htable_set * - - - - - - - ");
	DEBUG(1, "Setting x and y to 50 and x2 and y2 to 100");
	htable_set(htable, STRING32("x"), DATA(50));
	htable_set(htable, STRING32("y"), DATA(50));
	htable_set(htable, STRING32("x2"), DATA(100));
	htable_set(htable, STRING32("y2"), DATA(100));

	DEBUG(1, "\n - - - - - - - * htable_print * - - - - - - - ");
	htable_print(htable);

	htable_free(htable);
	DEBUG(1, " --- End of Testing HTable ---");
}

#endif /* TESTING_UTIL */
