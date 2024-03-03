#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <float.h>
#include <limits.h>
#include <stdnoreturn.h>
#include <stdalign.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include <fenv.h>
#include <ctype.h>
#include <errno.h>

#if defined __STDC_NO_ATOMICS__ || defined __STDC_NO_THREADS__
	#error "Both C11 threads and atomics are required"
#else
	#include <stdatomic.h>
	#include <threads.h>
#endif

#if defined __unix__ || (defined __APPLE__ && defined __MACH__)
	#include <unistd.h>
#elif defined _WIN32
	#include <io.h>
#endif

#include "config.h"

#define FPS              50.0
#define DT               (1.0/FPS)
#define DT_MS            (DT*1000.0)
#define DEBUG_MALLOC_MIN (128*1024)

/*  +------------------------------------------+
 *  |           Integral Types Chart           |
 *  +----------+--------+----------------------+
 *  |   Type   |  Bits  |   Range (approx.)    |
 *  +----------+--------+----------------------+
 *  |     char |    8   |     +-127 || 0..255  |
 *  |    short |   16   |  +-3.27e4 || 6.55e4  |
 *  |      int | 16-32* |  +-2.14e9 || 4.29e9  |
 *  |     long | 32*-64 |  +-2.14e9 || 4.29e9  |
 *  | longlong |   64   | +-9.22e18 || 1.84e19 |
 *  +----------+--------+----------------------+
 */
typedef  int8_t   int8;
typedef  int16_t  int16;
typedef  int32_t  int32;
typedef  int64_t  int64;
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uint64_t ID;

typedef unsigned int uint;
typedef  intptr_t  intptr;
typedef uintptr_t uintptr;
typedef unsigned char byte;
typedef  size_t usize;
typedef ssize_t isize;

typedef thrd_t Thread;
typedef mtx_t  Mutex;
typedef cnd_t  Condition;
typedef tss_t  ThreadLocal;

enum Axis {
	AXIS_NONE,
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
};
enum Direction {
	DIR_NONE         = 0x00,
	DIR_UP           = 0x01,
	DIR_DOWN         = 0x02,
	DIR_RIGHT        = 0x04,
	DIR_LEFT         = 0x08,
	DIR_FORWARDS     = 0x10,
	DIR_BACKWARDS    = 0x20,
	DIR_ROTATE_LEFT  = 0x21,
	DIR_ROTATE_RIGHT = 0x22,
};

typedef union Colour {
	uint32 abgr;
	struct { uint8 a, b, g, r; };
} Colour;
#define COLOUR(hex)      (Colour){ .abgr = hex }
#define COLOUR_TO_VEC(c) (float[4]){ c.r/255.0f, c.g/255.0f, c.b/255.0f, c.a/255.0f }

#define UNPACK2(x) x[0], x[1]
#define UNPACK3(x) x[0], x[1], x[2]
#define UNPACK4(x) x[0], x[1], x[2], x[3]
#define UNPACK5(x) x[0], x[1], x[2], x[3], x[4]
#define UNPACK6(x) x[0], x[1], x[2], x[3], x[4], x[5]
#define UNPACK7(x) x[0], x[1], x[2], x[3], x[4], x[5], x[6]
#define UNPACK8(x) x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]

#ifndef _WIN32
	#define MIN(a, b) ((a) < (b)? (a): (b))
	#define MAX(a, b) ((a) > (b)? (a): (b))
#endif
#define BETWEEN(a, b, c) ((bool)((a) >= (b) && (a) <= (c)))
#define CLAMP(a, b, c)   do { ((a) = (a) < (b)? (b): (a) > (c)? (c): (a)); } while (0)
#define ARRAY_SIZE(a)    (sizeof(a) / sizeof(a[0]))
#define DIV_CEIL(a, b)   (((a) + (b) - 1) / (b))
#define AVERAGE(a, b)    (((a) + (b)) / 2)

#define STRING_TF(x) ((x)? "true": "false")
#define STRING_YN(x) ((x)? "yes" : "no"   )

#define SELECT1(_1, ...) _1
#define SELECT2(_1, _2, ...) _2
#define SELECT3(_1, _2, _3, ...) _3

#define COLOUR_WHITE   1.0, 1.0, 1.0
#define COLOUR_BLACK   0.0, 0.0, 0.0
#define COLOUR_RED     1.0, 0.0, 0.0
#define COLOUR_GREEN   0.0, 1.0, 0.0
#define COLOUR_BLUE    0.0, 0.0, 1.0
#define COLOUR_MAGENTA 1.0, 0.0, 1.0
#define COLOUR_YELLOW  1.0, 1.0, 0.0
#define COLOUR_CYAN    0.0, 1.0, 1.0

