#ifndef LANG_PARSER_H
#define LANG_PARSER_H

#include "lexer.h"

enum ASTType {
	AST_NONE,
	AST_INT,
	AST_REAL,
	AST_STR,
	AST_BOOL,
	AST_IDENT,
	AST_IDENT_LIST,
	AST_PAREN,
	AST_UNARY,
	AST_BINARY,
	AST_ASSIGN,
};

struct ASTNode {
	String lexeme;
	enum ASTType type;
	union {
		/* Terminal */
		union {
			int64  integer;
			double real;
		};
		/* Non-terminal */
		struct ASTNode* paren;
		struct {
			String8 symbol;
			union {
				struct { /* Unary */
					struct ASTNode* node;
				};
				struct { /* Binary */
					struct ASTNode* left;
					struct ASTNode* right;
				};
			};
		};
	};
};

struct AST {
	struct ASTNode* nodes;
	intptr nodec;
};

struct AST parser_parse(struct TokenList* tokens);
void parser_print_ast(struct AST ast);

inline static bool token_is_terminal(enum ASTType type) {
	return type == AST_INT  || type == AST_REAL  || type == AST_STR ||
	       type == AST_BOOL || type == AST_IDENT || type == AST_IDENT_LIST;
}

#define STRING_OF_NODE(_e0) \
	(_e0) == AST_NONE?       "AST_NONE":       (_e0) == AST_INT?    "AST_INT":   (_e0) == AST_REAL?  "AST_REAL":  \
	(_e0) == AST_STR?        "AST_STR":        (_e0) == AST_BOOL?   "AST_BOOL":  (_e0) == AST_IDENT? "AST_IDENT": \
	(_e0) == AST_IDENT_LIST? "AST_IDENT_LIST": (_e0) == AST_PAREN?  "AST_PAREN": (_e0) == AST_UNARY? "AST_UNARY": \
	(_e0) == AST_BINARY?     "AST_BINARY":     (_e0) == AST_ASSIGN? "AST_ASSIGN":                                 \
	"<Unknown value for enum \"ASTType\">"

#endif
