#ifndef LANG_ERRORS_H
#define LANG_ERRORS_H

inline static void print_error(int line, char* err) {
	fprintf(stderr, "Error on line %d: \"%s\"", line, err);
}

#endif