#ifndef NO_TERM_COLOUR
#define TERM_NORMAL  "\x1B[0m"
#define TERM_RED     "\x1B[31m"
#define TERM_GREEN   "\x1B[32m"
#define TERM_YELLOW  "\x1B[33m"
#define TERM_BLUE    "\x1B[34m"
#define TERM_MAGENTA "\x1B[35m"
#define TERM_CYAN    "\x1B[36m"
#define TERM_WHITE   "\x1B[37m"
#define DEBUG_COLOUR(str) (fprintf(stderr,                                     \
	                               !strncmp((str), "[INIT]", 6)? TERM_BLUE   : \
	                               !strncmp((str), "[RES]" , 5)? TERM_GREEN  : \
	                               !strncmp((str), "[UTIL]", 6)? TERM_GREEN  : \
	                               !strncmp((str), "[INFO]", 6)? TERM_YELLOW : \
	                               !strncmp((str), "[ENT]" , 5)? TERM_BLUE   : \
	                               !strncmp((str), "[THR]" , 5)? TERM_YELLOW : \
	                               !strncmp((str), "[MAP]" , 5)? TERM_YELLOW : \
	                               !strncmp((str), "[GFX]" , 5)? TERM_MAGENTA: \
	                               !strncmp((str), "[VK]"  , 4)? TERM_CYAN   : \
	                               !strncmp((str), "[MEM]" , 5)? TERM_MAGENTA: \
	                               !strncmp((str), "[UI]"  , 4)? TERM_YELLOW : \
	                               TERM_NORMAL))
#else
#define TERM_NORMAL  ""
#define TERM_RED     ""
#define TERM_GREEN   ""
#define TERM_YELLOW  ""
#define TERM_BLUE    ""
#define TERM_MAGENTA ""
#define TERM_CYAN    ""
#define TERM_WHITE   ""
#define DEBUG_COLOUR(str)
#endif

#if DEBUG_LEVEL > 0
#define D DEBUG(1, TERM_RED "<debug marker>" TERM_NORMAL)
#define DEBUG(_lvl, ...) do {                                        \
			if (_lvl <= DEBUG_LEVEL && SELECT1(__VA_ARGS__, "")[0]) { \
				fprintf(stderr, __FILE__ ":%d ", __LINE__);            \
				DEBUG_COLOUR(SELECT1(__VA_ARGS__, ""));                 \
				fprintf(stderr, __VA_ARGS__);                            \
				fprintf(stderr, "\n" TERM_NORMAL);                        \
			}                                                              \
		} while (0)
#define DEBUG_VALUE(x) do {                                                               \
			fprintf(stderr, __FILE__ ":%d ", __LINE__);                                    \
			fprintf(stderr, _Generic((x),                                                   \
			         char: "Char: \"%c\"\n",               signed char: "SChar: \"%hhd\"\n", \
			        _Bool: "Bool: %d\n",                 unsigned char: "UChar: \"%hhu\"\n",  \
			    short int: "Short: %hd\n",          unsigned short int: "UShort: %hu\n",       \
			          int: "Int: %d\n",                   unsigned int: "UInt: %u\n",           \
			     long int: "Long: %ld\n",            unsigned long int: "ULong: %lu\n",          \
			long long int: "Long Long: %lld\n", unsigned long long int: "ULong Long: %llu\n",     \
			        float: "Float: %g\n",                       double: "Double: %g\n",            \
			  long double: "Long Double: %lg\n",                 char*: "Char*: \"%s\"\n",          \
			  default: "<unknown or pointer type>%p\n"), (x));                                       \
		} while (0)
#define ERROR(...) do {                                   \
			fprintf(stderr, TERM_RED);                     \
			fprintf(stderr, __VA_ARGS__);                   \
			fprintf(stderr, "\n\t%s:%d in %s\n" TERM_NORMAL, \
			        __FILE__, __LINE__, __func__);            \
		} while (0)
#else
#define DEBUG(...)
#define DEBUG_VALUE(x)
#define ERROR(...)
#endif

#define typename(x) (_Generic((x),                                                \
        _Bool: "_Bool",                  unsigned char: "unsigned char",          \
         char: "char",                     signed char: "signed char",            \
    short int: "short int",         unsigned short int: "unsigned short int",     \
          int: "int",                     unsigned int: "unsigned int",           \
     long int: "long int",           unsigned long int: "unsigned long int",      \
long long int: "long long int", unsigned long long int: "unsigned long long int", \
        float: "float",                         double: "double",                 \
  long double: "long double",                    char*: "pointer to char",        \
        void*: "pointer to void",                 int*: "pointer to int",         \
       float*: "pointer to float",             double*: "pointer to double",      \
      default: "other"))

#if DEBUG_LEVEL > 0
#define smalloc(x)     _smalloc( (x),      __FILE__, __LINE__, __func__)
#define scalloc(x, y)  _scalloc( (x), (y), __FILE__, __LINE__, __func__)
#define srealloc(x, y) _srealloc((x), (y), __FILE__, __LINE__, __func__)
#define sfree(x)       _sfree(   (x),      __FILE__, __LINE__, __func__)
#else
#define smalloc(x)     malloc(x)
#define scalloc(x, y)  calloc(x, y)
#define srealloc(x, y) realloc(x, y)
#define sfree(x)       free(x)
#endif

void* _smalloc(isize s, const char* file, int line, const char* fn);
void* _scalloc(isize n, isize s, const char* file, int line, const char* fn);
void* _srealloc(void* mem, isize n, const char* file, int line, const char* fn);
void  _sfree(void* mem, const char* file, int line, const char* fn);

// TODO: move this
/* [min, max] */
inline static int random_int(int min, int max)
{
    int num;
    int div = RAND_MAX / (max + 1);
    do {
        num = rand() / div;
    } while (!BETWEEN(num, min, max));

    return num;
}

extern int vk_err;

#endif

