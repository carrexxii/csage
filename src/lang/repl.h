#ifndef LANG_REPL_H
#define LANG_REPL_H

#include "lexer.h"

#define REPL_BUFFER_SIZE 1024

inline static void lang_repl() {
	char line[REPL_BUFFER_SIZE];
	while (fgets(line, REPL_BUFFER_SIZE, stdin) && *line != '\n') {
		lexer_tokenize(line);
	}
}

#endif
